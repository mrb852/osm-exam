#include "drivers/gcd.h"
#include "fs/vfs.h"
#include "kernel/assert.h"
#include "proc/syscall.h"
#include "proc/pipe.h"
#include "proc/io.h"

pipe_t pipe_table[MAX_PIPES];
spinlock_t pipe_table_slock;

void pipe_reset(pipe_id_t id) {
  pipe_table[id].free = 1;
  semaphore_t* w_sem;
  semaphore_t* r_sem;
  w_sem = semaphore_create(1);
  r_sem = semaphore_create(1);
  if (w_sem == NULL || r_sem == NULL) {
    kprintf("pipe_init: unable to create lock semaphore. Pipes might not work\n");
    return;
  }

  pipe_table[id].w_lock = w_sem;
  pipe_table[id].r_lock = r_sem;
  pipe_table[id].is_empty = 1;
  pipe_table[id].length = 0;
  pipe_table[id].read_end = 3 + (id * 2);
  pipe_table[id].write_end = 4 + (id * 2);

}

void pipe_init() {
  spinlock_reset(&pipe_table_slock);
  pipe_id_t i;
  for (i = 0; i < MAX_PIPES; i++) {
    pipe_reset(i);
  }
}

int pipe_pipe(int fds[2]) {
  spinlock_reset(&pipe_table_slock);
  pipe_id_t i;
  for(i = 0; i < MAX_PIPES; i++) {
    if(pipe_table[i].free) {
      pipe_table[i].free = 0;
      fds[0] = pipe_table[i].read_end;
      fds[1] = pipe_table[i].write_end;
      return 0;
    }
  }
  return -1;
}

int pipe_dup(int oldfd, int newfd) {
  oldfd = oldfd; newfd = newfd;
  // spinlock_reset(&pipe_table_slock);
  // pipe_id_t i;
  // for(i = 0; i < MAX_PIPES; i++) {
  //   if(pipe_table[i].write_end == oldfd) {
  //     io_close(newfd);
  //     pipe_table[i].write_end = newfd;
  //     return 0;
  //   }
  //   else if(pipe_table[i].read_end == oldfd) {
  //     io_close(newfd);
  //     pipe_table[i].read_end = newfd;
  //     return 0;
  //   }
  // }
  return 0;
}
