/*
Author: Gavin Witsken
Program: ExFS - extensible filesystem
File: types.h
*/

#include "const.h"


struct SuperBlock {
  unsigned int totBlkCnt;
  unsigned int freeBlkCnt;
  unsigned int totInodeCnt;
  unsigned int freeIndCnt;
};

struct FreeBlockList {
  char blocks[SEGMENT_SIZE / DATA_BLOCK_SIZE / CHAR_BIT];
};

struct Inode {
  union {
    char buf[INODE_BLOCK_SIZE];
    struct {
      unsigned int inodeNum;
      char type; // 0 for dir, nonzero for regular file
      unsigned int fsize; // Size of file in bytes
      unsigned int singleIndir; // Number of single indirect inode
      unsigned int doubleIndir; // Number of double indirect inode
      unsigned int tripleIndir; // Number of triple indirect inode
    };
  };
};