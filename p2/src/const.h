/*
Author: Gavin Witsken
Program: ExFS - extensible filesystem
File: const.h
*/

#include <limits.h>

/* Size of each segment in bytes */
#define SEGMENT_SIZE 5242880
/* Size of each inode block in bytes */
#define INODE_BLOCK_SIZE 4096
/* Size of each data block in bytes */
#define DATA_BLOCK_SIZE 4096 // Data block size in bytes
/* Number of inodes for per segment */
#define INODES_PER_SEGMENT 10 
/* Maximum character length of a file/directory name */
#define MAX_NAME_LENGTH 50

