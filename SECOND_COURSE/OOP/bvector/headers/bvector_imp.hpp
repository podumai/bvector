#ifndef __BITS_BVECTOR_IMPLEMENTATION__
#define __BITS_BVECTOR_IMPLEMENTATION__ 1

#include <cinttypes>
#include <cstddef>

// Bit masks for bit manipulations
#define __BITS_BMASK_BIT__ (0b10000000)
#define __BITS_BMASK_TRUE_BYTE__ (0b11111111)
#define __BITS_BMASK_FALSE_BYTE__ (0b00000000)
#define __BITS_BMASK_MOD__ (0b00000111)
#define __BITS_DEFBITS_PER_BYTE__ (8)

// inline functions to compute results
#define calculate_capacity(size) (((size) >> 3) + ((size) & 0b00000111 ? 1 : 0))
#define byte_division(size) ((size) >> 3)
#define byte_module(size) ((size) & 0b00000111)

#define min(var1, var2) ((var1) > (var2) ? (var1) : (var2))

#endif
