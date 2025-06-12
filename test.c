#include <stdio.h>
#include <stdint.h>

#include "branchless_control.h"

// Example

void f1() {
  printf("f1 (success clause) executed.\n\r");
  return;
}

void f2() {
  printf("f2 (else clause) executed.\n\r");
  return;
}

int i;

void init_func() {
  i = 0;
}

void expr_func() {
  i++;
}

unsigned int cond() {
  // Simple iteration condition
  return sellt(i, 10, 1, 0); // i < 10;
}

void body() {
  printf("%i, ", i++);
  return;
}

int main(void) {
  branchless_if(0, f1, f2); // c false  ->  "f2 executed."
  branchless_if(1, f1, f2); // c true   ->  "f1 executed."

  int init = 10;
  // This:
    init_func();
    branchless_do_while(cond, body);
    printf("%i, ", i);
    printf("\n\r");

  // is equivalent to this:
    init_func();
    do { body(); } while(cond());
    printf("%i, ", i);
    printf("\n\r");

  // And this:
    init_func();
    branchless_while(cond, body);
    printf("%i, ", i);
    printf("\n\r");

  // is equivalent to this:
    init_func();
    while(cond()) { body(); }
    printf("%i, ", i);
    printf("\n\r");


    for(init_func(); cond(); expr_func()) { body(); }
    printf("%i, ", i);
    printf("\n\r");

    branchless_for(init_func, cond, expr_func, body);
    printf("%i, ", i);
    printf("\n\r");

  printf("done\n\r");
  return 0;
}
