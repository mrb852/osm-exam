/*
 * System calls.
 *
 * Copyright (C) 2003 Juha Aatrokoski, Timo Lilja,
 *   Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: syscall.c,v 1.3 2004/01/13 11:10:05 ttakanen Exp $
 *
 */
#include "fs/vfs.h"
#include "kernel/cswitch.h"
#include "kernel/halt.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "proc/io.h"
#include "proc/syscall.h"
#include "proc/process.h"
#include "kernel/assert.h"
#include "drivers/device.h"
#include "drivers/gcd.h"
#include "proc/usr_sem.h"
#include "proc/pipe.h"

#define A0 user_context->cpu_regs[MIPS_REGISTER_A0]
#define A1 user_context->cpu_regs[MIPS_REGISTER_A1]
#define A2 user_context->cpu_regs[MIPS_REGISTER_A2]
#define A3 user_context->cpu_regs[MIPS_REGISTER_A3]
#define V0 user_context->cpu_regs[MIPS_REGISTER_V0]

int syscall_kill(int32_t pid, int retval) {
  return process_kill(pid, retval);
}

/**
 * Generates a pseudo random number between 0 and range - 1
 * @param  range the maximum output + 1
 * @return a pseudo generated random number between 0 and range - 1
 */
uint32_t syscall_rand(uint32_t range) {
  return _get_rand(range);
}

/**
 * Handle system calls. Interrupts are enabled when this function is
 * called.
 *
 * @param user_context The userland context (CPU registers as they
 * where when system call instruction was called in userland)
 */
void syscall_handle(context_t *user_context)
{
  /* When a syscall is executed in userland, register a0 contains
   * the number of the syscall. Registers a1, a2 and a3 contain the
   * arguments of the syscall. The userland code expects that after
   * returning from the syscall instruction the return value of the
   * syscall is found in register v0. Before entering this function
   * the userland context has been saved to user_context and after
   * returning from this function the userland context will be
   * restored from user_context.
   */

  switch (A0) {
  case SYSCALL_HALT:
    halt_kernel();
    break;

    /* Process management */
  case SYSCALL_EXEC:
    V0 = process_spawn((char*) A1);
    break;
  case SYSCALL_EXIT:
    process_finish((int) A1);
    break;
  case SYSCALL_JOIN:
    V0 = process_join((process_id_t) A1);
    break;
  case SYSCALL_FORK:
    V0 = process_fork();
    break;
  case SYSCALL_GETPID:
    V0 = process_get_current_process();
    break;
  case SYSCALL_RAND:
    V0 = syscall_rand((uint32_t)A1);
  break;
  case SYSCALL_KILL:
    V0 = syscall_kill((int32_t)A1, (int)A2);
  break;

    /* Memory allocation */
  case SYSCALL_MEMLIMIT:
    V0 = process_memlimit(A1);
    break;

    /* File I/O */
  case SYSCALL_OPEN:
    V0 = (uint32_t) io_open((const char*) A1);
    break;
  case SYSCALL_CLOSE:
    V0 = io_close((openfile_t) A1);
    break;
  case SYSCALL_CREATE:
    V0 = io_create((const char*) A1, (int) A2);
    break;
  case SYSCALL_REMOVE:
    V0 = io_remove((const char*) A1);
    break;
  case SYSCALL_SEEK:
    V0 = io_seek((openfile_t) A1, (int) A2);
    break;
  case SYSCALL_TELL:
    V0 = io_tell((openfile_t) A1);
    break;
  case SYSCALL_READ:
    V0 = io_read((openfile_t) A1, (void*) A2, (int) A3);
    break;
  case SYSCALL_WRITE:
    V0 = io_write((openfile_t) A1, (void*) A2, (int) A3);
    break;
  case SYSCALL_PIPE:
    V0 = pipe_pipe((int *)A1);
    break;
  case SYSCALL_DUP:
    V0 = pipe_dup((int)A1, (int)A2);
    break;

    /* User semaphores */
  case SYSCALL_SEM_OPEN:
    V0 = (uint32_t) usr_sem_open((char*) A1, A2);
    break;
  case SYSCALL_SEM_PROCURE:
    V0 = usr_sem_p((usr_sem_t*) A1);
    break;
  case SYSCALL_SEM_VACATE:
    V0 = usr_sem_v((usr_sem_t*) A1);
    break;
  case SYSCALL_SEM_DESTROY:
    V0 = usr_sem_destroy((usr_sem_t*) A1);
    break;

  default:
    KERNEL_PANIC("Unhandled system call\n");
  }

  /* Move to next instruction after system call */
  user_context->pc += 4;
}
