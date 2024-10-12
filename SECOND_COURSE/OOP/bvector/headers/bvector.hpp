#ifndef __BITS_BVECTOR_H__
#define __BITS_BVECTOR_H__ 1

#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <new>
#include <memory>
#include "bvector_imp.hpp"

#if __WORDSIZE == 64
#define __BITS_BVECTOR_MAX_SIZE__      0x2000000000
#define __BITS_BVECTOR_MAX_CAPACITY__  0x400000000
#define __BITS_BVECTOR_MID_CAPACITY__  0x100000000
#else
#define __BITS_BVECTOR_MAX_SIZE__     0x1fffffff
#define __BITS_BVECTOR_MAX_CAPACITY__ 0x4000000
#define __BITS_BVECTOR_MID_CAPACITY__ 0x1000000
#endif

namespace bits
{

  template<class allocator_type = std::allocator<std::uint8_t>>
  class bvector
  {
    using pointer   = std::uint8_t*;
    using i32       = std::int32_t;
    using i64       = std::int64_t;
    using size_type = std::size_t;
    using byte_type = std::uint8_t;
  private:
    allocator_type umalloc;
    pointer        storage;
    size_type      bits,
                   bytes;
  public:
    constexpr bvector() : storage (nullptr), bits (0), bytes (0) {}

    constexpr explicit bvector(size_type num_bits, i64 value = 0) : bits (num_bits),
    bytes (calculate_capacity(num_bits))
    {
      if (this->bits > __BITS_BVECTOR_MAX_SIZE__)
      {
        throw std::invalid_argument ("bvector:bvector(size_type, i64) -> invalid number of bits");
      }

      try { this->storage = this->umalloc.allocate(this->bytes); }
      catch (std::bad_alloc &error) { throw; }

      if (value)
      {
        (void) std::memcpy(this->storage,
                           &value,
                           this->bytes >= 8 ? 8 : this->bytes);
      }
    }

    bvector(const bvector &other) : bits (other.bits), bytes (other.bytes)
    {
      try
      {
        this->storage = this->umalloc.allocate(this->bytes);
        (void) std::memcpy(this->storage, other.storage, this->bytes);
      }
      catch (std::bad_alloc &error) { throw; }
    }

    bvector(bvector &&rvalue) noexcept : storage (rvalue.storage),
    bits (rvalue.bits), bytes (rvalue.bytes)
    {
      rvalue.storage = nullptr;
      rvalue.bits = rvalue.bytes = 0;
    }

    ~bvector()
    {
      if (this->storage != nullptr)
      {
        this->umalloc.deallocate(this->storage, this->bytes);
      }
    }
    
    // Capacity
    constexpr size_type size() const noexcept { return this->bits; }

    constexpr size_type capacity() const noexcept { return this->bytes; }

    constexpr size_type max_size() const noexcept { return __BITS_BVECTOR_MAX_SIZE__; }

    constexpr pointer data() const noexcept { return this->storage; }

    allocator_type get_allocator() const noexcept { return this->umalloc; }

    constexpr size_type count() const noexcept
    {
      if (this->storage == nullptr) { return 0; }

      size_type tmp_bytes { byte_division(this->bits) },
                bit_count {},
                byte;

      for (size_type i {}; i < tmp_bytes; ++i)
      {
        byte = ((this->storage[i] >> 1) & 0b01010101) +
                (this->storage[i] & 0b01010101);
        byte = ((byte >> 2) & 0b00110011) + (byte & 0b00110011);
        byte = ((byte >> 4) & 0b00001111) + (byte & 0b00001111);
        bit_count += byte;
      }

      byte = byte_module(this->bits);

      for (byte_type i {}; i < byte; ++i)
      {
        if (this->storage[tmp_bytes] & __BITS_BMASK_BIT__ >> byte_module(i))
        {
          ++bit_count;
        }
      }

      return bit_count;
    }

    void shrink_to_fit()
    {
      if (this->storage == nullptr) { return; }
      else if (this->bits == 0) { this->clear(); return; }

      const size_type tmp_bytes { calculate_capacity(this->bits) };

      if (tmp_bytes < this->bytes)
      {
        try
        {
          pointer tmp_storage { this->umalloc.allocate(tmp_bytes) };

          (void) std::memcpy(tmp_storage, this->storage, tmp_bytes);
          this->umalloc.deallocate(this->storage, this->bytes);

          this->storage = tmp_storage;
          this->bytes = tmp_bytes;
        }
        catch (std::bad_alloc &error) { throw; }
      }
    }

    constexpr bool any() const noexcept
    {
      const size_type tmp_bytes { byte_division(this->bits) };

      for (size_type i {}; i < tmp_bytes; ++i)
      {
        if (this->storage[i]) { return true; }
      }

      const size_type byte { byte_module(this->bits) };

      for (size_type i {}; i < byte; ++i)
      {
        if (this->storage[tmp_bytes] & __BITS_BMASK_BIT__ >> i) { return true; }
      }

      return false;
    }

    constexpr bool none() const noexcept
    {
      const size_type tmp_bytes { byte_division(this->bits) };

      for (size_type i {}; i < this->bytes; ++i)
      {
        if (this->storage[i]) { return false; }
      }

      const size_type byte { byte_module(this->bits) };

      for (size_type i {}; i < byte; ++i)
      {
        if (this->storage[tmp_bytes] & __BITS_BMASK_BIT__ >> i)
        {
          return false;
        }
      }

      return true;
    }

    constexpr bool empty() const noexcept { return this->bits == 0; }

    // Modifiers
    constexpr void clear()
    {
      if (this->storage == nullptr) { return; }

      this->umalloc.deallocate(this->storage, this->bytes);
      this->storage = nullptr;
      this->bits = this->bytes = 0;
    }

    void resize(size_type num_bits, bool value = false)
    {
      if (num_bits == 0) { this->clear(); return; }
      else if (num_bits == this->bits) { return; }

      const size_type tmp_bytes { calculate_capacity(num_bits) };

      try
      {
        pointer tmp_storage { this->umalloc.allocate(tmp_bytes) };

        if (this->storage)
        {
          (void) std::memcpy(tmp_storage,
                             this->storage,
                             min(this->bytes, tmp_bytes));
          this->umalloc.deallocate(this->storage, this->bytes);
        }

        this->storage = tmp_storage;
      }
      catch (std::bad_alloc &error) { throw; }

      if (tmp_bytes > this->bytes && value)
      {
        std::memset(this->storage + this->bytes,
                    __BITS_BMASK_TRUE_BYTE__,
                    tmp_bytes - this->bytes);
      }

      this->bits = num_bits;
      this->bytes = tmp_bytes;
    }

    void reserve(size_type num_bytes)
    {
      if (num_bytes > __BITS_BVECTOR_MAX_CAPACITY__ - this->bytes)
      {
        throw std::length_error ("bvector:reserve() -> invalid number of bytes");
      }
      else if (num_bytes == 0 || this->bytes == __BITS_BVECTOR_MAX_CAPACITY__)
      {
        return;
      }

      size_type tmp_bytes { this->bytes + num_bytes };
      
      try
      {
        pointer tmp_storage { this->umalloc.allocate(tmp_bytes) };
        if (this->storage)
        {
          (void) std::memcpy(tmp_storage,
                             this->storage,
                             this->bytes);
          this->umalloc.deallocate(this->storage, this->bytes);
        }
        this->storage = tmp_storage;
        this->bytes = tmp_bytes;
      }
      catch (std::bad_alloc &error) { throw; }
    }

    void push_back(bool value)
    {
      if (this->bytes && this->bytes < __BITS_BVECTOR_MAX_CAPACITY__)
      {
        size_type tmp_bytes { calculate_capacity(this->bits + 1) };

        if (tmp_bytes > this->bytes)
        {
          if (this->bytes < __BITS_BVECTOR_MID_CAPACITY__)
          {
            tmp_bytes = this->bytes;
          }
          else if (!(this->bytes + (tmp_bytes = this->bytes >> 1) <
                __BITS_BVECTOR_MAX_CAPACITY__))
          {
            tmp_bytes = __BITS_BVECTOR_MAX_CAPACITY__ - this->bytes;
          }

          try { this->reserve(tmp_bytes); }
          catch (std::bad_alloc &error) { throw; }
        }

        if (value)
        {
          this->storage[byte_division(this->bits)] |=
          __BITS_BMASK_BIT__ >> byte_module(this->bits);
        }
        else
        {
          this->storage[byte_division(this->bits)] &=
          ~(__BITS_BMASK_BIT__ >> byte_module(this->bits));
        }
        ++this->bits;
      }
      else if (this->bytes)
      {
        if (this->bits < __BITS_BVECTOR_MAX_SIZE__)
        {
          if (value)
          {
            this->storage[byte_division(this->bits)] |=
            __BITS_BMASK_BIT__ >> byte_module(this->bits);
          }
          else
          {
            this->storage[byte_division(this->bits)] &=
            ~(__BITS_BMASK_BIT__ >> byte_module(this->bits));
          }
          ++this->bits;
        }
        else
        {
          if (value)
          {
            this->storage[byte_division(this->bits - 1)] |=
            __BITS_BMASK_BIT__ >> byte_module(this->bits - 1);
          }
          else
          {
            this->storage[byte_division(this->bits - 1)] &=
            ~(__BITS_BMASK_BIT__ >> byte_module(this->bits - 1));
          }
        }
      }
      else
      {
        try { this->reserve(1); }
        catch (std::bad_alloc &error) { throw; }
        
        if (value) { this->storage[0] |= __BITS_BMASK_BIT__; }
        ++this->bits;
      }
    }

    bool pop_back()
    {
      if (this->bits == 0)
      {
        throw std::out_of_range ("bvector:pop_back() -> invalid storage size");
      }
      
      const bool value
      { bool (this->storage[byte_division(this->bits)] & __BITS_BMASK_BIT__ >> byte_module(this->bits)) };
      --this->bits;
      return value;
    }

    bvector &set(size_type index, bool value = true)
    {
      if (index >= this->bits)
      {
        throw std::out_of_range ("bvector:set(i32, bool) -> index is out of range");
      }
      else if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:set(i32, bool) -> invalid storage pointer (nullptr)");
      }
      
      if (value)
      {
        this->storage[byte_division(index)] |= __BITS_BMASK_BIT__ >> byte_module(index);
      }
      else
      {
        this->storage[byte_division(index)] &= ~(__BITS_BMASK_BIT__ >> byte_module(index));
      }
      return *this;
    }

    bvector &set()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:set() -> invalid storage pointer (nullptr)");
      }

      std::memset(this->storage,
                  __BITS_BMASK_TRUE_BYTE__,
                  calculate_capacity(this->bits));
      return *this;
    }

    bvector &reset(size_type index)
    {
      if (index >= this->bits)
      {
        throw std::out_of_range ("bvector:reset(i32) -> index is out of range");
      }
      else if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:reset(i32) -> invalid storage pointer (nullptr)");
      }

      this->storage[byte_division(index)] &= ~(__BITS_BMASK_BIT__ >> byte_module(index));
      return *this;
    }

    bvector &reset()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:reset() -> invalid storage pointer (nullptr)");
      }

      std::memset(this->storage,
                  __BITS_BMASK_FALSE_BYTE__,
                  calculate_capacity(this->bits));
      return *this;
    }

    bvector &flip(size_type index)
    {
      if (index >= this->bits)
      {
        throw std::out_of_range ("bvector:flip(i32) -> index is out of range");
      }
      else if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:flip(i32) -> invalid storage pointer (nullptr)");
      }

      this->storage[byte_division(index)] ^= __BITS_BMASK_BIT__ >> byte_module(index);

      return *this;
    }

    bvector &flip()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:flip() -> invalid storage pointer (nullptr)");
      }

      const size_type tmp_capacity { calculate_capacity(this->bits) };

      for (size_type i {}; i < tmp_capacity; ++i)
      {
        this->storage[i] ^= __BITS_BMASK_TRUE_BYTE__;
      }

      return *this;
    }

    void swap(bvector &other) noexcept
    {
      if (this == &other) { return; }
      
      std::swap(this->storage, other.storage);
      std::swap(this->bits, other.bits);
      std::swap(this->bytes, other.bytes);
    }
    
    // Element access
    bool operator [] (size_type index) const noexcept
    {
      return this->storage[byte_division(index)] & __BITS_BMASK_BIT__ >> byte_module(index);
    }

    bool front() const
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:front() -> invalid storage pointer (nullptr)");
      }

      return this->storage[0] & __BITS_BMASK_BIT__ ;
    }

    bool back() const
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:back() -> invalid storage pointer (nullptr)");
      }

      return this->storage[byte_division(this->bits - 1)] &
             __BITS_BMASK_BIT__ >> byte_module(this->bits - 1);
    }

    bool at(size_type index) const
    {
      if (index >= this->bits || this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:at(size_type) -> index is out of range");
      }

      return this->storage[byte_division(index)] & __BITS_BMASK_BIT__ >> byte_module(index);
    }
    
    // Operations
    bvector &operator = (const bvector &other)
    {
      if (this != &other)
      {
        if (this->bytes != other.bytes)
        {
          this->umalloc.deallocate(this->storage, this->bytes);

          try { this->storage = umalloc.allocate(other.bytes); }
          catch (std::bad_alloc &error) { throw; }
        }

        (void) std::memcpy(this->storage, other.storage, other.bytes);
        this->bits = other.bits;
        this->bytes = other.bytes;
      }

      return *this;
    }

    bvector &operator = (bvector &&rvalue)
    {
      if (this->storage)
      {
        this->umalloc.deallocate(this->storage, this->bytes);
        this->storage = nullptr;
      }

      std::swap(this->storage, rvalue.storage);
      this->bits = rvalue.bits;
      this->bytes = rvalue.bytes;
      rvalue.bits = rvalue.bytes = 0;
      return *this;
    }

    bvector &operator &= (const bvector &rhs)
    {
      if (this->bits != rhs.bits || !this->bits || !rhs.bits)
      {
        throw std::invalid_argument ("bvector:operator(&=) -> invalid storage size");
      }

      for (size_type i {}; i < this->bytes; ++i)
      {
        this->storage[i] &= rhs.storage[i];
      }

      return *this;
    }
    
    bvector &operator |= (const bvector &rhs)
    {
      if (this->bits != rhs.bits || !this->bits || !rhs.bits)
      {
        throw std::invalid_argument ("bvector:operator(|=) -> invalid storage size");
      }

      for (size_type i {}; i < this->bytes; ++i)
      {
        this->storage[i] |= rhs.storage[i];
      }

      return *this;
    }

    bvector &operator ^= (const bvector &rhs)
    {
      if (this->bits != rhs.bits || !this->bits || !rhs.bits)
      {
        throw std::invalid_argument ("bvector:operator(^=) -> invalid storage size");
      }

      for (size_type i {}; i < this->bytes; ++i)
      {
        this->storage[i] ^= rhs.storage[i];
      }

      return *this;
    }
    
    bvector operator ~ () const
    {
      try
      {
        bvector<allocator_type> temp (*this);
        
        for (size_type i {}; i < temp.bytes; ++i)
        {
          temp.storage[i] = ~temp.storage[i];
        }

        return temp;
      }
      catch (std::bad_alloc &error) { throw; }
    }

    /*bvector &operator <<= (size_type shift)
    {
      if (shift > _BITS_BVECTOR_MAX_SIZE_)
      {
        throw std::invalid_argument ("bvector:operator(<<=) -> invalid shift");
      }
      else if (shift)
      {
      }

      return *this;
    }*/

    // TODO: bvector &operator >>= (i32);
    
    std::string to_string() const
    {
      std::string storage_binary;
      
      try { storage_binary.reserve(this->bits); }
      catch (std::bad_alloc &error) { throw; }

      for (size_type i {}; i < this->bits; ++i)
      {
        if (this->storage[byte_division(i)] & __BITS_BMASK_BIT__ >> byte_module(i))
        {
          storage_binary.push_back('1');
        }
        else
        {
          storage_binary.push_back('0');
        }
      }

      return storage_binary;
    }
    
    // TODO: friend bvector operator << (i32) const;
    // TODO: friend bvector operator >> (i32) const;
  };

}

template<class allocator_type = std::allocator<std::uint8_t>>
bool operator == (const bits::bvector<allocator_type> &lhs,
                  const bits::bvector<allocator_type> &rhs)
{
  if (lhs.size() != rhs.size()) { return false; }

  return std::memcmp(lhs.data(),
                     rhs.data(),
                     lhs.capacity()) == 0;
}

template<class allocator_type = std::allocator<std::uint8_t>>
bool operator !=
(
  const bits::bvector<allocator_type> &lhs,
  const bits::bvector<allocator_type> &rhs
)
{
  return !(lhs == rhs);
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bvector<allocator_type> operator &
(
  const bits::bvector<allocator_type> &lhs,
  const bits::bvector<allocator_type> &rhs
)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj &= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &error) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bvector<allocator_type> operator |
(
  const bits::bvector<allocator_type> &lhs,
  const bits::bvector<allocator_type> &rhs
)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj |= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &error) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bvector<allocator_type> operator ^
(
  const bits::bvector<allocator_type> &lhs,
  const bits::bvector<allocator_type> &rhs
)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj ^= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &error) { throw; }
}

#endif
