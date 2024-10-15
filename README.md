# BVECTOR
> Dynamic array for storing and maintaining bit manipulations.
## :face_in_clouds: IN PROGRESS...
### TODO:
* [ ] Unit tests;
* [ ] Iterator class for traversing the collection;
* [ ] Full operation support;
* [x] Base interface;
## BVECTOR INTERFACE
* **Constructors**
  - bvector() ***noexcept***;
    > constructs the empty object
  - bvector(*size_type* **init_bits**, *size_type* **filling_val = 0**); [ *may throw std::bad_alloc exception* ]
    > consructs the object with init_bits size and optional filling value for the first eight bytes
  - bvector(const bvector& **other**); [ *may throw std::bad_alloc exception* ]
    > constructs the object by copying all the data from recieved object
  - bvector(bvector&& **rvalue**) ***noexcept***;
    > constructs the object by moving all the data from recieved object and then resets it

* **Destructor**
  > Deallocates the underlying storage.
* **Capacity**
  - *size_type* size() ***const noexcept***;
    > returns the number of stored elements
  - *size_type* capacity() ***const noexcept***;
    > returns the number of bytes that container is owning
  - *constexpr size_type* max_size() ***const noexcept***;
    > returns the maximum number of bits that container can hold
  - *bool* empty() ***const noexcept***;
    > checks whether the container is empty
  - *void* reserve(); [ may throw std::bad_alloc exception ]
    > reserves the storage
  - *void* shrink_to_fit(); [ may throw std::bad_alloc exception ]
    > reduces memory usage by freeing unused memory
 * **Element access**
   - *bool* at(*size_type* **bit_pos**); [ may throw std::out_of_range exception ]
     > returns the bit in requested position with bounce checking
   - *bool* operator [] (*size_type* **bit_pos**) ***const noexcept***;
     > returns the bit in requested position
   - *bool* front() ***const***; [ may throw std::out_of_range exception ]
     > returns the first bit
   - *bool* back() ***const***; [ may throw std::out_of_range exception ]
     > returns the last bit
   - *pointer* data() ***const noexcept***;
     > returns the raw data pointer
