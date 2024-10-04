#include <cinttypes>
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cstddef>
#include "bvector.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  bits::bvector test;
  for (std::size_t i {}; i < 1000; ++i)
  {
    test.push_back(true);
  }
  std::cout << (test.count() == 1000) << std::endl;

  return 0;
}
