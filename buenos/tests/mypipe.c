#include "tests/lib.h"

int main() {
  char buf[143];
  int ofds[2];
  (void) syscall_pipe(ofds);
  syscall_dup(ofds[1], 1);
  syscall_dup(ofds[0], 0);

  syscall_write(ofds[1], "Hello World\n", sizeof("Hello World") + 1);
  syscall_read(ofds[0], buf, sizeof(buf));
  // printf("buf: %s\n", buf);
  return 0;
}
