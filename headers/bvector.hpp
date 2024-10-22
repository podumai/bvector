#ifndef __BITS_BVECTOR_H__
#define __BITS_BVECTOR_H__ 1

#include <cinttypes>
#include <string>
#include <cstring>

#if __WORDSIZE == 64
#define __BITS_BVECTOR_MAX_SIZE__      (0x2000000000ULL)
#define __BITS_BVECTOR_MAX_CAPACITY__  (0x400000000ULL)
#define __BITS_BVECTOR_MID_CAPACITY__  (0x100000000ULL)
#else
#define __BITS_BVECTOR_MAX_SIZE__     (0x20000000ULL)
#define __BITS_BVECTOR_MAX_CAPACITY__ (0x4000000ULL)
#define __BITS_BVECTOR_MID_CAPACITY__ (0x1000000ULL)
#endif

#define BYTE_DIVISION(bits) ((bits) >> 3)
#define BYTE_MODULE(bits) ((bits) & 0b00000111)
#define CALCULATE_CAPACITY(size) (((size) >> 3) + ((size) & 0b00000111 ? 1 : 0)) 
#define RESIZE_FACTOR(bits, bytes) ((bits) >> 3 == (bytes))

#define EMPTY_STORAGE (nullptr)
#define ZERO_VALUE (0ULL)
#define BIT_SET (true)
#define BIT_UNSET (false)
#define ANY_SET (true)
#define NONE_SET (true)

namespace bit
{

  template<class allocator_type = std::allocator<std::uint8_t>>
  class bvector
  {
    using pointer   = std::uint8_t*;
    using size_type = std::size_t;
    using reference = bvector<allocator_type>&;
    using bit_state = bool;
    enum BMASK
    {
      BIT   = 0b10000000,
      RESET = 0b00000000,
      SET   = 0b11111111
    };
  private:
    allocator_type xmalloc;
    pointer        m_storage;
    size_type      m_bits;
    size_type      m_bytes;
  public:
    constexpr bvector() noexcept : m_storage (EMPTY_STORAGE), m_bits (ZERO_VALUE),
    m_bytes (ZERO_VALUE) {}

    bvector(size_type bits_number, size_type value = ZERO_VALUE) :
    m_bits (bits_number), m_bytes (CALCULATE_CAPACITY(bits_number))
    {
      if (m_bits > __BITS_BVECTOR_MAX_SIZE__)
        throw std::length_error("bvector:bvector(size_type, size_type) -> invalid number of bits");

      try { m_storage = xmalloc.allocate(m_bytes); }
      catch (std::bad_alloc& error) { throw; }

      if (value != ZERO_VALUE)
        (void) std::memcpy(m_storage, &value, m_bytes >= 8 ? 8 : m_bytes);
    }

    bvector(const reference other) : m_bits (other.m_bits), m_bytes (other.m_bytes)
    {
      try
      {
        m_storage = xmalloc.allocate(m_bytes);
        (void) std::memcpy(m_storage, other.m_storage, m_bytes);
      }
      catch (std::bad_alloc& error) { throw; }
    }

    bvector(bvector<allocator_type>&& rvalue) noexcept : m_storage (rvalue.m_storage),
    m_bits (rvalue.m_bits), m_bytes (rvalue.m_bytes)
    {
      rvalue.m_storage = EMPTY_STORAGE;
      rvalue.m_bits = rvalue.m_bytes = ZERO_VALUE; 
    }

    ~bvector() { xmalloc.deallocate(m_storage, m_bytes); }
    
    // Capacity
    [[nodiscard]]
    size_type size() const noexcept { return m_bits; }
    
    [[nodiscard]]
    size_type capacity() const noexcept { return m_bytes; }

    [[nodiscard]]
    constexpr size_type max_size() const noexcept
    {
      return __BITS_BVECTOR_MAX_SIZE__;
    }
    
    [[nodiscard]]
    pointer data() const noexcept { return m_storage; }

    [[nodiscard]]
    constexpr allocator_type get_allocator() const noexcept
    {
      return xmalloc;
    }

    [[nodiscard]]
    size_type count() const noexcept
    {
      if (m_storage == EMPTY_STORAGE) return ZERO_VALUE;

      pointer end { m_storage + m_bytes };
      size_type bit_count {};

      for (pointer begin { m_storage }; begin != end; ++begin)
      {
        size_type byte = ((*begin >> 1) & 0b01010101) + (*begin & 0b01010101);
        byte = ((byte >> 2) & 0b00110011) + (byte & 0b00110011);
        byte = ((byte >> 4) & 0b00001111) + (byte & 0b00001111);
        bit_count += byte;
      }

      const size_type remaining_bits { BYTE_MODULE(m_bits) };

      for (size_type current_bit {}; current_bit != remaining_bits; ++current_bit)
        if (*end & BMASK::BIT >> BYTE_MODULE(current_bit)) ++bit_count;

      return bit_count;
    }

    void shrink_to_fit()
    {
      if (m_storage == EMPTY_STORAGE) return;
      else if (m_bits == ZERO_VALUE) { clear(); return; }

      const size_type current_bytes { CALCULATE_CAPACITY(m_bits) };

      if (current_bytes < m_bytes)
      {
        try
        {
          pointer tmp_ptr { xmalloc.allocate(current_bytes) };

          (void) memcpy(tmp_ptr, m_storage, current_bytes);
          xmalloc.deallocate(m_storage, m_bytes);

          m_storage = tmp_ptr;
          m_bytes = current_bytes;
        }
        catch (std::bad_alloc& error) { throw; }
      }
    }
    
    [[nodiscard]]
    bit_state any() const noexcept
    {
      pointer end { m_storage + m_bytes };

      for (pointer begin { m_storage }; begin != end; ++begin)
        if (*begin) return ANY_SET;

      const size_type remaining_bits { BYTE_MODULE(m_bits) };

      for (size_type current_bit {}; current_bit != remaining_bits; ++current_bit)
        if (*end & BMASK::BIT >> current_bit) return ANY_SET;

      return NONE_SET;
    }

    [[nodiscard]]
    bit_state none() const noexcept
    {
      pointer end { m_storage + m_bytes };

      for (pointer begin { m_storage }; begin != end; ++begin)
        if (*begin) return !ANY_SET;

      const size_type remaining_bits { BYTE_MODULE(m_bits) };

      for (size_type current_bit {}; current_bit != remaining_bits; ++current_bit)
        if (*end & BMASK::BIT >> current_bit) return !ANY_SET;

      return NONE_SET;
    }

    [[nodiscard]]
    bool empty() const noexcept { return m_bits == ZERO_VALUE; }

    // Modifiers
    void clear()
    {
      xmalloc.deallocate(m_storage, m_bytes);
      m_storage = EMPTY_STORAGE;
      m_bits = m_bytes = ZERO_VALUE;
    }

    void resize(size_type bits_number, bit_state value = BIT_UNSET)
    {
      if (bits_number == ZERO_VALUE) { clear(); return; }
      else if (bits_number == m_bits) return;

      const size_type new_size { CALCULATE_CAPACITY(bits_number) };

      try
      {
        pointer tmp_ptr { xmalloc.allocate(new_size) };

        if (m_storage != EMPTY_STORAGE)
        {
          (void) std::memcpy(tmp_ptr,
                             m_storage,
                             m_bytes > new_size ? new_size : m_bytes);
          xmalloc.deallocate(m_storage, m_bytes);
        }

        m_storage = tmp_ptr;
      }
      catch (std::bad_alloc& error) { throw; }
      
      if (m_bytes < new_size && value == BIT_SET)
        std::memset(m_storage + m_bytes, BMASK::SET, new_size - m_bytes);

      m_bits = bits_number;
      m_bytes = new_size;
    }

    void reserve(size_type bytes_number)
    {
      const size_type new_size { m_bytes + bytes_number };

      if (new_size > __BITS_BVECTOR_MAX_CAPACITY__)
        throw std::length_error("bvector:reserve() -> invalid number of bytes");
      else if (bytes_number == ZERO_VALUE || m_bytes == __BITS_BVECTOR_MAX_CAPACITY__)
        return;

      try
      {
        pointer tmp_ptr { xmalloc.allocate(new_size) };

        if (m_storage != EMPTY_STORAGE)
        {
          (void) std::memcpy(tmp_ptr, m_storage, m_bytes);
          xmalloc.deallocate(m_storage, m_bytes);
        }

        m_storage = tmp_ptr;
      }
      catch (std::bad_alloc& error) { throw; }

      m_bytes = new_size;
    }

    void push_back(bit_state value)
    {
      if (ZERO_VALUE < m_bytes && m_bytes < __BITS_BVECTOR_MAX_CAPACITY__)
      {
        if (RESIZE_FACTOR(m_bits, m_bytes))
        {
          size_type addition_size;

          if (m_bytes < __BITS_BVECTOR_MID_CAPACITY__)
            addition_size = m_bytes;
          else if (!(m_bytes + (addition_size = m_bytes >> 1) < __BITS_BVECTOR_MAX_CAPACITY__))
            addition_size = __BITS_BVECTOR_MAX_CAPACITY__ - m_bytes;

          try { reserve(addition_size); }
          catch (std::bad_alloc& error) { throw; }
        }

        if (value == BIT_SET)
          m_storage[BYTE_DIVISION(m_bits)] |= BMASK::BIT >> BYTE_MODULE(m_bits);
        else
          m_storage[BYTE_DIVISION(m_bits)] &= ~(BMASK::BIT >> BYTE_MODULE(m_bits));

        ++m_bits;
      }
      else if (m_bytes > ZERO_VALUE)
      {
        if (m_bits != __BITS_BVECTOR_MAX_SIZE__)
        {
          if (value == BIT_SET)
            m_storage[BYTE_DIVISION(m_bits)] |= BMASK::BIT >> BYTE_MODULE(m_bits);
          else
            m_storage[BYTE_DIVISION(m_bits)] &= ~(BMASK::BIT >> BYTE_MODULE(m_bits));

          ++m_bits;
        }
        else
        {
          if (value == BIT_SET)
            m_storage[BYTE_DIVISION(m_bits - 1)] |= BMASK::BIT >> BYTE_MODULE(m_bits - 1);
          else
            m_storage[BYTE_DIVISION(m_bits - 1)] &= ~(BMASK::BIT >> BYTE_MODULE(m_bits - 1));
        }
      }
      else
      {
        try { reserve(1); }
        catch (std::bad_alloc& error) { throw; }

        if (value == BIT_SET) *m_storage = BMASK::BIT;
        
        ++m_bits;
      }
    }

    [[nodiscard]]
    bit_state pop_back()
    {
      if (m_bits == ZERO_VALUE)
        throw std::out_of_range("bvector:pop_back() -> invalid number of bits");

      --m_bits;
      return m_storage[BYTE_DIVISION(m_bits)] & BMASK::BIT >> BYTE_MODULE(m_bits);
    }

    reference set(size_type index, bit_state value = BIT_UNSET)
    {
      if (index >= m_bits)
        throw std::out_of_range("bvector:set(size_type, bit_state) -> index is out of range");

      if (value == BIT_SET)
        m_storage[BYTE_DIVISION(index)] |= BMASK::BIT >> BYTE_MODULE(index);
      else
        m_storage[BYTE_DIVISION(index)] &= ~(BMASK::BIT >> BYTE_MODULE(index));

      return *this;
    }

    reference set()
    {
      if (m_bits == ZERO_VALUE)
        throw std::out_of_range("bvector:set() -> invalid number of bits");

      std::memset(m_storage, BMASK::SET, CALCULATE_CAPACITY(m_bits));
      return *this;
    }
    
    reference reset(size_type index)
    {
      if (index >= m_bits)
        throw std::out_of_range("bvector:reset(size_type) -> index is out of range");

      m_storage[BYTE_DIVISION(index)] &= ~(BMASK::BIT >> BYTE_MODULE(index));
      return *this;
    }
    
    reference reset()
    {
      if (m_bits == ZERO_VALUE)
        throw std::out_of_range("bvector:reset() -> invalid number of bits");

      std::memset(m_storage, BMASK::RESET, CALCULATE_CAPACITY(m_bits));
      return *this;
    }
    
    reference flip(size_type index)
    {
      if (index >= m_bits)
        throw std::out_of_range("bvector:flip(size_type) -> index is out of range");

      m_storage[BYTE_DIVISION(index)] ^= BMASK::BIT >> BYTE_MODULE(index);
      return *this;
    }

    reference flip()
    {
      if (m_bits == ZERO_VALUE)
        throw std::out_of_range("bvector:flip() -> invalid number of bits");

      pointer end { m_storage + CALCULATE_CAPACITY(m_bits) };

      for (pointer begin { m_storage }; begin != end; ++begin)
        *begin ^= BMASK::SET;

      return *this;
    }

    void swap(reference other) noexcept
    {
      if (this == &other) return;

      std::swap(m_storage, other.m_storage);
      std::swap(m_bits, other.m_bits);
      std::swap(m_bytes, other.m_bytes);
    }

    // Element access
    [[nodiscard]]
    bit_state operator[](size_type index) const noexcept
    {
      return m_storage[BYTE_DIVISION(index)] & BMASK::BIT >> BYTE_MODULE(index);
    }

    [[nodiscard]]
    bool front() const
    {
      if (m_bits == ZERO_VALUE)
        throw std::out_of_range("bvector:front() -> invalid number of bits");

      return *m_storage & BMASK::BIT;
    }
    
    [[nodiscard]]
    bit_state back() const
    {
      if (m_bits == ZERO_VALUE)
        throw std::out_of_range("bvector:back() -> invalid number of bits");

      return m_storage[BYTE_DIVISION(m_bits - 1)] & BMASK::BIT >> BYTE_MODULE(m_bits - 1);
    }
    
    [[nodiscard]]
    bit_state at(size_type index) const
    {
      if (index >= m_bits || m_storage == EMPTY_STORAGE)
        throw std::out_of_range("bvector:at(size_type) -> index is out of range");

      return m_storage[BYTE_DIVISION(index)] & BMASK::BIT >> BYTE_MODULE(index);
    }    

    // Operations
    reference operator=(const reference other)
    {
      if (this != &other)
      {
        if (m_bytes != other.m_bytes)
        {
          xmalloc.deallocate(m_storage, m_bytes);

          try { m_storage = xmalloc.allocate(other.m_bytes); }
          catch (std::bad_alloc& error) { throw; }
        }

        (void) std::memcpy(m_storage, other.m_storage, other.m_bytes);
        m_bits = other.m_bits;
        m_bytes = other.m_bytes;
      }

      return *this;
    }
    
    reference operator=(bvector<allocator_type>&& rvalue)
    {
      xmalloc.deallocate(m_storage, m_bytes);
      m_storage = EMPTY_STORAGE;
      std::swap(m_storage, rvalue.m_storage);

      m_bits = rvalue.m_bits;
      m_bytes = rvalue.m_bytes;
      rvalue.m_bits = rvalue.m_bytes = ZERO_VALUE;

      return *this;
    }
    
    reference operator&=(const reference rhs)
    {
      if (m_bits != rhs.m_bits || m_bits != ZERO_VALUE || rhs.m_bits != ZERO_VALUE)
        throw std::invalid_argument("bvector:operator(&=) -> invalid storage size");

      pointer begin_lhs { m_storage };
      pointer begin_rhs { rhs.storage };
      pointer end { m_storage + m_bytes };

      while (begin_lhs != end)
      {
        *begin_lhs &= *begin_rhs;
        ++begin_lhs;
        ++begin_rhs;
      }

      return *this;
    }
    
    reference operator|=(const reference rhs)
    {
      if (m_bits != rhs.m_bits || m_bits != ZERO_VALUE || rhs.m_bits != ZERO_VALUE)
        throw std::invalid_argument("bvector:operator(|=) -> invalid storage size");

      pointer begin_lhs { m_storage };
      pointer begin_rhs { rhs.m_storage };
      pointer end { m_storage + m_bytes };

      while (begin_lhs != end)
      {
        *begin_lhs |= *begin_rhs;
        ++begin_lhs;
        ++begin_rhs;
      }

      return *this;
    }
    
    reference operator^=(const reference rhs)
    {
      if (m_bits != rhs.m_bits || m_bits != ZERO_VALUE || rhs.m_bits != ZERO_VALUE)
        throw std::invalid_argument("bvector:operator(^=) -> invalid storage size");

      pointer begin_lhs { m_storage };
      pointer begin_rhs { rhs.m_storage };
      pointer end { m_storage + m_bytes };

      while (begin_lhs != end)
      {
        *begin_lhs ^= *begin_rhs;
        ++begin_lhs;
        ++begin_rhs;
      }

      return *this;
    }
    
    [[nodiscard]]
    bvector<allocator_type> operator~() const
    {
      try
      {
        bvector<allocator_type> temp (*this);

        pointer end { temp.m_storage + temp.m_bytes };

        for (pointer begin { temp.m_storage }; begin != end; ++begin)
          *begin ^= *begin;

        return temp;
      }
      catch (std::bad_alloc& error) { throw; }
    }

    [[deprecated]]
    reference operator>>=(size_type shift);
    [[deprecated]]
    reference operator<<=(size_type shift);

    [[nodiscard]]
    std::string to_string() const
    {
      std::string binary_string;

      try { binary_string.reserve(m_bits); }
      catch (std::bad_alloc& error) { throw; }

      for (size_type current_bit {}; current_bit != m_bits; ++current_bit)
        binary_string.push_back(
        static_cast<bool> (m_storage[BYTE_DIVISION(current_bit)] & BMASK::BIT >>
                           BYTE_MODULE(current_bit)) + '0'
        );

      return binary_string;
    }
  };

}

#undef CALCULATE_CAPACITY
#undef RESIZE_FACTOR
#undef EMPTY_STORAGE
#undef ZERO_VALUE
#undef BIT_SET
#undef BIT_UNSET
#undef ANY_SET
#undef NONE_SET

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bool operator==(const bit::bvector<allocator_type>& lhs,
                const bit::bvector<allocator_type>& rhs)
{
  if (lhs.size() != rhs.size()) return false;

  return std::memcmp(lhs.data(), rhs.data(), lhs.capacity()) == 0;
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bool operator!=(const bit::bvector<allocator_type>& lhs,
                const bit::bvector<allocator_type>& rhs)
{
  return !(lhs == rhs);
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bit::bvector<allocator_type> operator&(const bit::bvector<allocator_type>& lhs,
                                       const bit::bvector<allocator_type>& rhs)
{
  try
  {
    bit::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj &= rhs;
  }
  catch (std::invalid_argument& incorrect_size) { throw; }
  catch (std::bad_alloc& error) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bit::bvector<allocator_type> operator|(const bit::bvector<allocator_type>& lhs,
                                       const bit::bvector<allocator_type>& rhs)
{
  try
  {
    bit::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj |= rhs;
  }
  catch (std::invalid_argument& incorrect_size) { throw; }
  catch (std::bad_alloc& error) { throw; }
}

template<class allocator_type = std::allocator<std::uint8_t>>
[[nodiscard]]
bit::bvector<allocator_type> operator^(const bit::bvector<allocator_type>& lhs,
                                       const bit::bvector<allocator_type>& rhs)
{
  try
  {
    bit::bvector<allocator_type> tmp_obj (lhs);
    return tmp_obj ^= rhs;
  }
  catch (std::invalid_argument& incorrect_size) { throw; }
  catch (std::bad_alloc& error) { throw; }
}

#endif
