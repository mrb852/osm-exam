/**
 * This program tests the functionallity of the pseudo random
 * syscall implementation.
 */

#include "tests/lib.h"

int main() {

  // Gets random numbers with range from 0 to 40
  int i, r = 0;
  for (; i < 40; ++i) {
    r = syscall_rand(i);
    printf("syscall_rand(%d): %d\n", i, r);
    if (r >= i) {
      printf("ERROR: random is over range.");
    }
  }

  return 0;
}
