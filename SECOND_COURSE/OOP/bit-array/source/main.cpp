#include <cinttypes>
#include <iostream>
#include <vector>
#include <iomanip>
#include "bit_array.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  
  bits::bit_array test1;
  //test1.reserve(268'435'456);

  for (std::int32_t i {}; i < INT32_MAX; ++i)
  {
    test1.push_back(true);
  }

  std::cout << test1.count() << std::endl;
  std::cout << "max_size: " << test1.max_size() << std::endl;
  
  return 0;
}
