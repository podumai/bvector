#include <iostream>
#include <vector>
#include <iomanip>
#include <cstddef>
#include "bvector.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  bits::bvector test1;
  size_t size { __BITS_BVECTOR_MAX_SIZE__ >> 2 };
  std::cout << size << ' ' << __BITS_BVECTOR_MAX_SIZE__ << std::endl;
  /*for (size_t i {}; i < size; ++i)
  {
    test1.push_back(true);
  }*/
  test1.resize(size, true);
  //std::vector<bool> test1;
  //test1.resize(size, true);
  //std::cout << (test1.count() == size) << std::endl;
  //test1.resize(INT32_MAX, true); test2.resize(INT32_MAX, true);

  return 0;
}
