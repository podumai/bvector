#include <iostream>
#include "matrix.hpp"

typedef struct
{
  std::int8_t x, y;
} Point;

std::int32_t main()
{
  my_lib::linear_algebra::matrix<my_lib::linear_algebra::f32> temp (3),
                                        test (3);
  temp.fill(1.0F);
  test.fill(1.0F);

  my_lib::linear_algebra::matrix<my_lib::linear_algebra::f32> test2 (temp * test);

  for (std::int32_t i {}; i < test2.get_rows(); ++i)
  {
    for (std::int32_t j {}; j < test2.get_columns(); ++j)
    {
      std::cout << test2.at(i, j) << ' ';
    }
    std::cout << std::endl;
  }
  
  return 0;
}
