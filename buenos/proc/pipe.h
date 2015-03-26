#ifndef BUENOS_PIPE_H
#define BUENOS_PIPE_H

#include "lib/types.h"

#define PIPE_BUFFER_LENGTH 256
#define MAX_PIPES 20

typedef uint32_t pipe_id_t;

typedef struct {
  semaphore_t *w_lock;
  semaphore_t *r_lock;
  char buffer[PIPE_BUFFER_LENGTH];
  uint32_t length;
  int read_end;
  int write_end;
  int free;
  int is_empty;
} pipe_t;

void pipe_init();
int pipe_pipe(int fds[2]);
int pipe_dup(int oldfd, int newfd);

#endif
