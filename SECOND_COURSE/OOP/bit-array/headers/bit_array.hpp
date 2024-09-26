#ifndef BIT_ARRAY_H_
#define BIT_ARRAY_H_

#include <cinttypes>
#include <string>

namespace bits
{
  
  class bit_array
  {
  private:
    std::int8_t *storage;
    std::int32_t storage_size,
                 storage_capacity;
  public:
    bit_array() noexcept;
    explicit bit_array(std::int32_t, std::int64_t = 0);
    bit_array(const bit_array &) noexcept;
    bit_array(bit_array &&)      noexcept;
    ~bit_array(); 
    
    // Capacity
    std::int32_t size()          const noexcept;
    std::int32_t capacity()      const noexcept;
    std::int32_t max_size()      const noexcept;
    std::int8_t  *data()         const;
    std::int32_t count()         const noexcept;
    void         shrink_to_fit();

    bool any()   const noexcept;
    bool none()  const noexcept;
    bool empty() const noexcept;   

    // Modifiers
    void clear() noexcept;
    void resize(std::int32_t, bool = false);
    // TODO: void push_back(bool);
    bool pop_back();
    bit_array &set(std::int32_t, bool = true);
    bit_array &set();
    bit_array &reset(std::int32_t);
    bit_array &reset();
    bit_array &flip(std::int32_t);
    bit_array &flip();
    void swap(bit_array &);
    
    // Element access
    bool operator [] (std::int32_t) const noexcept;
    bool front()                    const;
    bool back()                     const;
    bool at(std::int32_t)           const;

    // Operations
    bit_array &operator = (const bit_array &);
    bit_array &operator &= (const bit_array &);
    bit_array &operator |= (const bit_array &);
    bit_array &operator ^= (const bit_array &);
    // TODO: bit_array &operator <<= (std::int32_t);
    // TODO: bit_array &operator >>= (std::int32_t);
    std::string to_string() const;
    friend bool operator == (const bit_array &, const bit_array &);
    friend bool operator != (const bit_array &, const bit_array &);
    friend bit_array operator & (const bit_array &, const bit_array &);
    friend bit_array operator | (const bit_array &, const bit_array &);
    friend bit_array operator ^ (const bit_array &, const bit_array &);
    friend bit_array operator ~ (const bit_array &);
    // TODO: friend bit_array operator << (std::int32_t) const;
    // TODO: friend bit_array operator >> (std::int32_t) const;
 
  };

}

#endif
