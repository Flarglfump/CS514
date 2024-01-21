/* Author: Gavin Witsken
 * Program: Threaded Monte Carlo Estimation
 * File: montecarlo.c
 * Last modified: 2:24 PM, 21 January 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUMTHREADS 20
#define NUMPAIRS 10000

static void * thread_func(void *arg);

pthread_t threads[NUMTHREADS] = {0};
int thread_ids[NUMTHREADS] = {0};

int main() {

    // Write thread ids into thread_ids array
    for (int i = 0; i < NUMTHREADS; i++) {
        thread_ids[i] = i;
    }

    // Spin up threads
    for (int i = 0; i < NUMTHREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    // Join on threads - synchronization barrier
    for (int i = 0; i < NUMTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads complete!\n");

    return 0;
}

static void * thread_func(void *arg) {
    int thread_id = *((int*) arg);
    printf("Hello! I am thread %d!\n", thread_id);
    printf("Goodbye! I am thread %d!\n", thread_id);
}