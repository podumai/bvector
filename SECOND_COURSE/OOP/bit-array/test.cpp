#include <vector>
#include <iostream>

int main()
{
  std::vector<int> test (10);

  std::cout << test.size() << ' ' << test.capacity() << std::endl;

  return 0;
}
