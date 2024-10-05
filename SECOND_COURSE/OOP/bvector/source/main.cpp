#include <iostream>
#include <vector>
#include <iomanip>
#include <cstddef>
#include "bvector.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  bits::bvector test;
  const std::size_t var { _BITS_BVECTOR_MAX_SIZE_ >> 3 };
  //std::cout << var << std::endl;
  //test.reserve(_BITS_BVECTOR_MAX_CAPACITY_ >> 3);
  //std::vector<bool> test;
  //test.resize(UINT32_MAX, false);
  std::cout << var << std::endl;
  //test.reserve(_BITS_BVECTOR_MAX_CAPACITY_ >> 4);
  for (std::size_t i {}; i < var; ++i)
  {
    test.push_back(true);
  }
  std::cout << test.count() << std::endl;
  //std::cout << _WORD_LEN_ << std::endl;

  return 0;
}
