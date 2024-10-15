#ifndef __MEMORY_XALLOCATOR__
#define __MEMORY_XALLOCATOR__ 1

#include <cstdlib>
#include <cstring>
#include <memory>

namespace memory
{

  template<typename T>
  class xallocator
  {
  public:
    T* allocate(std::size_t blocks)
    {
      //if (blocks == 0) return nullptr;

      //T* pointer { static_cast<T*> (std::calloc(blocks, sizeof(T))) };
      T* pointer { static_cast<T*> (std::malloc(sizeof(T) * blocks)) };

      if (pointer == nullptr) throw std::bad_alloc();

      //std::memset(pointer, 0, blocks * sizeof(T));
      return pointer;
    }
    
    void deallocate(T* pointer, [[maybe_unused]] std::size_t blocks)
    {
      if (pointer) std::free(pointer);
    }
  };

}

#endif
