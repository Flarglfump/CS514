#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define BLOCKSIZE 4096
#define FILENODESIZE sizeof(struct filenode)

typedef struct filenode{
  char name[100];
  int numblocks;
  int size;
  int blockptrs[1000];
} filenode;

int getfsize(FILE* in){
  int bytesread=0;
  int total=0;
  long int where = ftell(in);
  fseek(in, 0, SEEK_SET);
  char buffer[BLOCKSIZE];
  while((bytesread=fread(&buffer, 1, BLOCKSIZE, in))>0){
    total+=bytesread;
  }
  fseek(in, where, SEEK_SET);
  return total;
}

int getnumblocks(int fsize){
  /*  int size = (fsize/BLOCKSIZE);
  if (size*BLOCKSIZE != fsize) size++;
  return size;
  */
  return fsize/BLOCKSIZE;
}

int main(int argc, char * argv[]) {
  if (argc != 2){
    printf("Usage: ./unbreak <file in>");
    return(1);
  }

  FILE * fin = fopen(argv[1], "r");

  if (!fin) {
    fprintf(stderr, "Error: could not open file \"%s\" for reading.\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  int fsize = getfsize(fin);
  printf("Filesize: %d bytes\n", fsize);

  int fblocks = getnumblocks(fsize);
  printf("Block count: %d blocks\n", fblocks);

  filenode inFnode;
  memset(&inFnode, 0, sizeof(filenode));

  fread(&inFnode, sizeof(filenode), 1, fin); // FILE * fin is now positioned at point where data starts

  int leftover = inFnode.size % BLOCKSIZE;

  printf("Original file name: %s\n", inFnode.name);
  printf("Original file size: %d\n", inFnode.size);
  printf("Last block data: %d\n", leftover);
  printf("Number of blocks: %d\n", inFnode.numblocks);

  int doWrite = 1;
  if (access(inFnode.name, F_OK) == 0) {
    printf("File \"%s\" already exists in current directory. Do you want to overwrite it? (y/n): ", inFnode.name);
    char response = getchar();
    if (response != 'y' && response != 'Y') {
      exit(EXIT_SUCCESS);
    }
  }

  FILE * fout = fopen(inFnode.name, "w+");
  
  if (!fout) {
    fprintf(stderr, "Error: could not open file \"%s\" for writing.\n", inFnode.name);
    exit(EXIT_FAILURE);
  }

  fpos_t dataStartPos;
  if (fgetpos(fin, &dataStartPos) != 0) {
    fprintf(stderr, "Error: could not get position in input file stream\n");
    exit(EXIT_FAILURE);
  }

  char buf[BLOCKSIZE];
  for (int i = 0; i < inFnode.numblocks; i++) {
    memset(buf, 0, BLOCKSIZE);
    fsetpos(fin, &dataStartPos);
    fseek(fin, inFnode.blockptrs[i] * BLOCKSIZE, SEEK_CUR);

    if (i == inFnode.numblocks - 1) {
      fread(buf, leftover, 1, fin);
      fwrite(buf, leftover, 1, fout);
    } else {
      fread(buf, BLOCKSIZE, 1, fin);
      fwrite(buf, BLOCKSIZE, 1, fout);
    }
  }

  fclose(fin);
  fclose(fout);
}