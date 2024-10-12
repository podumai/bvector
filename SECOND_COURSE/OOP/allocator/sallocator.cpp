#include "sallocator.hpp"

void chunk::initialize(std::size_t block_size, std::uint8_t blocks)
{
  this->pdata_ = new std::uint8_t[block_size * blocks];
  this->first_available_block = 0;
  block_available = blocks;
  std::uint8_t i { 0 },
              *p { pdata_ };
  for (; i != blocks; p += block_size)
  {
    *p = ++i;
  }
}
