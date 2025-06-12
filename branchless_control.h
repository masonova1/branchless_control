#ifndef BRANCHLESS_CONTROL_INCLUDED
#define BRANCHLESS_CONTROL_INCLUDED

// MIT No Attribution
//
// Copyright 2025 Mason Watmough
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// On the fundamental implementation of conditional and iterative statements.

// We will show that all conditional-iterative procedures can be reduced into
// unconditional-recursive procedures by explicit implementation. The first
// step is to build arithmetic multiplexing, which implements binary selection
// without the usual "party tricks" in such demonstrations like the ternary
// operator. Then, the "abstractions" of conditional if..then and iterative
// 'while' structures can be made using function pointers, which are perhaps the
// most powerful static type in all of computer science.

// Conditional operations are usually difficult to directly parallelize,
// because processors often relegate the duty of evaluating some comparison
// to speculative execution or special digital comparator hardware, which might
// be a dependency-linked or thread-stretched resource. However, almost all
// multithreading schemes give each core its own ALU, which we will exploit
// for demonstration here. To begin, we have to figure out how to reframe
// conditional statements into procedural arithmetic (in essence, placing the
// 'duty' of condition evaluation onto the ALU.) Standard C actually gives us
// all the tools we need.

// To begin, we'll assume the machine is a twos-complement arithmetic computer,
// and that the highest bit will be 1 if the number is strictly less than zero.

// Return all-ones mask if X sign bit is set

// The unsigned right shift should yield 0 or 1 for unsigned types, and
// something implementation-defined for signed types. So, this is only
// well-defined for strictly unsigned types. Note that we do not require signed
// right-shift behavior, and so this is completely defined behavior for unsigned
// types (and thus our demonstrated use case.)

// One of the most powerful features of ALU work is that it happens out-of-band
// of memory, avoiding congestions. A hypothetical architecture specialized
// for executing this very well would have minimal control logic and a
// highly-optimized pipeline for selection sequences and packed arithmetic.

// This is, perhaps, a compiler implementation tool: these routines will enable
// conditional evaluation on anything that can do integer math, leaving them as
// a backup for almost any architecture.

#ifndef CHAR_BIT
#define CHAR_BIT (8) // Educated guess
#endif

#define _msign(X) ( (typeof((X)) )( -( (X) >> (sizeof((X))*CHAR_BIT-1)) ) )

// It is possible on most architectures to use an arithmetic right shift on a
// signed type to produce a mask faster. Unfortunately, C does not guarantee
// this behavior, so we must do it this way for absolute portability. However,
// if the implementation behavior is known to do this, we can substitute the
// expression:

// #define _msign(X) ((typeof((X)))(((signed)(X) >> (sizeof((X))*CHAR_BIT-1))))

// Return all-ones mask if X nonzero

// We can use bitwise tricks to fill the bits upwards from the lowest-set-bit up
// to the sign bit. The only integer
// which will leave the sign bit low is 0.
#define _mnez(X) _msign( (typeof((X)))((X) | (-(X))) )

// Addition and XOR can both be used as invertible operations for the
// multiplexing function.

// #define ADD_MULTIPLEX

// Multiplex operation
#ifndef ADD_MULTIPLEX
  #define _mux(X, A, B) ((((A) ^ (B)) & ((typeof((A)))(X))) ^ (B))
  // #define _mux(X, A, B) ((~((typeof((A)))(X)) & (B)) | (((typeof((A)))(X)) & (A)))
#else
  #define _mux(X, A, B) ((((A) - (B)) & ((typeof((A)))(X))) + (B))
#endif

// Ternary operator replacements

// (X != 0) ? A : B
#define selnez(X, A, B) _mux(_mnez((typeof((A)))(X)), (A), (B))

// (X < 0) ? A : B
#define selltz(X, A, B) _mux(_msign((typeof((A)))(X)), (A), (B))

// (X == Y) ? A : B
#define seleq(X, Y, A, B) selnez(((X) ^ (Y)), (A), (B))

// (X < Y) ? A : B
#define sellt(X, Y, A, B) selltz(((X) - (Y)), (A), (B))

// (X != Y) ? A : B
#define selne(X, Y, A, B) seleq((X), (Y), (B), (A))

// (X > Y) ? A : B
#define selgt(X, Y, A, B) sellt((Y), (X), (A), (B))

// (X <= Y) ? A : B
#define selle(X, Y, A, B) selgt((X), (Y), (B), (A))

// (X >= Y) ? A : B
#define selge(X, Y, A, B) sellt((X), (Y), (B), (A))

const static inline void empty_clause() { return; } // Don't do anything

// if the condition 'c' is nonzero, the success procedure 'cs' is taken.
// Otherwise, the "else clause" procedure 'ce' is taken.
const static inline void branchless_if(const unsigned int c, \
  void (*cs)(), void (*ce)()) {
  ((void(*)())selnez(c, (uintptr_t)cs, (uintptr_t)ce))();
  return;
}

const static inline void end_while(const unsigned int (*cond)(), \
const void (*f)()) {
  // We don't need to do anything
  (void)cond;
  (void)f;
  return;
}

const static inline void branchless_do_while( \
const unsigned int (*_condition)(), const void (*f)()) {
  f(); // Do the operation
  // Select whether or not we continue iteration
  ((void(*)(const unsigned int (*)(), const void (*)))selnez( \
    _condition(), (uintptr_t)branchless_do_while, (uintptr_t)end_while))(_condition, f); \
  return; /* Undecidable reachability */
}

const static inline void branchless_while(const unsigned int (*_condition)(), \
const void (*f)()) {
  // Check if the condition is satisfied, and continue as regular do-while
  ((void(*)(const unsigned int (*)(), const void (*)))selnez( \
    _condition(), (uintptr_t)branchless_do_while, (uintptr_t)end_while))(_condition, f); \
  return; /* Undecidable reachability */
}

const static inline void end_for(const unsigned int (*_condition)(), \
const void (*expression)(), \
const void (*f)()) {
  // We don't need to do anything
  (void)_condition;
  (void)expression;
  (void)f;
  return;
}

const static inline void branchless_continue_for(const unsigned int (*_condition)(), \
const void (*expression)(), \
const void (*f)()) {
  f();
  expression();
  ((void(*)(const unsigned int (*)(), const void (*), const void (*)))selnez( \
    _condition(), (uintptr_t)branchless_continue_for, (uintptr_t)end_for))(_condition, expression, f); \
  return; /* Undecidable reachability */
}

const static inline void branchless_for(const void (*init)(), \
const unsigned int (*_condition)(), \
const void (*expression)(), \
const void (*f)()) {
  init();
  ((void(*)(const unsigned int (*)(), const void (*), const void (*)))selnez( \
    _condition(), (uintptr_t)branchless_continue_for, (uintptr_t)end_for))(_condition, expression, f); \
  return; /* Undecidable reachability */
}

#endif /* BRANCHLESS_CONTROL_INCLUDED */
