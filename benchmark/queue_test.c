#include "fg_queue.h"
#include "cg_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

// The number of items to producer/consume
#define MAX 5000000

/**
 * producer function for the fine grained queue
 * @param  queue the fg queue
 * @return       null
 */
void *fg_producer(void* queue) {

  // Insert MAX items into queue
  for (long i = 0; i < MAX; ++i)
    fg_queue_put(queue, (void*)1);

  pthread_exit(NULL);
}

/**
 * consumer function for the fine grained queue
 * @param  queue the queue
 * @return       null
 */
void *fg_consumer(void* queue) {

  // Pop all items from the queue
  for(int i = 1; i < MAX;)
    if ((fg_queue_get(queue) != NULL)) ++i;

  pthread_exit(NULL);
}

/**
 * producer function for the coarse grained queue
 * @param  queue the cg queue
 * @return       null
 */
void *cg_producer(void* queue) {

  // Insert MAX items into queue
  for (long i = 0; i < MAX; ++i)
    cg_queue_put(queue, (void*)1);

  pthread_exit(NULL);
}

/**
 * consumer function for the coarse grained queue
 * @param  queue the queue
 * @return       null
 */
void *cg_consumer(void* queue) {

  // Pop all items from the queue
  for(int i = 1; i < MAX;)
    if ((cg_queue_get(queue) != NULL)) ++i;

  pthread_exit(NULL);
}

/**
 * Helper function for benchmarking. Creates threads and measures performance
 * @param t number of threads
 * @param producer the producer function pointer
 * @param consumer the consumer function pointer
 * @param arg the function argument
 * returns the benchmarking duration in seconds
 */
float benchmark_start(int t, void *(*producer) (void *), void *(*consumer) (void *), void* arg) {

  // Setup timers
  clock_t start_t, end_t;
  float total;

  // Setup array of threads
  pthread_t producer_threads[t];
  pthread_t consumer_threads[t];
  int s, s0;

  // Start timer
  start_t = clock();

  // Create the threads and start benchmarking
  for (int i = 0; i < t; ++i) {
    s = pthread_create(&producer_threads[i], NULL, producer, arg);
    s0 = pthread_create(&consumer_threads[i], NULL, consumer, arg);
    if (s != 0 || s0 != 0) {
      printf("Error creating thread.");
      exit(-1);
    }
  }

  // Synchronize threads
  for (int i = 0; i < t; i++) {
    pthread_join(producer_threads[i], 0);
    pthread_join(consumer_threads[i], 0);
  }

  // Calculate the benchmark duration in seconds
  end_t = clock();
  total = (double)(end_t - start_t) / CLOCKS_PER_SEC;

  return total;
}

/**
 * Performs a benchmark on fine grained queue
 * @param t number of threads
 */
void fg_benchmark(int t) {
  fg_queue_t queue;
  fg_queue_init(&queue);
  float dur = benchmark_start(t, &fg_producer, &fg_consumer, &queue);
  fg_queue_destroy(&queue);
  printf("%-10s%-10d%-10d%-10f\n", "FG", t, MAX, dur);
}

/**
 * Performs a benchmark on coarse grained queue
 * @param t number of threads
 */
void cg_benchmark(int t) {
  cg_queue_t queue;
  cg_queue_init(&queue);
  float dur = benchmark_start(t, &cg_producer, &cg_consumer, &queue);
  cg_queue_destroy(&queue);
  printf("%-10s%-10d%-10d%-10f\n", "CG", t, MAX, dur);
}

int main(int argc, char const *argv[]) {

  // Pretty print for results
  printf("%-10s%-10s%-10s%-10s\n", "Queue", "Threads", "Items", "Time (seconds)");

  int q = 2; // The number of threads

  // Start benchmarking fine grained
  for (int i = 1; i <= q; i *= 2) {
    fg_benchmark(i);
  }

  printf("\n\n------------------------------------------------------------\n\n");

  // Benchmark coarse grained
  for (int i = 1; i <= q; i *= 2) {
    cg_benchmark(i);
  }
  return 0;
}
