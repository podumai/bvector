#ifndef MY_LIB_MATRIX_H_
#define MY_LIB_MATRIX_H_

#include <cstdint>
#include <cstring>
#include <typeinfo>
#include <stdexcept>

namespace my_lib
{

  namespace linear_algebra
  {
    typedef float f32;
    typedef double f64;
    typedef long double f128;

    template<typename T>
    class matrix
    {
    private:
      std::int32_t rows, columns;
      T **storage_ptr;
    public:
      matrix(std::int32_t r, std::int32_t c)
      {
        if (typeid(T) != typeid(std::int8_t) &&
            typeid(T) != typeid(std::int16_t) &&
            typeid(T) != typeid(std::int32_t) &&
            typeid(T) != typeid(std::int64_t) &&
            typeid(T) != typeid(f32) &&
            typeid(T) != typeid(f64) &&
            typeid(T) != typeid(f128))
        {
          throw std::invalid_argument ("Arithmetic type is required.");
        }

        if (r <= 0 || c <= 0)
        {
          throw std::out_of_range ("Invalid matrix size.");
        }

        this->rows = r;
        this->columns = c;

        try
        {
          storage_ptr = new T*[r];

          for (std::int32_t i {}; i < r; ++i)
          {
            storage_ptr[i] = new T[c];
          }
        }
        catch (std::bad_alloc &err)
        {
          throw err;
        }
      }

      explicit matrix(std::int32_t size) : matrix(size, size) {}

      matrix(const matrix &obj_matrix)
      {
        this->rows = obj_matrix.rows;
        this->columns = obj_matrix.columns;
        
        try
        {
          this->storage_ptr = new T*[this->rows];
          for (std::int32_t i {}; i < this->rows; ++i)
          {
            this->storage_ptr[i] = new T[this->columns];
            std::memcpy(this->storage_ptr[i],
                        obj_matrix.storage_ptr[i],
                        sizeof(T) * this->columns);
          }
        }
        catch (std::bad_alloc &err)
        {
          throw err;
        }
      }

      /*matrix(matrix &&obj_matrix)
      {
        if (typeid(this->storage_ptr) != typeid(obj_matrix.storage_ptr))
        {
          throw std::invalid_argument ("Incorrect matrix types.");
        }

        for (std::int32_t i {}; i < this->rows; ++i)
        {
          delete [] this->storage_ptr[i];
        }
        delete [] this->storage_ptr;
        
        this->rows = obj_matrix.rows;
        this->columns = obj_matrix.columns;
        this->storage_ptr = obj_matrix.storage_ptr;
        obj_matrix.rows = obj_matrix.columns = 0;
        obj_matrix.storage_ptr = nullptr;
      }*/

      ~matrix()
      {
        for (std::int32_t i {}; i < this->rows; ++i)
        {
          delete [] storage_ptr[i];
        }
        delete [] storage_ptr;
      }

      matrix &operator = (const matrix &obj_matrix)
      {
        if (this != &obj_matrix)
        {
          if (this->rows != obj_matrix.rows ||
              this->columns != obj_matrix.columns)
          {
            for (std::int32_t i {}; i < this->rows; ++i)
            {
              delete [] this->storage_ptr[i];
            }
            delete [] this->storage_ptr;

            this->rows = obj_matrix.rows;
            this->columns = obj_matrix.columns;

            try
            {
              this->storage_ptr = new T*[this->rows];
              for (std::int32_t i {}; i < this->rows; ++i)
              {
                this->storage_ptr[i] = new T[this->columns];
              }
            }
            catch (std::bad_alloc &err)
            {
              throw err;
            }
          }

          for (std::int32_t i {}; i < this->rows; ++i)
          {
            std::memcpy(this->storage_ptr[i],
                        obj_matrix.storage_ptr[i],
                        sizeof(T) * this->columns);
          }
        }

        return *this;
      }

      constexpr std::int32_t get_rows() const noexcept { return this->rows; }
      constexpr std::int32_t get_columns() const noexcept { return this->columns; }

      T &at(std::int32_t row_index, std::int32_t column_index)
      {
        if (row_index < 0 || row_index >= this->rows ||
            column_index < 0 || column_index >= this->columns)
        {
          throw std::out_of_range ("Invalid index.");
        }

        return this->storage_ptr[row_index][column_index];
      }

      void fill(T value)
      {
        for (std::int32_t i {}; i < this->rows; ++i)
        {
          for (std::int32_t j {}; j < this->columns; ++j)
          {
            this->storage_ptr[i][j] = value;
          }
        }
      }

      void make_identity()
      {
        if (this->rows != this->columns)
        {
          throw std::invalid_argument ("Invalid matrix size. Square matrix required.");
        }

        for (std::int32_t i {}; i < this->rows; ++i)
        {
          std::memset(this->storage_ptr[i], 0, sizeof(T) * this->columns);
          this->storage_ptr[i][i] = 1;
        }
      }

      matrix operator + (const matrix &obj_matrix)
      {
        if (this->rows != obj_matrix.rows ||
            this->columns != obj_matrix.columns)
        {
          throw std::out_of_range ("Invalid matrix sizes.");
        }
        else if (typeid(this->storage_ptr) != typeid(obj_matrix.storage_ptr))
        {
          throw std::invalid_argument ("Incorrect matrix types.");
        }

        matrix tmp (this->rows, this->columns);

        for (std::int32_t i {}; i < this->rows; ++i)
        {
          for (std::int32_t j {}; j < this->columns; ++j)
          {
            tmp.storage_ptr[i][j] = this->storage_ptr[i][j] +
                                    obj_matrix.storage_ptr[i][j];
          }
        }
        return tmp;
      }

      matrix operator - (const matrix &obj_matrix)
      {
        if (this->rows != obj_matrix.rows ||
            this->columns != obj_matrix.columns)
        {
          throw std::out_of_range ("Invalid matrix sizes.");
        }
        else if (typeid(this->storage_ptr) != typeid(obj_matrix.storage_ptr))
        {
          throw std::invalid_argument ("Incorrect matrix types.");
        }

        matrix tmp (this->rows, this->columns);

        for (std::int32_t i {}; i < this->rows; ++i)
        {
          for (std::int32_t j {}; j < this->columns; ++j)
          {
            tmp.storage_ptr[i][j] = this->storage_ptr[i][j] -
                                    obj_matrix.storage_ptr[i][j];
          }
        }
        return tmp;
      }
      // TODO: need to finish the asterics operation
      matrix operator * (const matrix &obj_matrix)
      {
        if (this->rows != obj_matrix.columns ||
            this->columns != obj_matrix.rows)
        {
          throw std::invalid_argument ("Incompatible matrix sizes.");
        }
        else if (typeid(this->storage_ptr) != typeid(obj_matrix.storage_ptr))
        {
          throw std::invalid_argument ("Incorrect matrix types.");
        }

        matrix tmp (obj_matrix.rows, this->columns);

        for (std::int32_t i {}; i < this->columns; ++i)
        {
          for (std::int32_t j {}; j < this->columns; ++i)
          {
            for (std::int32_t k {}; k < obj_matrix.rows; ++k)
            {
              tmp.storage_ptr[i][j] += this->storage_ptr[j][k] *
                                       obj_matrix.storage_ptr[k][j];
            }
          }
        }
        return tmp;
      }

      matrix &operator += (matrix &obj_matrix)
      {
        if (this->rows != obj_matrix.rows ||
            this->columns != obj_matrix.columns)
        {
          throw std::out_of_range ("Invalid matrix sizes.");
        }
        else if (typeid(this->storage_ptr) != typeid(obj_matrix.storage_ptr))
        {
          throw std::invalid_argument ("Incorrect matrix types.");
        }

        for (std::int32_t i {}; i < this->rows; ++i)
        {
          for (std::int32_t j {}; j < this->columns; ++j)
          {
            this->storage_ptr[i][j] += obj_matrix.storage[i][j];
          }
        }
        return *this;
      }

      matrix &operator -= (matrix &obj_matrix)
      {
        if (this->rows != obj_matrix.rows ||
            this->columns != obj_matrix.columns)
        {
          throw std::out_of_range ("Invalid matrix sizes.");
        }
        else if (typeid(this->storage_ptr) != typeid(obj_matrix.storage_ptr))
        {
          throw std::invalid_argument ("Incorrect matrix types.");
        }

        for (std::int32_t i {}; i < this->rows; ++i)
        {
          for (std::int32_t j {}; j < this->columns; ++j)
          {
            this->storage_ptr[i][j] -= obj_matrix.storage[i][j];
          }
        }
        return *this;
      }

      friend bool operator == (const matrix &l_matrix, const matrix &r_matrix)
      {
        if (l_matrix.rows != r_matrix.rows ||
            l_matrix.columns != r_matrix.columns)
        {
          return false;
        }

        for (std::int32_t i {}; i < l_matrix.rows; ++i)
        {
          for (std::int32_t j {}; j < l_matrix.columns; ++j)
          {
            if (l_matrix.storage_ptr[i][j] != r_matrix.storage_ptr[i][j])
            {
              return false;
            }
          }
        }
        return true;
      }

      friend bool operator != (const matrix &l_matrix, const matrix &r_matrix)
      {
        return (l_matrix == r_matrix) == false;
      }

      friend bool operator < (const matrix &l_matrix, const matrix &r_matrix)
      {
        if (l_matrix.rows != r_matrix.rows ||
            l_matrix.columns != r_matrix.columns)
        {
          throw std::invalid_argument ("Incompitable matrix sizes.");
        }

        std::int32_t LGT { 0 }, RGT { 0 };

        for (std::int32_t i {}; i < l_matrix.rows; ++i)
        {
          for (std::int32_t j {}; j < l_matrix.columns; ++j)
          {
            if (l_matrix.storage_ptr[i][j] < r_matrix.storage_ptr[i][j])
            {
              ++RGT;
            }
            else
            {
              ++LGT;
            }
          }
        }
        return LGT < RGT;
      }

      friend bool operator <= (const matrix &l_matrix, const matrix &r_matrix)
      {
        if (l_matrix.rows != r_matrix.rows ||
            l_matrix.columns != r_matrix.columns)
        {
          throw std::invalid_argument ("Incompitable matrix sizes.");
        }

        std::int32_t L_LEQ { 0 }, R_GEQ { 0 };

        for (std::int32_t i {}; i < l_matrix.rows; ++i)
        {
          for (std::int32_t j {}; j < l_matrix.columns; ++j)
          {
            if (l_matrix.storage_ptr[i][j] <= r_matrix.storage_ptr[i][j])
            {
              ++R_GEQ;
            }
            else
            {
              ++L_LEQ;
            }
          }
        }
        return L_LEQ <= R_GEQ;
      }

      friend bool operator > (const matrix &l_matrix, const matrix &r_matrix)
      {
        if (l_matrix.rows != r_matrix.rows ||
            l_matrix.columns != r_matrix.columns)
        {
          throw std::invalid_argument ("Incompitable matrix sizes.");
        }

        std::int32_t LGT { 0 }, RGT { 0 };

        for (std::int32_t i {}; i < l_matrix.rows; ++i)
        {
          for (std::int32_t j {}; j < l_matrix.columns; ++j)
          {
            if (l_matrix.storage_ptr[i][j] > r_matrix.storage_ptr[i][j])
            {
              ++LGT;
            }
            else
            {
              ++RGT;
            }
          }
        }
        return LGT > RGT;
      }

      friend bool operator >= (const matrix &l_matrix, const matrix &r_matrix)
      {
        if (l_matrix.rows != r_matrix.rows ||
            l_matrix.columns != r_matrix.columns)
        {
          throw std::invalid_argument ("Incompitable matrix sizes");
        }

        std::int32_t L_GEQ { 0 }, R_LEQ { 0 };

        for (std::int32_t i {}; i < l_matrix.rows; ++i)
        {
          for (std::int32_t j {}; j < l_matrix.columns; ++j)
          {
            if (l_matrix.storage_ptr[i][j] >= r_matrix.storage_ptr[i][j])
            {
              ++L_GEQ;
            }
            else
            {
              ++R_LEQ;
            }
          }
        }
        return L_GEQ >= R_LEQ;
      }
    };

  }

}

#endif
