#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE 4096

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

void shuffle(int* ints, int size){
  int other;
  int t;
  
  for(int i = 0; i < size; i++){
    other = rand()%size;
    t = ints[i];
    ints[i] = ints[other];
    ints[other] = t;
    
  }
}


int getnumblocks(int fsize){
  /*  int size = (fsize/BLOCKSIZE);
  if (size*BLOCKSIZE != fsize) size++;
  return size;
  */
  return fsize/BLOCKSIZE;
}

int* getrands(int fsize){
  int size = getnumblocks(fsize);
  int* ints = malloc(sizeof(int)*size);
  for(int i = 0; i < size; i++){
    ints[i] = i;
  }
  shuffle(ints, size);
  return ints;
}

int main(int argc, char** argv){
  if (argc != 3){
    printf("Usage: ./break <file in> <file out>");
    return(1);
  }

  FILE* filein = fopen(argv[1], "r");
  FILE* fileout = fopen(argv[2], "w+");
  int* rands = getrands(getfsize(filein));
  
  
  filenode new;
  memset(&new, 0, sizeof(filenode));
  strcpy(new.name, argv[1]);
  for(int i = 0; i < getnumblocks(getfsize(filein)); i++){
    new.blockptrs[i] = rands[i];
  }

  new.numblocks = getnumblocks(getfsize(filein));
  new.size = getfsize(filein);
  
  fwrite(&new, sizeof(filenode), 1, fileout);
  
  char buffer[BLOCKSIZE];
  int amount = 0;
  int index = 0;
  int total = 0;
  
  while((amount = fread(&buffer, 1, BLOCKSIZE, filein)) > 0){
    fseek(fileout, sizeof(filenode)+BLOCKSIZE*rands[index], SEEK_SET);
    fwrite(&buffer, amount, 1, fileout);
    total+=amount;
    index++;
    printf("amount read %d\n", amount);
  }

  printf("total bytes written %d\n", total);
  printf("at index %d\n", index);
  fclose(fileout);

  return 0;
}
