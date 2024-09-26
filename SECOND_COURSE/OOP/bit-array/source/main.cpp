#include <cinttypes>
#include <iostream>
#include <limits>
#include "bit_array.hpp"

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  try
  {
    bits::bit_array temp (10);
    temp.set(3, true);
    std::cout << temp.to_string() << ' ' << temp.count() << std::endl;
  }
  catch (std::out_of_range &err)
  {
    std::cerr << "Catched an error: " << err.what() << std::endl;
  }

  return 0;
}
