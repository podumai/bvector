#include <cinttypes>
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cstddef>

template<class T, class U>
class Conversion
{
  typedef char Small;
  class Big { char dummy[2]; };
  static Small Test(const U &);
  static Big Test(...);
  static T MakeT();
public:
  enum { exists = sizeof(Test(MakeT())) == sizeof(Small) };
};

std::int32_t main()
{
  std::cout.setf(std::ios::boolalpha);
  std::cout << Conversion<double, int>::exists << ' '
            << Conversion<char, char*>::exists << ' '
            << Conversion<std::size_t, std::vector<int>>::exists << std::endl;

  return 0;
}
