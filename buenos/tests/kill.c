/*
 * Userland exec test.
 */

#include "tests/lib.h"

// No timer so just do something wait
#define wait_clock 100000

void read() {
  char c;
  syscall_read(stdin, &c, 1);
}

void wait(int time) {

  int ct = 0; int i = 0;
  while(ct < time) {
    i = 0;
    while(i++ < wait_clock); {
      ct++;
      printf("killing process in %d time...\n", time - ct);
    }
  }
}

void run_program(char *program, int time) {
  uint32_t pid = syscall_exec(program);
  printf("\nkilling process in %d time...\n\n", time);
  wait(time);

  syscall_kill(pid, 0);
  printf("Press any key to continue...\n\n");
  read();
}

int main() {
  run_program("[disk]exec_child1", 10);
  run_program("[disk]exec_child2", 5);
  return 0;
}
