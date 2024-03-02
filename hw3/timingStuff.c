/*
Author: Gavin Witsken
Program: Timing some stuff
File: timingStuff.c (driver)
*/

long numIterations;

#include "helperFuncs.h"

int main(int argc, char * argv[]) {
    if (argc > 1) {
        numIterations = atoi(argv[1]);
    } else {
        numIterations = 1000;
    }

    timespec_t start;
    timespec_t end;
    timespec_t diff;

    timespec_t function_call_cost;
    timespec_t simple_syscall_cost;
    timespec_t complex_syscall_cost;

    long tot_sec;
    long tot_nsec;

    int dummyCtr = 0;

    // Function call
    tot_sec = 0;
    tot_nsec = 0;
    for (int i = 0; i < numIterations; i++) {
        clock_gettime(CLOCK_REALTIME, &start);
        empty_function();
        clock_gettime(CLOCK_REALTIME, &end);
        diff = calcDiff(start, end);

        tot_sec += diff.tv_sec;
        tot_nsec += diff.tv_nsec;
    }
    function_call_cost.tv_sec = tot_sec / numIterations;
    function_call_cost.tv_nsec = tot_nsec / numIterations;

    // Simple system call
    volatile pid_t id;
    tot_sec = 0;
    tot_nsec = 0;
    for (int i = 0; i < numIterations; i++) {
        clock_gettime(CLOCK_REALTIME, &start);
        
        id = getpid();

        clock_gettime(CLOCK_REALTIME, &end);
        diff = calcDiff(start, end);

        dummyCtr += id;

        tot_sec += diff.tv_sec;
        tot_nsec += diff.tv_nsec;
    }
    simple_syscall_cost.tv_sec = tot_sec / numIterations;
    simple_syscall_cost.tv_nsec = tot_nsec / numIterations;

    // Complex system call
    tot_sec = 0;
    tot_nsec = 0;
    for (int i = 0; i < numIterations; i++) {
        clock_gettime(CLOCK_REALTIME, &start);
        
        id = fork();

        clock_gettime(CLOCK_REALTIME, &end);

        if (id == 0) {
            exit(1);
        }

        dummyCtr += id;

        diff = calcDiff(start, end);

        tot_sec += diff.tv_sec;
        tot_nsec += diff.tv_nsec;
    }
    complex_syscall_cost.tv_sec = tot_sec / numIterations;
    complex_syscall_cost.tv_nsec = tot_nsec / numIterations;

    printf("Empty function call: ");
    printTime(function_call_cost);
    printf("Simple system call: ");
    printTime(simple_syscall_cost);
    printf("Complex system call: ");
    printTime(complex_syscall_cost);

    return 0;
}

