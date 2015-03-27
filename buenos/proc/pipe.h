#ifndef BUENOS_PIPE_H
#define BUENOS_PIPE_H

#include "lib/types.h"

#define PIPE_BUFFER_LENGTH 256
#define MAX_PIPES 20

typedef uint32_t pipe_id_t;

typedef struct {
  char buffer[PIPE_BUFFER_LENGTH];
  uint32_t length;
  int read_end;
  int write_end;
  int write_id;
  int read_id;
  int free;
  pipe_id_t id;
} pipe_t;

void pipe_init();
pipe_t* pipe_get_pipe(int fd);
void pipe_write(char* buffer, int length, pipe_t* pipe);
void pipe_read(void* buffer, int length, pipe_t* pipe);
int pipe_pipe(int fds[2]);
int pipe_dup(int oldfd, int newfd);

#endif
