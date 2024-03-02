/*
Author: Gavin Witsken
Program: Timing some stuff
File: timingStuff.c
*/
#include "helperFuncs.h"

timespec_t calcDiff(timespec_t start, timespec_t end) {
    timespec_t temp;

    if ((end.tv_nsec-start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }

    return temp;
}

void printTime(timespec_t t) {
    printf("%d.%09ld\n", t.tv_sec, t.tv_nsec);
}

void empty_function() {
    asm volatile(""); // Ensure compiler does not optimize out - just do a nop
}