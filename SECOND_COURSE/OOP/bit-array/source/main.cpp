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

  bits::bvector test1;
  test1.resize(INT32_MAX, true);
  for (std::size_t i {}; i < INT32_MAX; ++i)
  {
    test1.push_back(true);
  }
  std::cout << test1.count() << std::endl;
  /*try
  {
    std::cout << test.to_string() << std::endl;
  }
  catch (std::bad_alloc &err)
  {
    std::cerr << "  what(): " << err.what() << std::endl;
  }*/

  return 0;
}
