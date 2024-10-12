#ifndef __BITS_SALLOCATOR_H__
#define __BITS_SALLOCATOR_H__ 1

#include <cinttypes>
#include <cstddef>

struct chunk
{
  std::uint8_t *pdata_,
               first_available_block_,
               blocks_available;

  void initialize(std::size_t, std::uint8_t);
  void *allocate(std::size_t);
  void deallocate(void *, std::size_t);
  void release();
};

#endif
