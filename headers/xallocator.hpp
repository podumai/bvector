#ifndef __MEMORY_XALLOCATOR__
#define __MEMORY_XALLOCATOR__ 1

#include <cstdlib>
#include <cstring>
#include <memory>

/*
 * Test user allocator for template parameter
*/

namespace memory
{

  template<typename T>
  class xallocator
  {
  public:
    T* allocate(std::size_t blocks)
    {
      T* pointer { static_cast<T*> (std::malloc(sizeof(T) * blocks)) };

      if (pointer == nullptr) throw std::bad_alloc();
      
      return pointer;
    }
    
    void deallocate(T* pointer, [[maybe_unused]] std::size_t blocks)
    {
      if (pointer) std::free(pointer);
    }
  };

}

#endif
