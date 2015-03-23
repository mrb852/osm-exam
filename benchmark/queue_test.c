#include "fg_queue.h"
#include "cg_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

#define MAX 500000

void *fg_producer(void* queue) {
  for (long i = 0; i < MAX; ++i)
    fg_queue_put(queue, (void*)1);

  pthread_exit(NULL);
}

void *fg_consumer(void* queue) {
  for(int i = 1; i < MAX;)
    if ((fg_queue_get(queue) != NULL)) ++i;

  pthread_exit(NULL);
}

void *cg_producer(void* queue) {
  for (long i = 0; i < MAX; ++i)
    cg_queue_put(queue, (void*)1);

  pthread_exit(NULL);
}

void *cg_consumer(void* queue) {
  for(int i = 1; i < MAX;)
    if ((cg_queue_get(queue) != NULL)) ++i;

  pthread_exit(NULL);
}

float benchmark_start(int t, void *(*producer) (void *), void *(*consumer) (void *), void* arg) {
  clock_t start_t, end_t;
  float total;
  pthread_t producer_threads[t];
  pthread_t consumer_threads[t];
  int s, s0;
  start_t = clock();
  for (int i = 0; i < t; ++i) {
    s = pthread_create(&producer_threads[i], NULL, producer, arg);
    s0 = pthread_create(&consumer_threads[i], NULL, consumer, arg);
    if (s != 0 || s0 != 0) {
      printf("Error creating thread.");
      exit(-1);
    }
  }

  for (int i = 0; i < t; i++) {
    pthread_join(producer_threads[i], 0);
    pthread_join(consumer_threads[i], 0);
  }

  end_t = clock();
  total = (double)(end_t - start_t) / CLOCKS_PER_SEC;

  return total;
}

void fg_benchmark(int t) {
  fg_queue_t queue;
  fg_queue_init(&queue);
  float dur = benchmark_start(t, &fg_producer, &fg_consumer, &queue);
  fg_queue_destroy(&queue);
  printf("%-10s%-10d%-10d%-10f\n", "FG", t, MAX, dur);
}

// CG Benchmark

void cg_benchmark(int t) {
  cg_queue_t queue;
  cg_queue_init(&queue);
  float dur = benchmark_start(t, &cg_producer, &cg_consumer, &queue);
  cg_queue_destroy(&queue);
  printf("%-10s%-10d%-10d%-10f\n", "CG", t, MAX, dur);
}

int main(int argc, char const *argv[]) {

  printf("%-10s%-10s%-10s%-10s\n", "Queue", "Threads", "Items", "Time (seconds)");

  for (int i = 1; i <= 2; i *= 2) {
    fg_benchmark(i);
  }

  printf("\n\n------------------------------------------------------------\n\n");

  for (int i = 1; i <= 2; i *= 2) {
    cg_benchmark(i);
  }
  return 0;
}
