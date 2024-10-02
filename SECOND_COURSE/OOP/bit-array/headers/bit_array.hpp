#ifndef __BITS_BIT_ARRAY_H__
#define __BITS_BIT_ARRAY_H__

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
  class bit_array
  {
  public:
    using pointer   = std::uint8_t*;
    using byte      = std::uint8_t;
    using data_type = std::int32_t;
    using i32       = std::int32_t;
    using u32       = std::uint32_t;
    using i64       = std::int64_t;
  private:
    allocator_type umalloc;
    pointer        storage;
    data_type      bits,
                   blocks;

    // Private functions
    i32 calc_capacity(i32 num_bits) const noexcept
    {
      return num_bits ? num_bits / 8 + (num_bits % 8 ? 1 : 0) : 1;
    }

    i32 div_byte(i32 index) const noexcept { return index / 8; }

    i32 mod_byte(i32 index) const noexcept { return index % 8; }

    void set_bit(i32 index, bool value) noexcept
    {
      if (value)
      {
        this->storage[div_byte(index)] |= (0b10000000 >> mod_byte(index));
      }
      else
      {
        const i32 index_position { div_byte(index) };
        this->storage[index_position] &= (~(0b10000000 >> mod_byte(index))
                                      & this->storage[index_position]);
      }
    }
  public:
    /*
     * TODO: Write description.
    */
    bit_array() : storage (nullptr), bits (0), blocks (0) {}

    /*
     * TODO: Write description.
    */
    explicit bit_array(i32 num_bits, i64 value = false) : bits (num_bits),
    blocks (calc_capacity(num_bits))
    {
      if (num_bits < 0)
      {
        throw std::invalid_argument ("bit_array:bit_array(i64) -> invalid number of bits");
      }

      try
      {
        this->storage = this->umalloc.allocate(this->blocks);
      }
      catch (std::bad_alloc &no_memory)
      {
        throw no_memory;
      }

      if (value)
      {
        std::memcpy(this->storage,
                    &value,
                    this->blocks >= 8 ? 8 : this->blocks);
      }
    }

    bit_array(const bit_array &other) : bits (other.bits), blocks (other.blocks)
    {
      try
      {
        this->storage = this->umalloc.allocate(this->blocks);
        std::memcpy(this->storage, other.storage, this->blocks);
      }
      catch (std::bad_alloc &no_memory)
      {
        throw no_memory;
      }
    }

    bit_array(bit_array &&other) noexcept : storage (other.storage),
    bits (other.bits), blocks (other.blocks)
    {
      other.storage = nullptr;
    }

    ~bit_array()
    {
      if (this->storage != nullptr)
      {
        this->umalloc.deallocate(this->storage, this->blocks);
      }
    }
    
    // Capacity
    i32 size() const noexcept { return this->bits; }

    i32 capacity() const noexcept { return this->blocks; }

    constexpr i32 max_size() const noexcept { return INT32_MAX; }

    pointer data() const noexcept { return this->storage; }

    u32 count() const noexcept
    {
      if (this->storage == nullptr) { return 0; }

      i32 bit_count {};

      for (i32 i {}; i < this->bits; ++i)
      {
        if ((*this)[i]) { ++bit_count; }
      }

      return bit_count;
    }

    void shrink_to_fit()
    {
      if (this->storage == nullptr) { return; }
      else if (this->bits == 0) { this->clear(); return; }

      const i32 tmp_capacity { calc_capacity(this->bits) };

      if (tmp_capacity < this->blocks)
      {
        try
        {
          pointer tmp_storage { this->umalloc.allocate(tmp_capacity) };

          std::memcpy(tmp_storage, this->storage, tmp_capacity);
          this->umalloc.deallocate(this->storage, this->blocks);

          this->storage = tmp_storage;
          this->blocks = tmp_capacity;
        }
        catch (std::bad_alloc &no_memory)
        {
          throw no_memory;
        }
      }
    }

    bool any() const noexcept
    {
      for (i32 i {}; i < this->blocks; ++i)
      {
        if (this->storage[div_byte(i)] & 0b11111111) { return true; }
      }

      return false;
    }

    bool none() const noexcept
    {
      for (i32 i {}; i < this->blocks; ++i)
      {
        if (this->storage[div_byte(i)] & 0b11111111) { return false; }
      }

      return true;
    }

    bool empty() const noexcept
    {
      return this->bits == 0;
    }

    // Modifiers
    void clear()
    {
      if (this->storage == nullptr) { return; }

      this->umalloc.deallocate(this->storage, this->blocks);
      this->storage = nullptr;
      this->bits = this->blocks = 0;
    }

    void resize(i32 num_bits, bool value = false)
    {
      if (num_bits < 0)
      {
        throw std::invalid_argument ("bit_array:resize() -> invalid number of bits");
      }
      else if (num_bits == 0)
      {
        this->clear();
        return;
      }
      else if (num_bits == this->bits)
      {
        return;
      }

      const i32 tmp_capacity { calc_capacity(num_bits) };

      try
      {
        pointer tmp_storage { this->umalloc.allocate(tmp_capacity) };

        std::memcpy(tmp_storage,
                    this->storage,
                    std::min(this->blocks, tmp_capacity));
        this->umalloc.deallocate(this->storage, this->blocks);
        this->storage = tmp_storage;
      }
      catch (std::bad_alloc &no_memory)
      {
        throw no_memory;
      }

      if (tmp_capacity > this->blocks)
      {
        std::memset(this->storage + this->blocks,
                    value ? 0b11111111 : 0b00000000,
                    tmp_capacity - this->blocks);
      }

      this->bits = num_bits;
      this->blocks = tmp_capacity;
    }

    void reserve(i32 num_blocks)
    {
      if (num_blocks < 0 || num_blocks > 268435456)
      {
        throw std::length_error ("bit_array:reserve() -> invalid number of blocks");
      }
      else if (num_blocks == 0 || this->blocks == 268435456)
      {
        return;
      }

      const i32 tmp_capacity { this->blocks + num_blocks };

      if (tmp_capacity > this->blocks)
      {
        try
        {
          pointer tmp_storage { this->umalloc.allocate(tmp_capacity) };

          std::memcpy(tmp_storage, this->storage, this->blocks);
          this->umalloc.deallocate(this->storage, this->blocks);

          this->storage = tmp_storage;
          this->blocks = tmp_capacity;
        }
        catch (std::bad_alloc &no_memory) { throw no_memory; }
      }
    }

    void push_back(bool value)
    {
      if (this->blocks && this->blocks < 268'435'456)
      {
        i32 tmp_capacity { calc_capacity(this->bits + 1) };

        if (tmp_capacity > this->blocks)
        {
          if (this->blocks < 134'217'728)
          {
            tmp_capacity = this->blocks;
          }
          else if (!(this->blocks + (tmp_capacity = this->blocks >> 1) < 268'435'456))
          {
            tmp_capacity = 268'435'456 - this->blocks;
          }

          try { this->reserve(tmp_capacity); }
          catch (std::bad_alloc &no_memory) { throw no_memory; }
        }

        this->set_bit(this->bits, value);
        ++this->bits;
      }
      else if (this->blocks)
      {
        if (this->bits < INT32_MAX) { this->set_bit(this->bits++, value); }
        else { this->set_bit(this->bits - 1, value); }
      }
      else
      {
        try { this->reserve(1); }
        catch (std::bad_alloc &no_memory) { throw no_memory; }
        
        if (value) { this->storage[0] |= 0b10000000; }
        ++this->bits;
      }
    }

    bool pop_back()
    {
      if (this->bits == 0)
      {
        throw std::out_of_range ("bit_array:pop_back() -> invalid storage size");
      }
      return this->storage[div_byte(--this->bits)] &
        (0b10000000 >> mod_byte(this->bits));
    }

    bit_array &set(i32 index, bool value = true)
    {
      if (index < 0 || index >= this->bits)
      {
        throw std::out_of_range ("bit_array:set(i32, bool) -> index is out of range");
      }
      else if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:set(i32, bool) -> invalid storage pointer (nullptr)");
      }
      
      this->set_bit(index, value);
      return *this;
    }

    bit_array &set()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:set() -> invalid storage pointer (nullptr)");
      }

      std::memset(this->storage, 0b11111111, calc_capacity(this->bits));
      return *this;
    }

    bit_array &reset(i32 index)
    {
      if (index < 0 || index >= this->bits)
      {
        throw std::out_of_range ("bit_array:reset(i32) -> index is out of range");
      }
      else if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:reset(i32) -> invalid storage pointer (nullptr)");
      }

      this->set_bit(index, false);
      return *this;
    }

    bit_array &reset()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:reset() -> invalid storage pointer (nullptr)");
      }

      std::memset(this->storage, 0b00000000, calc_capacity(this->bits));
      return *this;
    }

    bit_array &flip(i32 index)
    {
      if (index < 0 || index >= this->bits)
      {
        throw std::out_of_range ("bit_array:flip(i32) -> index is out of range");
      }
      else if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:flip(i32) -> invalid storage pointer (nullptr)");
      }

      const i32 index_position { div_byte(index) };
      this->storage[index_position] ^= (0b10000000 >> mod_byte(index));

      return *this;
    }

    bit_array &flip()
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:flip() -> invalid storage pointer (nullptr)");
      }

      const i32 tmp_capacity { calc_capacity(this->bits) };

      for (i32 i {}; i < tmp_capacity; ++i)
      {
        this->storage[i] ^= 0b11111111;
      }

      return *this;
    }

    void swap(bit_array &other) noexcept
    {
      if (this == &other) { return; }
      
      std::swap(this->storage, other.storage);
      std::swap(this->bits, other.bits);
      std::swap(this->blocks, other.blocks);
    }
    
    // Element access
    bool operator [] (i32 index) const noexcept
    {
      return this->storage[div_byte(index)] & (0b10000000 >> (mod_byte(index)));
    }

    bool front() const
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:front() -> invalid storage pointer (nullptr)");
      }

      return this->storage[0] & 0b10000000 ;
    }

    bool back() const
    {
      if (this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:back() -> invalid storage pointer (nullptr)");
      }

      const i32 index_position { this->bits - 1 };
      return this->storage[div_byte(index_position)] &
             (0b10000000 >> mod_byte(index_position));
    }

    bool at(i32 index) const
    {
      if (index < 0 || index >= this->bits || this->storage == nullptr)
      {
        throw std::out_of_range ("bit_array:at(i32) -> index is out of range");
      }

      return this->storage[div_byte(index)] & (0b10000000 >> mod_byte(index));
    }

    allocator_type get_allocator() const noexcept { return this->umalloc; }

    // Operations
    bit_array &operator = (const bit_array &other)
    {
      if (this != &other)
      {
        if (this->blocks != other.blocks)
        {
          this->umalloc.deallocate(this->storage, this->blocks);
          this->storage = umalloc.allocate(other.blocks);
        }

        std::memcpy(this->storage, other.storage, other.blocks);
        this->bits = other.bits;
        this->blocks = other.blocks;
      }

      return *this;
    }

    bit_array &operator = (bit_array &&other)
    {
      if (this != &other)
      {
        if (this->storage)
        {
          this->umalloc.deallocate(this->storage, this->blocks);
        }

        this->storage = other.storage;
        this->bits = other.bits;
        this->blocks = other.blocks;
        other.storage = nullptr;
        other.bits = other.blocks = 0;
      }

      return *this;
    }

    bit_array &operator &= (const bit_array &rhs)
    {
      if (this->bits != rhs.bits || !this->bits || !rhs.bits)
      {
        throw std::invalid_argument ("bit_array:operator(&=) -> invalid storage size");
      }

      for (i32 i {}; i < this->blocks; ++i)
      {
        this->storage[i] &= rhs.storage[i];
      }

      return *this;
    }
    
    bit_array &operator |= (const bit_array &rhs)
    {
      if (this->bits != rhs.bits || !this->bits || !rhs.bits)
      {
        throw std::invalid_argument ("bit_array:operator(|=) -> invalid storage size");
      }

      for (i32 i {}; i < this->blocks; ++i)
      {
        this->storage[i] |= rhs.storage[i];
      }

      return *this;
    }

    bit_array &operator ^= (const bit_array &rhs)
    {
      if (this->bits != rhs.bits || !this->bits || !rhs.bits)
      {
        throw std::invalid_argument ("bit_array:operator(^=) -> invalid storage size");
      }

      for (i32 i {}; i < this->blocks; ++i)
      {
        this->storage[i] ^= rhs.storage[i];
      }

      return *this;
    }
    
    bit_array operator ~ () const
    {
      try
      {
        bit_array<allocator_type> temp (*this);
        
        for (i32 i {}; i < temp.blocks; ++i)
        {
          temp.storage[i] = ~temp.storage[i];
        }

        return temp;
      }
      catch (std::bad_alloc &no_memory) { throw no_memory; }
    }

    /*bit_array &operator <<= (i32 shift)
    {
      if (shift < 0)
      {
        throw std::invalid_argument ("bit_array:bitwise and assignment operator(<<=) -> invalid shift");
      }
      else if (shift)
      {
        
      }

      return *this;
    }*/

    // TODO: bit_array &operator >>= (i32);
    
    std::string to_string() const noexcept
    {
      std::string storage_binary;
      storage_binary.reserve(this->bits);

      for (i32 i {}; i < this->bits; ++i)
      {
        if ((*this)[i]) { storage_binary.push_back('1'); }
        else { storage_binary.push_back('0'); }
      }

      return storage_binary;
    }
    
    // TODO: friend bit_array operator << (i32) const;
    // TODO: friend bit_array operator >> (i32) const;
 
  };

}

template<class allocator_type = std::allocator<std::uint8_t>>
bool operator == (const bits::bit_array<allocator_type> &lhs,
                  const bits::bit_array<allocator_type> &rhs)
{
  if (lhs.size() != rhs.size()) { return false; }

  return std::memcmp(lhs.data(),
                     rhs.data(),
                     lhs.capacity()) == 0;
}

template<class allocator_type = std::allocator<std::uint8_t>>
bool operator != (const bits::bit_array<allocator_type> &lhs,
                  const bits::bit_array<allocator_type> &rhs)
{
  return !(lhs == rhs);
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bit_array<allocator_type> operator & (const bits::bit_array<allocator_type> &lhs,
                                            const bits::bit_array<allocator_type> &rhs)
{
  try
  {
    bits::bit_array<allocator_type> tmp_obj (lhs);
    return tmp_obj &= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw incorrect_size; }
  catch (std::bad_alloc &no_memory) { throw no_memory; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bit_array<allocator_type> operator | (const bits::bit_array<allocator_type> &lhs,
                                            const bits::bit_array<allocator_type> &rhs)
{
  try
  {
    bits::bit_array<allocator_type> tmp_obj (lhs);
    return tmp_obj |= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw incorrect_size; }
  catch (std::bad_alloc &no_memory) { throw no_memory; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
bits::bit_array<allocator_type> operator ^ (const bits::bit_array<allocator_type> &lhs,
                                            const bits::bit_array<allocator_type> &rhs)
{
  try
  {
    bits::bit_array<allocator_type> tmp_obj (lhs);
    return tmp_obj ^= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw incorrect_size; }
  catch (std::bad_alloc &no_memory) { throw no_memory; }
}

#endif
