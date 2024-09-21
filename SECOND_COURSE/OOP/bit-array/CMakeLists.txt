cmake_minimum_required (VERSION 3.5)

project (bit-array
         DESCRIPTION "Bit array library (C++)"
         LANGUAGES CXX)

include_directories ("headers/")

set (SOURCE_EXE source/main.cpp)
set (BIT_ARR_LIB source/bit_array.cpp)

add_library (bit_array STATIC ${BIT_ARR_LIB})
add_executable (main ${SOURCE_EXE})

target_link_libraries (main bit_array)
