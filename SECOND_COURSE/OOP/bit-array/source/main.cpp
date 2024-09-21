#include <cinttypes>
#include <iostream>
#include <limits>
#include "bit_array.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  try
  {
    bits::bit_array test1 (12, 0xFF);
    bits::bit_array test2 (~test1);
    std::cout << "Printing test1...\n"
              << test1.to_string() << std::endl
              << "Printing test2...\n"
              << test2.to_string() << std::endl;
  }
  catch (std::out_of_range &err)
  {
    std::cout << "Catched error: " << err.what() << std::endl;
  }

  return 0;
}
