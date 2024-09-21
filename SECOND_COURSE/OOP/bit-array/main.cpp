#include <cinttypes>
#include <iostream>
#include <limits>
#include "bit_array.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  try
  {
    bits::bit_array test (16, 0xFFFF);
    std::cout << test.to_string() << std::endl;
    for (; test.size();)
    {
      std::cout << test.pop_back() << '\n';
    }
    std::cout << test.size() << ' ' << test.capacity() << '\n'
              << "Shrink...\n";
    test.shrink_to_fit();
    std::cout << test.size() << ' ' << test.capacity() << '\n';
    test.pop_back();
  }
  catch (std::out_of_range &err)
  {
    std::cout << "Catched error: " << err.what() << std::endl;
  }

  return 0;
}
