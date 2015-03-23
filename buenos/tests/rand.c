#include "tests/lib.h"

int main() {
  int i = 0;
  for (; i < 40; ++i) {
    printf("%d\n", syscall_rand(i));
  }

  return 0;
}
