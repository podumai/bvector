#include <cstdlib>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <memory>
#include "bvector.hpp"

#define __BMASK_BIT__ (0b10000000)
#define __BMASK_TRUE_BYTE__ (0b11111111)
#define __BMASK_FALSE_BYTE__ (0b00000000)
#define __BMASK_MOD__ (0b00000111)
#define __DEFBITS_PER_BYTE__ (8)

#define calculate_capacity(size) (((size) >> 3) + ((size) & 0b00000111 ? 1 : 0)) 
#define byte_division(size) ((size) >> 3)
#define byte_module(size) ((size) & 0b00000111)
#define resize_factor(bits, bytes) ((bits) >> 3 == (bytes))

using namespace bits;

bvector::bvector() noexcept : storage (nullptr), bits (0), bytes (0) { }

bvector::bvector(size_type bits_number, size_type value) :
bits (bits_number), bytes (calculate_capacity(bits_number))
{
  if (this->bits > __BITS_BVECTOR_MAX_SIZE__)
  {
    throw std::length_error ("bvector:bvector(size_type, size_type) -> invalid number of bits");
  }

  try { this->storage = this->xmalloc.allocate(this->bytes); }
  catch (std::bad_alloc &error) { throw; }

  if (value)
  {
    (void) std::memcpy(this->storage,
                       &value,
                       this->bytes >= 8 ? 8 : this->bytes);
  }
}

bvector::bvector(const bvector &other) : bits (other.bits), bytes (other.bytes)
{
  try
  {
    this->storage = this->xmalloc.allocate(this->bytes);
    (void) std::memcpy(this->storage, other.storage, this->bytes);
  }
  catch (std::bad_alloc &error) { throw; }
}

bvector::bvector(bvector &&rvalue) noexcept : storage (rvalue.storage),
bits (rvalue.bits), bytes (rvalue.bytes)
{
  rvalue.storage = nullptr;
  rvalue.bits = rvalue.bytes = 0;
}

bvector::~bvector() { this->xmalloc.deallocate(&this->storage); }

// Capacity
std::size_t bvector::size() const noexcept { return this->bits; }

std::size_t bvector::capacity() const noexcept { return this->bytes; }

constexpr std::size_t bvector::max_size() const noexcept
{
  return __BITS_BVECTOR_MAX_SIZE__;
}

std::uint8_t *bvector::data() const noexcept { return this->storage; }

memory::xallocator<std::uint8_t> bvector::get_allocator() const noexcept { return this->xmalloc; }

std::size_t bvector::count() const noexcept
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

  for (size_type i {}; i < byte; ++i)
  {
    if (this->storage[tmp_bytes] & __BMASK_BIT__ >> byte_module(i))
    {
      ++bit_count;
    }
  }

  return bit_count;
}

void bvector::shrink_to_fit()
{
  if (this->storage == nullptr) { return; }
  else if (this->bits == 0) { this->clear(); return; }

  const size_type tmp_bytes { calculate_capacity(this->bits) };

  if (tmp_bytes < this->bytes)
  {
    this->bytes = tmp_bytes;

    try { this->xmalloc.reallocate(this->storage, this->bytes); }
    catch (std::bad_alloc &error) { throw; }
  }
}

bool bvector::any() const noexcept
{
  size_type tmp_bytes { byte_division(this->bits) };

  for (size_type i {}; i != tmp_bytes; ++i)
  {
    if (this->storage[i]) { return true; }
  }

  tmp_bytes = byte_module(this->bits);

  for (size_type i {}; i != tmp_bytes; ++i)
  {
    if (this->storage[tmp_bytes] & __BMASK_BIT__ >> i) { return true; }
  }

  return false;
}

bool bvector::none() const noexcept
{
  size_type tmp_bytes { byte_division(this->bits) };

  for (size_type i {}; i != this->bytes; ++i)
  {
    if (this->storage[i]) { return false; }
  }

  tmp_bytes = byte_module(this->bits);

  for (size_type i {}; i != tmp_bytes; ++i)
  {
    if (this->storage[tmp_bytes] & __BMASK_BIT__ >> i)
    {
      return false;
    }
  }

  return true;
}

bool bvector::empty() const noexcept { return this->bits == 0; }

// Modifiers
void bvector::clear()
{
  if (this->storage == nullptr) { return; }

  this->xmalloc.deallocate(&this->storage);
  this->bits = this->bytes = 0;
}

void bvector::resize(size_type bits_number, bool value)
{
  if (bits_number == 0) { this->clear(); return; }
  else if (bits_number == this->bits) { return; }

  const size_type new_size { calculate_capacity(bits_number) };

  try
  {
    if (this->storage)
    {
      this->xmalloc.reallocate(this->storage, new_size);

      if (new_size > this->bytes)
      {
        std::memset(this->storage + this->bytes,
                   value ? __BMASK_TRUE_BYTE__ : __BMASK_FALSE_BYTE__,
                    new_size - this->bytes);
      }
    }
    else
    {
      this->storage = this->xmalloc.allocate(new_size);

      std::memset(this->storage,
                  value ? __BMASK_TRUE_BYTE__ : __BMASK_FALSE_BYTE__,
                  new_size);
    }
  }
  catch (std::bad_alloc &error) { return; }

  this->bits = bits_number;
  this->bytes = new_size;
}

void bvector::reserve(size_type bytes_number)
{
  const size_type new_size { this->bytes + bytes_number };

  if (new_size > __BITS_BVECTOR_MAX_CAPACITY__)
  {
    throw std::length_error ("bvector:reserve() -> invalid number of bytes");
  }
  else if (bytes_number == 0 || this->bytes == __BITS_BVECTOR_MAX_CAPACITY__)
  {
    return;
  }
  
  try
  {
    if (this->storage)
    {
      this->xmalloc.reallocate(this->storage, new_size);
    }
    else
    {
      this->storage = this->xmalloc.allocate(new_size);
    }
  }
  catch (std::bad_alloc &error) { throw; }

  this->bytes = new_size;
}

void bvector::push_back(bool value)
{
  if (this->bytes && this->bytes < __BITS_BVECTOR_MAX_CAPACITY__)
  {
    if (resize_factor(this->bits, this->bytes))
    {
      size_type addition_size;

      if (this->bytes < __BITS_BVECTOR_MID_CAPACITY__)
      {
        addition_size = this->bytes;
      }
      else if (!(this->bytes + (addition_size = this->bytes >> 1) <
               __BITS_BVECTOR_MAX_CAPACITY__))
      {
        addition_size = __BITS_BVECTOR_MAX_CAPACITY__ - this->bytes;
      }

      try { this->reserve(addition_size); }
      catch (std::bad_alloc &error) { throw; }
    }

    /*if (value)
    {
      this->storage[byte_division(this->bits)] |=
        __BMASK_BIT__ >> byte_module(this->bits);
    }
    else
    {
      this->storage[byte_division(this->bits)] &=
        ~(__BMASK_BIT__ >> byte_module(this->bits));
    }*/
    
    ++this->bits;
  }
  else if (this->bytes)
  {
    if (this->bits < __BITS_BVECTOR_MAX_SIZE__)
    {
      if (value)
      {
        this->storage[byte_division(this->bits)] |=
          __BMASK_BIT__ >> byte_module(this->bits);
      }
      else
      {
        this->storage[byte_division(this->bits)] &=
          ~(__BMASK_BIT__ >> byte_module(this->bits));
      }

      ++this->bits;
    }
    else
    {
      if (value)
      {
        this->storage[byte_division(this->bits - 1)] |=
          __BMASK_BIT__ >> byte_module(this->bits - 1);
      }
      else
      {
        this->storage[byte_division(this->bits - 1)] &=
          ~(__BMASK_BIT__ >> byte_module(this->bits - 1));
      }
    }
  }
  else
  {
    try { this->reserve(1); }
    catch (std::bad_alloc &error) { throw; }

    if (value) { *(this->storage) = __BMASK_BIT__; }

    ++this->bits;
  }
}

bool bvector::pop_back()
{
  if (this->bits == 0)
  {
    throw std::out_of_range ("bvector:pop_back() -> invalid storage size");
  }

  const bool value
  { bool (this->storage[byte_division(this->bits)] & __BMASK_BIT__ >> byte_module(this->bits)) };
  --this->bits;
  return value;
}

bvector &bvector::set(size_type index, bool value)
{
  if (index >= this->bits)
  {
    throw std::out_of_range ("bvector:set(size_type, bool) -> index is out of range");
  }
  else if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:set(size_type, bool) -> invalid storage pointer (nullptr)");
  }

  if (value)
  {
    this->storage[byte_division(index)] |= __BMASK_BIT__ >> byte_module(index);
  }
  else
  {
    this->storage[byte_division(index)] &= ~(__BMASK_BIT__ >> byte_module(index));
  }

  return *this;
}

bvector &bvector::set()
{
  if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:set() -> invalid storage pointer (nullptr)");
  }

  std::memset(this->storage,
              __BMASK_TRUE_BYTE__,
              calculate_capacity(this->bits));
  return *this;
}

bvector &bvector::reset(size_type index)
{
  if (index >= this->bits)
  {
    throw std::out_of_range ("bvector:reset(i32) -> index is out of range");
  }
  else if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:reset(i32) -> invalid storage pointer (nullptr)");
  }

  this->storage[byte_division(index)] &= ~(__BMASK_BIT__ >> byte_module(index));
  return *this;
}

bvector &bvector::reset()
{
  if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:reset() -> invalid storage pointer (nullptr)");
  }

  std::memset(this->storage,
              __BMASK_FALSE_BYTE__,
              calculate_capacity(this->bits));
  return *this;
}

bvector &bvector::flip(size_type index)
{
  if (index >= this->bits)
  {
    throw std::out_of_range ("bvector:flip(i32) -> index is out of range");
  }
  else if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:flip(i32) -> invalid storage pointer (nullptr)");
  }

  this->storage[byte_division(index)] ^= __BMASK_BIT__ >> byte_module(index);

  return *this;
}

bvector &bvector::flip()
{
  if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:flip() -> invalid storage pointer (nullptr)");
  }

  const size_type tmp_capacity { calculate_capacity(this->bits) };

  for (size_type i {}; i != tmp_capacity; ++i)
  {
    this->storage[i] ^= __BMASK_TRUE_BYTE__;
  }

  return *this;
}

void bvector::swap(bvector &other) noexcept
{
  if (this == &other) { return; }

  std::swap(this->storage, other.storage);
  std::swap(this->bits, other.bits);
  std::swap(this->bytes, other.bytes);
}

// Element access
bool bvector::operator [] (size_type index) const noexcept
{
  return this->storage[byte_division(index)] & __BMASK_BIT__ >> byte_module(index);
}

bool bvector::front() const
{
  if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:front() -> invalid storage pointer (nullptr)");
  }

  return this->storage[0] & __BMASK_BIT__ ;
}

bool bvector::back() const
{
  if (this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:back() -> invalid storage pointer (nullptr)");
  }

  return this->storage[byte_division(this->bits - 1)] &
    __BMASK_BIT__ >> byte_module(this->bits - 1);
}

bool bvector::at(size_type index) const
{
  if (index >= this->bits || this->storage == nullptr)
  {
    throw std::out_of_range ("bvector:at(size_type) -> index is out of range");
  }

  return this->storage[byte_division(index)] & __BMASK_BIT__ >> byte_module(index);
}

// Operations
bvector &bvector::operator = (const bvector &other)
{
  if (this != &other)
  {
    if (this->bytes != other.bytes)
    {
      try { xmalloc.reallocate(this->storage, other.bytes); }
      catch (std::bad_alloc &error) { throw; }
    }

    (void) std::memcpy(this->storage, other.storage, other.bytes);
    this->bits = other.bits;
    this->bytes = other.bytes;
  }

  return *this;
}

bvector &bvector::operator = (bvector &&rvalue)
{
  this->xmalloc.deallocate(&this->storage);
  std::swap(this->storage, rvalue.storage);

  this->bits = rvalue.bits;
  this->bytes = rvalue.bytes;
  rvalue.bits = rvalue.bytes = 0;

  return *this;
}

bvector &bvector::operator &= (const bvector &rhs)
{
  if (this->bits != rhs.bits || !this->bits || !rhs.bits)
  {
    throw std::invalid_argument ("bvector:operator(&=) -> invalid storage size");
  }

  for (size_type i {}; i != this->bytes; ++i)
  {
    this->storage[i] &= rhs.storage[i];
  }

  return *this;
}

bvector &bvector::operator |= (const bvector &rhs)
{
  if (this->bits != rhs.bits || !this->bits || !rhs.bits)
  {
    throw std::invalid_argument ("bvector:operator(|=) -> invalid storage size");
  }

  for (size_type i {}; i != this->bytes; ++i)
  {
    this->storage[i] |= rhs.storage[i];
  }

  return *this;
}

bvector &bvector::operator ^= (const bvector &rhs)
{
  if (this->bits != rhs.bits || !this->bits || !rhs.bits)
  {
    throw std::invalid_argument ("bvector:operator(^=) -> invalid storage size");
  }

  for (size_type i {}; i != this->bytes; ++i)
  {
    this->storage[i] ^= rhs.storage[i];
  }

  return *this;
}

bvector bvector::operator ~ () const
{
  try
  {
    bvector temp (*this);

    for (size_type i {}; i != temp.bytes; ++i)
    {
      temp.storage[i] = ~temp.storage[i];
    }

    return temp;
  }
  catch (std::bad_alloc &error) { throw; }
}

std::string bvector::to_string() const
{
  std::string storage_binary;

  try { storage_binary.reserve(this->bits); }
  catch (std::bad_alloc &error) { throw; }

  for (size_type i {}; i < this->bits; ++i)
  {
    if (this->storage[byte_division(i)] & __BMASK_BIT__ >> byte_module(i))
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

bool operator == (const bvector &lhs, const bvector &rhs)
{
  if (lhs.size() != rhs.size()) { return false; }

  return std::memcmp(lhs.data(), rhs.data(), lhs.capacity()) == 0;
}

bool operator != (const bvector &lhs, const bvector &rhs)
{
  return !(lhs == rhs);
}

bvector operator & (const bvector &lhs, const bvector &rhs)
{
  try
  {
    bvector tmp_obj (lhs);
    return tmp_obj &= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &error) { throw; }
}

bvector operator | (const bvector &lhs, const bvector &rhs)
{
  try
  {
    bvector tmp_obj (lhs);
    return tmp_obj |= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &error) { throw; }
}

bvector operator ^ (const bvector &lhs, const bvector &rhs)
{
  try
  {
    bvector tmp_obj (lhs);
    return tmp_obj ^= rhs;
  }
  catch (std::invalid_argument &incorrect_size) { throw; }
  catch (std::bad_alloc &error) { throw; }
}
