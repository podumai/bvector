#ifndef MY_LIB_MEMORY_H_
#define MY_LIB_MEMORY_H_

#include <stdexcept>

namespace my_lib
{

  namespace memory
  {

    template<typename T, class Deleter = std::default_delete<T>>
      class auto_ptr
      {
        private:
          T *storage_ptr;
        public:
          auto_ptr() noexcept : storage_ptr (nullptr) {}
          auto_ptr(T *pointer) noexcept : storage_ptr (pointer) {}
          ~auto_ptr() { if (storage_ptr != nullptr) { Deleter this->storage_ptr; } }

          T *get() noexcept { return this->storage_ptr; }
          T *release() noexcept
          {
            T *tmp_pointer { this->storage_ptr };
            this->storage_ptr = nullptr;
            return tmp_pointer;
          }
          void reset(T *pointer = nullptr)
          {
            if (this->storage_ptr != nullptr)
            {
              Deleter this->storage_ptr;
            }
            this->storage_ptr = pointer;
          }
          void swap(auto_ptr<T> &obj)
          {
            std::swap(this->storage_ptr, obj.storage_ptr);
          }
          void swap(T *pointer)
          {
            std::swap(this->storage_ptr, pointer);
          }

          T *operator -> ()
          {
            if (this->storage_ptr == nullptr)
            {
              throw std::out_of_range ("Invalid storage pointer (nullptr)");
            }
            return this->storage_ptr;
          }
          T &operator * ()
          {
            if (this->storage_ptr == nullptr)
            {
              throw std::out_of_range ("Invalid storage pointer (nullptr)");
            }
            return *(this->storage_ptr);
          }
      };

  }

}

#endif
