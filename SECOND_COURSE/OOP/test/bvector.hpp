#ifndef __BITS_BVECTOR_H__
#define __BITS_BVECTOR_H__

#define _BITS_BVECTOR_MAX_SIZE_     0x2000000000
#define _BITS_BVECTOR_MAX_CAPACITY_ 0x400000000

#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <new>
#include <memory>

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
    bytes ((num_bits >> 3) + (num_bits & 0b00000111 ? 1 : 0))
    {
      if (this->bits > _BITS_BVECTOR_MAX_SIZE_)
      {
        throw std::invalid_argument ("bvector:bvector(size_type, i64) -> invalid number of bits");
      }

      try { this->storage = this->umalloc.allocate(this->bytes); }
      catch (std::bad_alloc) { throw; }

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
      catch (std::bad_alloc) { throw; }
    }

    bvector(bvector &&rvalue) noexcept : storage (rvalue.storage),
    bits (rvalue.bits), bytes (rvalue.bytes)
    {
      rvalue.storage = nullptr;
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

    constexpr size_type max_size() const noexcept { return _BITS_BVECTOR_MAX_SIZE_; }

    constexpr pointer data() const noexcept { return this->storage; }

    allocator_type get_allocator() const noexcept { return this->umalloc; }

    constexpr size_type count() const noexcept
    {
      if (this->storage == nullptr) { return 0; }

      size_type tmp_bytes { this->bits >> 3 };
      size_type bit_count {};
      byte_type byte;

      for (size_type i {}; i < tmp_bytes; ++i)
      {
        byte = ((this->storage[i] >> 1) & 0b01010101) +
                (this->storage[i] & 0b01010101);
        byte = ((byte >> 2) & 0b00110011) + (byte & 0b00110011);
        byte = ((byte >> 4) & 0b00001111) + (byte & 0b00001111);
        bit_count += byte;
      }

      byte = this->bits & 0b00000111;

      for (byte_type i {}; i < byte; ++i)
      {
        if (this->storage[tmp_bytes] & 0b10000000 >> (i & 0b00000111)) { ++bit_count; }
      }

      return bit_count;
    }

    void shrink_to_fit()
    {
      if (this->storage == nullptr) { return; }
      else if (this->bits == 0) { this->clear(); return; }

      const size_type tmp_capacity { (this->bits >> 3) +
                                     (this->bits & 0b00000111 ? 1 : 0) };

      if (tmp_capacity < this->bytes)
      {
        try
        {
          pointer tmp_storage { this->umalloc.allocate(tmp_capacity) };

          (void) std::memcpy(tmp_storage, this->storage, tmp_capacity);
          this->umalloc.deallocate(this->storage, this->bytes);

          this->storage = tmp_storage;
          this->bytes = tmp_capacity;
        }
        catch (std::bad_alloc &no_memory) { throw; }
      }
    }

    constexpr bool any() const noexcept
    {
      const size_type tmp_capacity { this->bits >> 3 };

      for (size_type i {}; i < tmp_capacity; ++i)
      {
        if (this->storage[i]) { return true; }
      }

      const byte_type byte { byte_type (this->bits & 0b00000111) };

      for (byte_type i {}; i < byte; ++i)
      {
        if (this->storage[tmp_capacity] & 0b10000000 >> i) { return true; }
      }

      return false;
    }

    constexpr bool none() const noexcept
    {
      const size_type tmp_capacity { this->bits >> 3 };

      for (size_type i {}; i < this->bytes; ++i)
      {
        if (this->storage[i]) { return false; }
      }

      const byte_type byte { byte_type (this->bits & 0b00000111) };

      for (byte_type i {}; i < byte; ++i)
      {
        if (this->storage[tmp_capacity] & 0b10000000 >> i) { return false; }
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

      const size_type tmp_capacity { (num_bits >> 3) +
                                     (num_bits & 0b00000111 ? 1 : 0)};

      try
      {
        pointer tmp_storage { this->umalloc.allocate(tmp_capacity) };

        if (this->storage)
        {
          (void) std::memcpy(tmp_storage,
                             this->storage,
                             std::min(this->bytes, tmp_capacity));
          this->umalloc.deallocate(this->storage, this->bytes);
        }

        this->storage = tmp_storage;
      }
      catch (std::bad_alloc) { throw; }

      if (tmp_capacity > this->bytes && value)
      {
        std::memset(this->storage + this->bytes,
                    0b11111111,
                    tmp_capacity - this->bytes);
      }

      this->bits = num_bits;
      this->bytes = tmp_capacity;
    }

    void reserve(size_type num_bytes)
    {
      if (num_bytes > _BITS_BVECTOR_MAX_CAPACITY_)
      {
        throw std::length_error ("bvector:reserve() -> invalid number of bytes");
      }
      else if (num_bytes == 0 || this->bytes == _BITS_BVECTOR_MAX_CAPACITY_)
      {
        return;
      }

      size_type tmp_capacity { (this->bytes + num_bytes)/* & 0x7ffffff*/ };

      if (tmp_capacity > this->bytes)
      {
        try
        {
          pointer tmp_storage { this->umalloc.allocate(tmp_capacity) };

          if (this->storage)
          {
            (void) std::memcpy(tmp_storage, this->storage, this->bytes);
            this->umalloc.deallocate(this->storage, this->bytes);
          }

          this->storage = tmp_storage;
          this->bytes = tmp_capacity;
        }
        catch (std::bad_alloc) { throw; }
      }
    }

    void push_back(bool value)
    {
      if (this->bytes && this->bytes < _BITS_BVECTOR_MAX_CAPACITY_)
      {
        size_type tmp_capacity
        { ((this->bits + 1) >> 3) + ((this->bits + 1) & 0b00000111 ? 1 : 0) };

        if (tmp_capacity > this->bytes)
        {
          if (this->bytes < 0x100000000)
          {
            tmp_capacity = this->bytes;
          }
          else if (!(this->bytes + (tmp_capacity = this->bytes >> 1) <
                _BITS_BVECTOR_MAX_CAPACITY_))
          {
            tmp_capacity = _BITS_BVECTOR_MAX_CAPACITY_ - this->bytes;
          }

          try { this->reserve(tmp_capacity); }
          catch (std::bad_alloc) { throw; }
        }

        if (value)
        {
          this->storage[this->bits >> 3] |=
          (0b10000000 >> (this->bits & 0b00000111));
        }
        else
        {
          this->storage[this->bits >> 3] &=
          ~(0b10000000 >> (this->bits & 0b00000111));
        }
        ++this->bits;
      }
      else if (this->bytes)
      {
        if (this->bits < _BITS_BVECTOR_MAX_SIZE_)
        {
          if (value)
          {
            this->storage[this->bits >> 3] |=
            (0b10000000 >> (this->bits & 0b00000111));
          }
          else
          {
            this->storage[this->bits >> 3] &=
            ~(0b10000000 >> (this->bits & 0b00000111));
          }
          ++this->bits;
        }
        else
        {
          if (value)
          {
            this->storage[(this->bits - 1) >> 3] |=
            (0b10000000 >> (this->bits - 1 & 0b00000111));
          }
          else
          {
            this->storage[(this->bits - 1) >> 3] &=
            ~(0b10000000 >> (this->bits - 1 & 0b00000111));
          }
        }
      }
      else
      {
        try { this->reserve(1); }
        catch (std::bad_alloc) { throw; }
        
        if (value) { this->storage[0] |= 0b10000000; }
        ++this->bits;
      }
    }

    bool pop_back()
    {
      if (this->bits == 0)
      {
        throw std::out_of_range ("bvector:pop_back() -> invalid storage size");
      }
      
      const bool value { bool (this->storage[this->bits >> 3] & 0b10000000 >> (this->bits & 0b00000111)) };
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
        this->storage[index >> 3] |= (0b10000000 >> (index & 0b00000111));
      }
      else
      {
        this->storage[index >> 3] &= ~(0b10000000 >> (index & 0b00000111));
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
                  0b11111111,
                  (this->bits >> 3) + (this->bits & 0b00000111 ? 1 : 0));
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

      this->storage[index >> 3] &= ~(0b10000000 >> (index & 0b00000111));
      return *this;
    }

    bvector &reset()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:reset() -> invalid storage pointer (nullptr)");
      }

      std::memset(this->storage,
                  0b00000000,
                  (this->bits >> 3) + (this->bits & 0b00000111 ? 1 : 0));
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

      this->storage[index >> 3] ^= (0b10000000 >> (index & 0b00000111));

      return *this;
    }

    bvector &flip()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:flip() -> invalid storage pointer (nullptr)");
      }

      const size_type tmp_capacity
      { (this->bits >> 3) + (this->bits & 0b00000111 ? 1 : 0) };

      for (size_type i {}; i < tmp_capacity; ++i)
      {
        this->storage[i] ^= 0b11111111;
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
    bool operator [] (i64 index) const noexcept
    {
      return this->storage[index >> 3] & 0b10000000 >> (index & 0b00000111);
    }

    bool front() const
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:front() -> invalid storage pointer (nullptr)");
      }

      return this->storage[0] & 0b10000000 ;
    }

    bool back() const
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:back() -> invalid storage pointer (nullptr)");
      }

      return this->storage[(this->bits - 1) >> 3] &
             (0b10000000 >> ((this->bits - 1) & 0b00000111));
    }

    bool at(i64 index) const
    {
      if (index < 0 || index >= this->bits || this->storage == nullptr)
      {
        throw std::out_of_range ("bvector:at(i32) -> index is out of range");
      }

      return this->storage[index >> 3] & (0b10000000 >> (index & 0b00000111));
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
          catch (std::bad_alloc &err) { throw; }
        }

        (void) std::memcpy(this->storage, other.storage, other.bytes);
        this->bits = other.bits;
        this->bytes = other.bytes;
      }

      return *this;
    }

    bvector &operator = (bvector &&rvalue)
    {
      std::swap(this->storage, rvalue.storage);
      this->bits = rvalue.bits;
      this->bytes = rvalue.bytes;
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
      catch (std::bad_alloc) { throw; }
    }

    /*bvector &operator <<= (i32 shift)
    {
      if (shift < 0)
      {
        throw std::invalid_argument ("bvector:bitwise and assignment operator(<<=) -> invalid shift");
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
      catch (std::bad_alloc) { throw; }

      for (size_type i {}; i < this->bits; ++i)
      {
        if (this->storage[i >> 3] & 0b10000000 >> (i & 0b00000111))
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
bool operator != (const bits::bvector<allocator_type> &lhs,
                  const bits::bvector<allocator_type> &rhs)
{
  return !(lhs == rhs);
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bvector<allocator_type> operator & (const bits::bvector<allocator_type> &lhs,
                                          const bits::bvector<allocator_type> &rhs)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj &= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &no_memory) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bvector<allocator_type> operator | (const bits::bvector<allocator_type> &lhs,
                                          const bits::bvector<allocator_type> &rhs)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj |= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &no_memory) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bvector<allocator_type> operator ^ (const bits::bvector<allocator_type> &lhs,
                                          const bits::bvector<allocator_type> &rhs)
{
  try
  {
    bits::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj ^= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &no_memory) { throw; }
}

#endif
