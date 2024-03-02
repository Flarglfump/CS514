/*
Author: Gavin Witsken
Program: Timing some stuff
File: timingStuff.h
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

typedef struct timespec timespec_t;

timespec_t calcDiff(timespec_t start, timespec_t end);

void printTime(timespec_t t);
void empty_function();