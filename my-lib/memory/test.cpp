#include <iostream>
#include <cstdint>
#include "memory.hpp"

class simple_class
{
private:
  std::int32_t i;
  float b;
public:
  std::int32_t get_i() const { return this->i; }
  float get_b() const { return this->b; }
  simple_class(std::int32_t var_i, float var_b) :
  i (var_i), b (var_b) {}
  simple_class() : i (0), b (0.0F) {}
};

std::int32_t main()
{
  local::memory::auto_ptr<simple_class> test (new simple_class(5, 5.0F));

  std::cout << "Test pointer: " << (*test).get_i() << ' '
            << (*test).get_b() << std::endl;

  test.swap(new simple_class(10, 10.0F));

  std::cout << "Test pointer: " << (*test).get_i() << ' '
            << (*test).get_b() << std::endl;

  return 0;
}
