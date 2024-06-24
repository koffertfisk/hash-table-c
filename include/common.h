#pragma once

#include <stdbool.h>

#define int_elem(x) (elem_t) { .i=(x) }
#define ptr_elem(x) (elem_t) { .p=(x) }
#define float_elem(x) (elem_t) { .f=(x) }
#define bool_elem(x) (elem_t) { .b=(x) }
#define unsigned_int_elem(x) (elem_t) { .u=(x) }

/**
 * @file common.h
 * @author Marcus Enderskog
 * @date 2021-04-15
 * @brief Utility for handling storing generic elements.
**/

typedef union elem elem_t;

/* Generic element that can be representend as a signed/unsigned integer, 
 * boolean, floating point number or generic pointer.
 */
union elem
{
  int i;
  unsigned int u;
  bool b;
  float f;
  void *p;
};
