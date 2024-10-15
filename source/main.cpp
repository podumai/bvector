#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <cstddef>
#include <cinttypes>
#include "xallocator.hpp"
#include "bvector.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);

  constexpr std::size_t SIZE { INT32_MAX };

  std::cout << SIZE << std::endl;

  bits::bvector test;
  for (std::size_t i {}; i != SIZE; ++i)
    test.push_back(true);

  return 0;
}
