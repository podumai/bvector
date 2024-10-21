#include <iostream>
#include <iomanip>
#include <string>
#include <cinttypes>
#include "bvector.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);

  constexpr std::size_t SIZE { INT32_MAX };

  std::cout << SIZE << std::endl;

  bit::bvector test;
  for (std::size_t i {}; i != SIZE; ++i)
    test.push_back(true);

  return 0;
}
