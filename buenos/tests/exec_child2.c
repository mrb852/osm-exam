
#include "tests/lib.h"

#define wait_clock 1000

void wait(int time) {

  int ct = 0; int i = 0;
  while(ct < time) {
    i = 0;
    while(i++ < wait_clock); {
      ct++;
    }
  }
}

int main() {
  syscall_write(1, "Running executable 2\n\n", 36);
  while(1) {
    wait(100);
    syscall_write(1, "Myaaaaaahhhhhh\n\n", 36);
  };
  return 0;
}
