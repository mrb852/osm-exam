#ifndef OSM2015_CG_QUEUE_H
#define OSM2015_CG_QUEUE_H

#include <pthread.h>


typedef struct cg_node {
  void* item;
  struct cg_node* next;
} cg_node_t;

typedef struct {
  cg_node_t* head;
  cg_node_t* tail;
  pthread_mutex_t global_lock; /* for get and put */
} cg_queue_t;

int cg_queue_init(cg_queue_t *queue);
int cg_queue_put(cg_queue_t *queue, void* item);
void* cg_queue_get(cg_queue_t *queue);
int cg_queue_destroy(cg_queue_t *queue);

#endif
