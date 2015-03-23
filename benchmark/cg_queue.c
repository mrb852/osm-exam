/* A thread-safe coarse-grained queue. */

// #include <error.h> // perror
#include <errno.h> // ENOMEM
#include <stdio.h> // printf
#include <stdlib.h> // malloc
#include <pthread.h> // pthread_*

#include "cg_queue.h"


/**
 * Initialize the queue. Must be called before calling get or put.
 *
 * Returns: 0 on success; nonzero on error. See also the manual page for
 * pthread_mutex_init(3).
 */
int cg_queue_init(cg_queue_t* q) {
  int ret;

  /* We store a dummy element to avoid handling some special cases. */
  cg_node_t *new = malloc(sizeof(cg_node_t));
  if (new == NULL) {
    return ENOMEM;
  }

  new->next = NULL;
  q->head = q->tail = new;

  ret = pthread_mutex_init(&q->global_lock, NULL);
  if (ret != 0) {
    free(new);
  }
  return ret;
}

static inline void unlock_or_die(pthread_mutex_t *mtx) {
  if (pthread_mutex_unlock(mtx)) {
    perror("Couldn't unlock held mutex.\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * Insert a node at the tail of the queue.
 *
 * Returns: 0 on success; nonzero on error. See also the manual page for
 * pthread_mutex_lock(). Kills the host if the mutex locked in this function
 * cannot be unlocked.
 */
int cg_queue_put(cg_queue_t* q, void* item) {
  int ret;

  /* Allocate memory to a node and assign its elements. */
  cg_node_t* new = malloc(sizeof(cg_node_t));
  if (new == NULL) {
    return ENOMEM;
  }

  new->item = item;
  new->next = NULL;

  /* Add the new node to the global. */
  ret = pthread_mutex_lock(&q->global_lock);
  if (ret != 0) {
    return ret;
  }

  q->tail->next = new;
  q->tail = new;

  unlock_or_die(&q->global_lock);

  return 0;
}

/**
 * Remove an element from the front of the queue, if queue is nonempty.
 *
 * Returns: a pointer to the value of the node at the head of the queue; NULL
 * if the queue is empty, or an error occurred. Kills the host if the mutex
 * locked in this function cannot be unlocked.
 */
void* cg_queue_get(cg_queue_t* q) {
  int ret;
  void *item;
  cg_node_t *old;

  ret = pthread_mutex_lock(&q->global_lock);
  if (ret != 0) {
    return NULL;
  }

  old = q->head;
  /* Handle the queue being empty: leave the removed element as a new dummy.*/
  if (old->next == NULL) {
    unlock_or_die(&q->global_lock);
    return NULL;
  }

  item = old->next->item;

  /* Update the head, and free the old memory. */
  q->head = old->next;
  free(old);
  unlock_or_die(&q->global_lock);

  return item;
}

/**
 * Destroy the queue. put and get must not be called after destroy. Unless
 * init has been called again.
 */
int cg_queue_destroy(cg_queue_t *q) {
  free(q->head);
  return pthread_mutex_destroy(&q->global_lock);
}
