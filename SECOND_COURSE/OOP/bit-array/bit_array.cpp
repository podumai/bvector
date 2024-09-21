#include <iostream>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "bit_array.hpp"

#define BYTE              8
#define BIT_MASK_TEMPLATE 0xFF
#define BIT_MASK_INDEX    0x1
#define INT64_SIZE        8

namespace bits
{

  bit_array::bit_array() noexcept :
  storage (nullptr), storage_size (0), storage_capacity (0) {}

  bit_array::bit_array(std::int32_t num_bits, std::int64_t value) :
  bit_array()
  {
    if (num_bits < 0)
    {
      throw std::length_error ("Invalid number of bits");
    }
    else if (!num_bits)
    {
      return;
    }

    std::int32_t tmp_capacity { num_bits / BYTE + (num_bits % BYTE ? 1 : 0) };

    try
    {
      this->storage = new std::int8_t[tmp_capacity];
      this->storage_capacity = tmp_capacity;

      if (value)
      {
        std::memcpy(this->storage,
            &value,
            tmp_capacity >= INT64_SIZE ? INT64_SIZE : tmp_capacity);

      }

      this->storage_size = num_bits;
    }
    catch (std::bad_alloc &err)
    {
      throw err;
    }
  }

  bit_array::bit_array(const bit_array &obj) noexcept :
  storage (obj.data()), storage_size (obj.storage_size),
  storage_capacity (obj.storage_capacity) {}

  bit_array::bit_array(bit_array &&obj) noexcept :
  storage_size (obj.storage_size), storage_capacity (obj.storage_capacity),
  storage (obj.data())
  {
    obj.clear();
  }

  bit_array::~bit_array() { if (this->storage) { delete [] this->storage; } }

  std::int32_t bit_array::size() const noexcept
  {
    return this->storage_size;
  }

  std::int32_t bit_array::capacity() const noexcept
  {
    return this->storage_capacity;
  }

  std::int32_t bit_array::max_size() const noexcept { return INT32_MAX; }

  std::int8_t *bit_array::data() const
  {
    if (this->storage == nullptr) { return nullptr; }

    try
    {
      std::int8_t *tmp_ptr { new std::int8_t[this->storage_capacity] };

      std::memcpy(tmp_ptr, this->storage, this->storage_capacity);
      return tmp_ptr;
    }
    catch (std::bad_alloc &err)
    {
      throw err;
    }
  }

  std::int32_t bit_array::count() const noexcept
  {
    if (this->storage == nullptr) { return 0; }

    std::int32_t bit_count {};

    for (std::int32_t i {}; i < this->storage_capacity; ++i)
    {
      for (std::int8_t j {}; j < BYTE; ++j)
      {
        if (this->storage[i] & (BIT_MASK_INDEX << j)) { ++bit_count; }
      }
    }

    return bit_count;
  }

  void bit_array::shrink_to_fit()
  {
    if (this->storage == nullptr)
    {
      return;
    }
    else if (this->storage_size == 0)
    {
      delete [] this->storage;
      this->storage = nullptr;
      this->storage_size = this->storage_capacity = 0;
      return;
    }

    std::int32_t tmp_capacity { this->storage_size / BYTE };

    if (this->storage_size % BYTE) { ++tmp_capacity; }

    if (tmp_capacity < this->storage_capacity)
    {
      try
      {
        std::int8_t *tmp_ptr { new std::int8_t[tmp_capacity] };

        std::memcpy(tmp_ptr, this->storage, tmp_capacity);
        delete [] this->storage;

        this->storage = tmp_ptr;
        this->storage_capacity = tmp_capacity;
      }
      catch (std::bad_alloc &err)
      {
        throw err;
      }
    }
  }

  bool bit_array::any() const noexcept
  {
    for (std::int32_t i {}; i < this->storage_capacity; ++i)
    {
      if (this->storage[i / BYTE] & BIT_MASK_TEMPLATE) { return true; }
    }

    return false;
  }

  bool bit_array::none() const noexcept
  {
    for (std::int32_t i {}; i < this->storage_capacity; ++i)
    {
      if (this->storage[i / BYTE] & BIT_MASK_TEMPLATE) { return false; }
    }

    return true;
  }

  bool bit_array::empty() const noexcept { return this->storage_size == 0; }

  /*void bit_array::push_back(bool value)
  {
  }*/

  bool bit_array::pop_back()
  {
    if (this->storage_size == 0)
    {
      throw std::out_of_range ("Invalid storage size");
    }

    return this->storage[(this->storage_size - 1) / BYTE] &
           (BIT_MASK_INDEX << --this->storage_size % BYTE);
  }
 
  void bit_array::resize(std::int32_t num_bits, bool value)
  {
    if (num_bits < 0)
    {
      throw std::length_error ("Invalid number of bits");
    }
    else if (num_bits == 0)
    {
      if (this->storage)
      {
        delete [] this->storage;
        this->storage = nullptr;
        this->storage_size = this->storage_capacity = 0;
      }

      return;
    }

    std::int32_t tmp_capacity { num_bits / BYTE };

    if (num_bits % BYTE) { ++tmp_capacity; }
    
    try
    {
      std::int8_t *tmp_ptr { new std::int8_t[tmp_capacity] };

      if (tmp_capacity < this->storage_capacity)
      {
        if (this->storage)
        {
          std::memcpy(tmp_ptr, this->storage, tmp_capacity);
          delete [] this->storage;
        }

        this->storage = tmp_ptr;
        this->storage_size = std::min(this->storage_size,
            (this->storage_capacity - tmp_capacity) * BYTE);
        this->storage_capacity = tmp_capacity;
      }
      else if (tmp_capacity > this->storage_capacity)
      {
        if (this->storage)
        {
          std::memcpy(tmp_ptr, this->storage, this->storage_capacity);
          delete [] this->storage;
        }

        if (value) { std::memset(tmp_ptr,
            0xFF,
            tmp_capacity - this->storage_capacity); }

        this->storage = tmp_ptr;
        this->storage_capacity = tmp_capacity;
      }
    }
    catch (std::bad_alloc &err)
    {
      throw err;
    }
  }

  bit_array &bit_array::set(std::int32_t index, bool value)
  {
    if (index < 0 || index >= this->storage_size)
    {
      throw std::out_of_range ("Index is out of range");
    }
    else if (this->storage == nullptr)
    {
      return *this;
    }

    if (value)
    {
      this->storage[index / BYTE] |= (BIT_MASK_INDEX << index % BYTE);
    }
    else
    {
      this->storage[index / BYTE] &= ~(this->storage[index / BYTE]
                                  & (BIT_MASK_INDEX << index % BYTE));
    }

    return *this;
  }

  bit_array &bit_array::set() noexcept
  {
    if (this->storage == nullptr) { return *this; }

    std::memset(this->storage, 0xFF, this->storage_size / BYTE +
               (this->storage_size % BYTE ? 1 : 0));
    return *this;
  }
  
  bit_array &bit_array::reset(std::int32_t index)
  {
    if (index < 0 || index >= this->storage_size)
    {
      throw std::out_of_range ("Index is out of range");
    }
    else if (this->storage == nullptr)
    {
      return *this;
    }

    this->storage[index / BYTE] &= ~(this->storage[index / BYTE]
                                & (BIT_MASK_INDEX << index % BYTE));
    return *this;
  }

  bit_array &bit_array::reset() noexcept
  {
    if (this->storage == nullptr) { return *this; }

    std::memset(this->storage, 0, this->storage_size / BYTE +
               (this->storage_size % BYTE ? 1 : 0));
    return *this;
  }

  bit_array &bit_array::flip(std::int32_t index)
  {
    if (index < 0 || index >= this->storage_size)
    {
      throw std::out_of_range ("Index is out of range");
    }
    else if (this->storage == nullptr)
    {
      return *this;
    }

    this->storage[index / BYTE] ^= (BIT_MASK_INDEX << index % BYTE);
    return *this;
  }

  bit_array &bit_array::flip() noexcept
  {
    if (this->storage == nullptr) { return *this; }

    std::int32_t actual_capacity { this->storage_size / BYTE +
                                   this->storage_size % BYTE };

    for (std::int32_t i {}; i < actual_capacity; ++i)
    {
      this->storage[i] ^= BIT_MASK_TEMPLATE;
    }
    return *this;
  }
  
  void bit_array::swap(bit_array &obj)
  {
    if (this == &obj) { return; }

    this->shrink_to_fit();

    bit_array tmp_obj { std::move(*this) };
    
    *this = std::move(obj);
    obj = std::move(tmp_obj);
  }

  void bit_array::clear() noexcept
  {
    if (this->storage == nullptr) { return; }

    delete [] this->storage;
    this->storage = nullptr;
    this->storage_size = this->storage_capacity = 0;
  }

  bool bit_array::operator [] (std::int32_t index) const noexcept
  {
    return this->storage[index / BYTE] & (BIT_MASK_INDEX << (index % BYTE));
  }

  bool bit_array::front() const
  {
    if (this->storage == nullptr)
    {
      throw std::out_of_range ("Invalid storage pointer (nullptr)");
    }

    return this->storage[0] & BIT_MASK_INDEX;
  }

  bool bit_array::back() const
  {
    if (this->storage == nullptr)
    {
      throw std::out_of_range ("Invalid storage pointer (nullptr)");
    }

    return this->storage[(this->storage_size - 1) / BYTE] &
           (BIT_MASK_INDEX << (this->storage_size - 1) % BYTE);
  }

  bool bit_array::at(std::int32_t index) const
  {
    if (index < 0 || index >= this->storage_size || this->storage == nullptr)
    {
      throw std::out_of_range ("Index is out of range");
    }

    return this->storage[index / BYTE] & (BIT_MASK_INDEX << (index % BYTE));
  }

  bit_array &bit_array::operator = (const bit_array &obj)
  {
    if (this != &obj)
    {
      this->storage_size = obj.storage_size;
      this->storage_capacity = obj.storage_capacity;

      if (this->storage) { delete [] this->storage; }

      this->storage = obj.data();
    }

    return *this;
  }

  bit_array &bit_array::operator &= (const bit_array &obj)
  {
    if (this->storage_size != obj.storage_size)
    {
      throw std::invalid_argument ("Invalid storage size");
    }

    for (std::int32_t i {}; i < this->storage_capacity; ++i)
    {
      this->storage[i] &= obj.storage[i];
    }

    return *this;
  }

  bit_array &bit_array::operator |= (const bit_array &obj)
  {
    if (this->storage_size != obj.storage_size)
    {
      throw std::invalid_argument ("Invalid storage size");
    }

    for (std::int32_t i {}; i < this->storage_capacity; ++i)
    {
      this->storage[i] |= obj.storage[i];
    }

    return *this;
  }

  bit_array &bit_array::operator ^= (const bit_array &obj)
  {
    if (this->storage_size != obj.storage_size)
    {
      throw std::invalid_argument ("Invalid storage size");
    }

    for (std::int32_t i {}; i < this->storage_capacity; ++i)
    {
      this->storage[i] ^= obj.storage[i];
    }

    return *this;
  }

  std::string bit_array::to_string() const
  {
    std::string storage_binary;

    for (std::int32_t i { this->storage_size - 1}; i >= 0; --i)
    {
      if ((*this)[i]) { storage_binary.push_back('1'); }
      else { storage_binary.push_back('0'); }
    }

    return storage_binary;
  }

  bool operator == (const bit_array &l_arg, const bit_array &r_arg)
  {
    if (l_arg.storage_size != r_arg.storage_size) { return false; }

    return std::memcmp(l_arg.storage, r_arg.storage, l_arg.storage_capacity) == 0;
  }

  bool operator != (const bit_array &l_arg, const bit_array &r_arg)
  {
    if (l_arg.storage_size != r_arg.storage_size) { return true; }

    return std::memcmp(l_arg.storage, r_arg.storage, l_arg.storage_capacity) != 0;
  }

  bit_array operator & (const bit_array &l_arg, const bit_array &r_arg)
  {
    if (l_arg.storage_size != r_arg.storage_size)
    {
      throw std::invalid_argument ("Invalid size");
    }

    bit_array tmp_obj (l_arg.storage_size);

    for (std::int32_t i {}; i < tmp_obj.storage_capacity; ++i)
    {
      tmp_obj.storage[i] = l_arg.storage[i] & r_arg.storage[i];
    }

    return tmp_obj;
  }

  bit_array operator | (const bit_array &l_arg, const bit_array &r_arg)
  {
    if (l_arg.storage_size != r_arg.storage_size)
    {
      throw std::invalid_argument ("Invalid size");
    }

    bit_array tmp_obj (l_arg.storage_size);

    for (std::int32_t i {}; i < tmp_obj.storage_capacity; ++i)
    {
      tmp_obj.storage[i] = l_arg.storage[i] | r_arg.storage[i];
    }

    return tmp_obj;
  }

  bit_array operator ^ (const bit_array &l_arg, const bit_array &r_arg)
  {
    if (l_arg.storage_size != r_arg.storage_size)
    {
      throw std::invalid_argument ("Invalid size");
    }

    bit_array tmp_obj (l_arg.storage_size);

    for (std::int32_t i {}; i < tmp_obj.storage_capacity; ++i)
    {
      tmp_obj.storage[i] = l_arg.storage[i] ^ r_arg.storage[i];
    }

    return tmp_obj;
  }

}
