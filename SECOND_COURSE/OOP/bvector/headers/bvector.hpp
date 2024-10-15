#ifndef __BITS_BVECTOR_H__
#define __BITS_BVECTOR_H__ 1

#include <cinttypes>
#include <string>
#include <cstring>
#include "xallocator.hpp"

#if __WORDSIZE == 64
#define __BITS_BVECTOR_MAX_SIZE__      0x2000000000
#define __BITS_BVECTOR_MAX_CAPACITY__  0x400000000
#define __BITS_BVECTOR_MID_CAPACITY__  0x100000000
#else
#define __BITS_BVECTOR_MAX_SIZE__     0x1fffffff
#define __BITS_BVECTOR_MAX_CAPACITY__ 0x4000000
#define __BITS_BVECTOR_MID_CAPACITY__ 0x1000000
#endif

#define __BMASK_BIT__ (0b10000000)
#define __BMASK_SET_BYTE__ (0b11111111)
#define __BMASK_RESET_BYTE__ (0b00000000)

#define calculate_capacity(size) (((size) >> 3) + ((size) & 0b00000111 ? 1 : 0)) 
#define byte_division(size) ((size) >> 3)
#define byte_module(size) ((size) & 0b00000111)
#define resize_factor(bits, bytes) ((bits) >> 3 == (bytes))

namespace bits
{
  template<class allocator_type = std::allocator<std::uint8_t>>
  class bvector
  {
  public:
    using pointer   = std::uint8_t*;
    using size_type = std::size_t;
    using byte_type = std::uint8_t;
  private:
    allocator_type xmalloc;
    pointer        storage;
    size_type      bits,
                   bytes;
  public:
    bvector() noexcept : storage (nullptr), bits (0), bytes (0) { }

    bvector(size_type bits_number, size_type value = 0) :
    bits (bits_number), bytes (calculate_capacity(bits_number))
    {
      if (bits > __BITS_BVECTOR_MAX_SIZE__)
        throw std::length_error ("bvector:bvector(size_type, size_type) -> invalid number of bits");

      try { storage = xmalloc.allocate(bytes); }
      catch (std::bad_alloc& error) { throw; }

      if (value)
        (void) std::memcpy(storage, &value, bytes >= 8 ? 8 : bytes);
    }

    bvector(const bvector& other) : bits (other.bits), bytes (other.bytes)
    {
      try
      {
        storage = xmalloc.allocate(bytes);
        (void) std::memcpy(storage, other.storage, bytes);
      }
      catch (std::bad_alloc& error) { throw; }
    }

    bvector(bvector&& rvalue) noexcept : storage (rvalue.storage),
    bits (rvalue.bits), bytes (rvalue.bytes)
    {
      rvalue.storage = nullptr;
      rvalue.bits = rvalue.bytes = 0;
    }

    ~bvector() { xmalloc.deallocate(storage, bytes); }
    
    // Capacity
    [[nodiscard]]
    size_type size() const noexcept { return bits; }
    
    [[nodiscard]]
    size_type capacity() const noexcept { return bytes; }

    [[nodiscard]]
    constexpr size_type max_size() const noexcept
    {
      return __BITS_BVECTOR_MAX_SIZE__;
    }
    
    [[nodiscard]]
    pointer data() const noexcept { return storage; }

    [[nodiscard]]
    constexpr allocator_type get_allocator() const noexcept
    {
      return xmalloc;
    }

    [[nodiscard]]
    size_type count() const noexcept
    {
      if (storage == nullptr) return 0;

      size_type tmp_bytes { byte_division(bits) };
      size_type bit_count {};
      size_type byte;

      for (size_type i {}; i != tmp_bytes; ++i)
      {
        byte = ((storage[i] >> 1) & 0b01010101) + (storage[i] & 0b01010101);
        byte = ((byte >> 2) & 0b00110011) + (byte & 0b00110011);
        byte = ((byte >> 4) & 0b00001111) + (byte & 0b00001111);
        bit_count += byte;
      }

      byte = byte_module(bits);

      for (size_type i {}; i != byte; ++i)
        if (storage[tmp_bytes] & __BMASK_BIT__ >> byte_module(i)) ++bit_count;

      return bit_count;
    }

    void shrink_to_fit()
    {
      if (storage == nullptr) return;
      else if (bits == 0) { clear(); return; }

      const size_type tmp_bytes { calculate_capacity(bits) };

      if (tmp_bytes < bytes)
      {
        try
        {
          pointer tmp_ptr { xmalloc.allocate(tmp_bytes) };
          (void) memcpy(tmp_ptr, storage, tmp_bytes);
          xmalloc.deallocate(storage, bytes);
          storage = tmp_ptr;
          bytes = tmp_bytes;
        }
        catch (std::bad_alloc& error) { throw; }
      }
    }
    
    [[nodiscard]]
    bool any() const noexcept
    {
      size_type tmp_bytes { byte_division(bits) };

      for (size_type i {}; i != tmp_bytes; ++i)
        if (storage[i]) return true;

      tmp_bytes = byte_module(bits);

      for (size_type i {}; i != tmp_bytes; ++i)
        if (storage[tmp_bytes] & __BMASK_BIT__ >> i) return true;

      return false;
    }

    [[nodiscard]]
    bool none() const noexcept
    {
      size_type tmp_bytes { byte_division(bits) };

      for (size_type i {}; i != bytes; ++i)
        if (storage[i]) return false;

      tmp_bytes = byte_module(bits);

      for (size_type i {}; i != tmp_bytes; ++i)
        if (storage[tmp_bytes] & __BMASK_BIT__ >> i) return false;

      return true;
    }

    [[nodiscard]]
    bool empty() const noexcept { return bits == 0; }

    // Modifiers
    void clear()
    {
      xmalloc.deallocate(storage, bytes);
      storage = nullptr;
      bits = bytes = 0;
    }

    // TODO:
    void resize(size_type bits_number, bool value = false)
    {
      if (bits_number == 0) { clear(); return; }
      else if (bits_number == bits) return;

      const size_type new_size { calculate_capacity(bits_number) };

      try
      {
        pointer tmp_ptr { xmalloc.allocate(new_size) };

        if (storage)
        {
          (void) std::memcpy(tmp_ptr,
                             storage,
                             bytes > new_size ? new_size : bytes);
          xmalloc.deallocate(storage, bytes);
        }

        storage = tmp_ptr;
      }
      catch (std::bad_alloc& error) { return; }
      
      if (bytes < new_size && value)
        std::memset(storage + bytes, __BMASK_SET_BYTE__, new_size - bytes);

      bits = bits_number;
      bytes = new_size;
    }

    void reserve(size_type bytes_number)
    {
      const size_type new_size { bytes + bytes_number };

      if (new_size > __BITS_BVECTOR_MAX_CAPACITY__)
        throw std::length_error ("bvector:reserve() -> invalid number of bytes");
      else if (bytes_number == 0 || bytes == __BITS_BVECTOR_MAX_CAPACITY__)
        return;

      try
      {
        pointer tmp_ptr { xmalloc.allocate(new_size) };

        if (storage)
        {
          (void) std::memcpy(tmp_ptr, storage, bytes);
          xmalloc.deallocate(storage, bytes);
        }

        storage = tmp_ptr;
      }
      catch (std::bad_alloc& error) { throw; }

      bytes = new_size;
    }

    void push_back(bool value)
    {
      if (bytes && bytes < __BITS_BVECTOR_MAX_CAPACITY__)
      {
        if (resize_factor(bits, bytes))
        {
          size_type addition_size;

          if (bytes < __BITS_BVECTOR_MID_CAPACITY__)
            addition_size = bytes;
          else if (!(bytes + (addition_size = bytes >> 1) < __BITS_BVECTOR_MAX_CAPACITY__))
            addition_size = __BITS_BVECTOR_MAX_CAPACITY__ - bytes;

          try { reserve(addition_size); }
          catch (std::bad_alloc& error) { throw; }
        }

        if (value)
          storage[byte_division(bits)] |= __BMASK_BIT__ >> byte_module(bits);
        else
          storage[byte_division(bits)] &= ~(__BMASK_BIT__ >> byte_module(bits));
        //storage[byte_division(bits)] |= (value & __BMASK_BIT__) >> byte_module(bits);

        ++bits;
      }
      else if (bytes)
      {
        if (bits != __BITS_BVECTOR_MAX_SIZE__)
        {
          if (value)
            storage[byte_division(bits)] |= __BMASK_BIT__ >> byte_module(bits);
          else
            storage[byte_division(bits)] &= ~(__BMASK_BIT__ >> byte_module(bits));

          ++bits;
        }
        else
        {
          if (value)
            storage[byte_division(bits - 1)] |= __BMASK_BIT__ >> byte_module(bits - 1);
          else
            storage[byte_division(bits - 1)] &= ~(__BMASK_BIT__ >> byte_module(bits - 1));
        }
      }
      else
      {
        try { storage = xmalloc.allocate(1); }
        catch (std::bad_alloc& error) { throw; }

        if (value) *storage = __BMASK_BIT__;
        
        ++bits;
        bytes = 1;
      }
    }

    [[nodiscard]]
    bool pop_back()
    {
      if (this->bits == 0)
        throw std::out_of_range ("bvector:pop_back() -> invalid storage size");

      --bits;
      return storage[byte_division(bits)] & __BMASK_BIT__ >> byte_module(bits);
    }

    bvector& set(size_type index, bool value = false)
    {
      if (index >= bits)
        throw std::out_of_range ("bvector:set(size_type, bool) -> index is out of range");
      else if (storage == nullptr)
        throw std::out_of_range ("bvector:set(size_type, bool) -> invalid storage pointer (nullptr)");

      if (value)
        storage[byte_division(index)] |= __BMASK_BIT__ >> byte_module(index);
      else
        storage[byte_division(index)] &= ~(__BMASK_BIT__ >> byte_module(index));

      return *this;
    }

    bvector& set()
    {
      if (storage == nullptr)
        throw std::out_of_range ("bvector:set() -> invalid storage pointer (nullptr)");

      std::memset(storage, __BMASK_SET_BYTE__, calculate_capacity(bits));
      return *this;
    }
    
    bvector& reset(size_type index)
    {
      if (index >= bits)
        throw std::out_of_range ("bvector:reset(i32) -> index is out of range");
      else if (storage == nullptr)
        throw std::out_of_range ("bvector:reset(i32) -> invalid storage pointer (nullptr)");

      storage[byte_division(index)] &= ~(__BMASK_BIT__ >> byte_module(index));
      return *this;
    }
    
    bvector& reset()
    {
      if (storage == nullptr)
        throw std::out_of_range ("bvector:reset() -> invalid storage pointer (nullptr)");

      std::memset(storage, __BMASK_RESET_BYTE__, calculate_capacity(bits));
      return *this;
    }
    
    bvector& flip(size_type index)
    {
      if (index >= bits)
        throw std::out_of_range ("bvector:flip(i32) -> index is out of range");
      else if (storage == nullptr)
        throw std::out_of_range ("bvector:flip(i32) -> invalid storage pointer (nullptr)");

      storage[byte_division(index)] ^= __BMASK_BIT__ >> byte_module(index);

      return *this;
    }

    bvector& flip()
    {
      if (storage == nullptr)
        throw std::out_of_range ("bvector:flip() -> invalid storage pointer (nullptr)");

      const size_type tmp_capacity { calculate_capacity(bits) };

      for (size_type i {}; i != tmp_capacity; ++i)
        storage[i] ^= __BMASK_SET_BYTE__;

      return *this;
    }

    void swap(bvector& other) noexcept
    {
      if (this == &other) return;

      std::swap(storage, other.storage);
      std::swap(bits, other.bits);
      std::swap(bytes, other.bytes);
    }

    // Element access
    [[nodiscard]]
    bool operator [] (size_type index) const noexcept
    {
      return storage[byte_division(index)] & __BMASK_BIT__ >> byte_module(index);
    }

    [[nodiscard]]
    bool front() const
    {
      if (storage == nullptr)
        throw std::out_of_range ("bvector:front() -> invalid storage pointer (nullptr)");

      return *storage & __BMASK_BIT__;
    }
    
    [[nodiscard]]
    bool back() const
    {
      if (storage == nullptr)
        throw std::out_of_range ("bvector:back() -> invalid storage pointer (nullptr)");

      return storage[byte_division(bits - 1)] & __BMASK_BIT__ >> byte_module(bits - 1);
    }
    
    [[nodiscard]]
    bool at(size_type index) const
    {
      if (index >= bits || storage == nullptr)
        throw std::out_of_range ("bvector:at(size_type) -> index is out of range");

      return storage[byte_division(index)] & __BMASK_BIT__ >> byte_module(index);
    }    

    // Operations
    bvector& operator = (const bvector& other)
    {
      if (this != &other)
      {
        if (bytes != other.bytes)
        {
          xmalloc.deallocate(storage, bytes);

          try { storage = xmalloc.allocate(other.bytes); }
          catch (std::bad_alloc& error) { throw; }
        }

        (void) std::memcpy(storage, other.storage, other.bytes);
        bits = other.bits;
        bytes = other.bytes;
      }

      return *this;
    }
    
    bvector& operator = (bvector&& rvalue)
    {
      xmalloc.deallocate(storage, bytes);
      std::swap(storage, rvalue.storage);

      bits = rvalue.bits;
      bytes = rvalue.bytes;
      rvalue.bits = rvalue.bytes = 0;

      return *this;
    }
    
    bvector& operator &= (const bvector& rhs)
    {
      if (bits != rhs.bits || !bits || !rhs.bits)
        throw std::invalid_argument ("bvector:operator(&=) -> invalid storage size");

      for (size_type i {}; i != bytes; ++i)
        storage[i] &= rhs.storage[i];

      return *this;
    }
    
    bvector& operator |= (const bvector& rhs)
    {
      if (bits != rhs.bits || !bits || !rhs.bits)
        throw std::invalid_argument ("bvector:operator(|=) -> invalid storage size");

      for (size_type i {}; i != bytes; ++i)
        storage[i] |= rhs.storage[i];

      return *this;
    }
    
    bvector& operator ^= (const bvector& rhs)
    {
      if (bits != rhs.bits || !bits || !rhs.bits)
        throw std::invalid_argument ("bvector:operator(^=) -> invalid storage size");

      for (size_type i {}; i != bytes; ++i)
        storage[i] ^= rhs.storage[i];

      return *this;
    }
    
    [[nodiscard]]
    bvector operator ~ () const
    {
      try
      {
        bvector<allocator_type> temp (*this);

        for (size_type i {}; i != temp.bytes; ++i)
          temp.storage[i] = ~temp.storage[i];

        return temp;
      }
      catch (std::bad_alloc& error) { throw; }
    }

    [[deprecated]]
    bvector& operator >>= (size_type shift);
    [[deprecated]]
    bvector& operator <<= (size_type shift);

    [[nodiscard]]
    std::string to_string() const
    {
      std::string storage_binary;

      try { storage_binary.reserve(bits); }
      catch (std::bad_alloc& error) { throw; }

      for (size_type i {}; i != bits; ++i)
        storage_binary.push_back(
        bool (storage[byte_division(i)] & __BMASK_BIT__ >> byte_module(i)) + '0');

      return storage_binary;
    }
  };

}

#undef __BMASK_BIT__
#undef __BMASK_SET_BYTE__
#undef __BMASK_RESET_BYTE__
#undef calculate_capacity
#undef byte_division
#undef byte_module
#undef resize_factor

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bool operator == (const bits::bvector<allocator_type>& lhs,
                  const bits::bvector<allocator_type>& rhs)
{
  if (lhs.size() != rhs.size()) return false;

  return std::memcmp(lhs.data(), rhs.data(), lhs.capacity()) == 0;
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bool operator != (const bits::bvector<allocator_type> &lhs,
                  const bits::bvector<allocator_type> &rhs)
{
  return !(lhs == rhs);
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bits::bvector<allocator_type> operator & (const bits::bvector<allocator_type> &lhs,
                                          const bits::bvector<allocator_type> &rhs )
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj &= rhs;
  }
  catch (std::invalid_argument& incorrect_size) { throw; }
  catch (std::bad_alloc& error) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bits::bvector<allocator_type> operator | (const bits::bvector<allocator_type>& lhs,
                                          const bits::bvector<allocator_type>& rhs)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj |= rhs;
  }
  catch (std::invalid_argument& incorrect_size) { throw; }
  catch (std::bad_alloc& error) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bits::bvector<allocator_type> operator ^ (const bits::bvector<allocator_type>& lhs,
                                          const bits::bvector<allocator_type>& rhs)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj ^= rhs;
  }
  catch (std::invalid_argument& incorrect_size) { throw; }
  catch (std::bad_alloc& error) { throw; }
}

#endif
