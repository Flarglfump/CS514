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

pthread_mutex_t lock1;
pthread_t threads[NUMTHREADS] = {0};
int thread_ids[NUMTHREADS] = {0};
int circle_points = 0;
int square_points = 0;

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
    double pi = 4 * (double) (circle_points) / square_points;
    printf("Estimated value of pi: %.7g\n", pi);

    return 0;
}

static void * thread_func(void *arg) {
    int thread_id = *((int*) arg);
    printf("Hello from thread %d!\n", thread_id);

    double circle_points_local = 0, square_points_local = NUMPAIRS, origin_dist;

    struct drand48_data randbuf;
    srand48_r(thread_id, &randbuf);
    double rand_x, rand_y;
    for (int i = 0; i < NUMPAIRS; i++) {
        drand48_r(&randbuf, &rand_x);
        drand48_r(&randbuf, &rand_y);

        origin_dist = rand_x * rand_x + rand_y * rand_y;
        if (origin_dist <= 1) {
            circle_points_local++;
        }
    }

    /* Critical Section Start */
    pthread_mutex_lock(&lock1);
    circle_points += circle_points_local;
    square_points += square_points_local;
    pthread_mutex_unlock(&lock1);
    /* Critical Section End */
    
    printf("Goodbye from thread %d!\n", thread_id);
}