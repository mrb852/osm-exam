#include "drivers/gcd.h"
#include "fs/vfs.h"
#include "kernel/assert.h"
#include "proc/syscall.h"
#include "proc/pipe.h"
#include "proc/io.h"
#include "kernel/interrupt.h"

// A table of pipes for reference
pipe_t pipe_table[MAX_PIPES];

// Spinlock to protect the pipe table
spinlock_t pipe_table_slock;

/**
 * Resets a pipe
 * @param id the id for the pipe to reset
 */
void pipe_reset(pipe_id_t id) {

  pipe_table[id].free = 1;
  pipe_table[id].id = id;
  pipe_table[id].length = 0;

  // Set the read, write-id and end to id with an offset of 3
  pipe_table[id].read_id = 3 + (id * 2);
  pipe_table[id].write_id = 4 + (id * 2);
  pipe_table[id].read_end = pipe_table[id].read_id;
  pipe_table[id].write_end = pipe_table[id].write_id;

  // initialize the buffer
  pipe_table[id].buffer[0] = '\0';
}

/**
 * initializes the pipe subsystem
 */
void pipe_init() {
  spinlock_release(&pipe_table_slock);
  int i;
  for (i = 0; i < MAX_PIPES; i++) {
    pipe_reset(i);
  }
}

/**
 * Sets up a pipe with a filehandle for read end and write end
 * @param fds file descriptors for read end and write end. 0 1 respectively
 */
int pipe_pipe(int fds[2]) {
  // Find a free pipe
  int i;
  for(i = 0; i < MAX_PIPES; i++) {
    if(pipe_table[i].free) {
      pipe_table[i].free = 0;

      // Set the file descriptors
      fds[0] = pipe_table[i].read_id;
      fds[1] = pipe_table[i].write_id;
      return 0;
    }
  }
  kwrite("No available PIPES -1\n");
  return -1;
}

/**
 * Writes to a pipe
 * @param buffer the content to write
 * @param length the length of the buffer
 * @param pipe   the pipe to write to
 */
void pipe_write(char* buffer, int length, pipe_t* pipe) {

  // Protection
  interrupt_status_t intr_status = _interrupt_disable();
  spinlock_acquire(&pipe_table_slock);

  // Block while the buffer is full or are about to get full
  int data_to_write = (int)pipe->length + length;
  while(data_to_write >= PIPE_BUFFER_LENGTH) {
    spinlock_release(&pipe_table_slock);
    thread_switch();
    spinlock_acquire(&pipe_table_slock);
    data_to_write = (int)pipe->length + length;
    _interrupt_set_state(intr_status);
  }

  // update the pipe and write to it
  pipe->length += length;
  str_cat(pipe->buffer, buffer);

  spinlock_release(&pipe_table_slock);
  _interrupt_set_state(intr_status);
}

/**
 * Reads from the pipe
 * @param buffer the buffer to read into
 * @param length the amount to read
 * @param pipe   the pipe
 */
void pipe_read(void* buffer, int length, pipe_t* pipe) {
  // Protection
  interrupt_status_t intr_status = _interrupt_disable();
  spinlock_acquire(&pipe_table_slock);

  // Make sure we are not reading an empty pipe, and block if so
  int data_to_read = (int)pipe->length - length;
  while(data_to_read <= 0) {
    spinlock_release(&pipe_table_slock);
    thread_switch();
    spinlock_acquire(&pipe_table_slock);
    data_to_read = (int)pipe->length - length;
    _interrupt_set_state(intr_status);
  }

  // update the pipe and read from it
  pipe->length -= length;
  str_read(pipe->buffer, buffer, length);

  spinlock_release(&pipe_table_slock);
  _interrupt_set_state(intr_status);
}

/**
 * Gets a pipe by searching for a match on read_id or write_id
 * @param  fd the read or write id
 * @return    pipe
 */
pipe_t* pipe_get_pipe(int fd) {

  pipe_id_t i;
  for(i = 0; i < MAX_PIPES; i++) {
    pipe_t* pipe = &pipe_table[i];

    // If the pipe's write or read id matches the file descriptor
    // we have a match.
    if(pipe->write_id == fd || pipe->read_id == fd ) {
      return pipe;
    }
  }

  // No pipe found. Return null
  return NULL;
}

/**
 * Copies a pipe's file descriptor
 * @param  oldfd the old fd
 * @param  newfd the new fd
 * @return       0 on success, -1 on error
 */
int pipe_dup(int oldfd, int newfd) {

  pipe_t* pipe = pipe_get_pipe(oldfd);
  if (pipe) {
    if (pipe->write_id == oldfd) {
      pipe->write_end = newfd;
    } else if (pipe->read_id == oldfd) {
      pipe->read_end = newfd;
    }
    return 0;
  }
  return -1;
}
