#ifndef OSM2015_FG_QUEUE_H
#define OSM2015_FG_QUEUE_H

#include <pthread.h>


typedef struct fg_node {
  void* item;
  struct fg_node* next;
} fg_node_t;

typedef struct {
  fg_node_t* head;
  fg_node_t* tail;
  pthread_mutex_t head_lock; /* for get */
  pthread_mutex_t tail_lock; /* for put */
} fg_queue_t;

int fg_queue_init(fg_queue_t *queue);
int fg_queue_put(fg_queue_t *queue, void *item);
void* fg_queue_get(fg_queue_t *queue);
int fg_queue_destroy(fg_queue_t *queue);

#endif
