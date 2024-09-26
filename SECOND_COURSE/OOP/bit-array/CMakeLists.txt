cmake_minimum_required (VERSION 3.5)
project (BIT-ARRAY)
include_directories ("headers/")

set (SOURCE_LIB source/bit_array.cpp)
set (SOURCE_EXE source/main.cpp)

add_library (BIT_ARRAY_LIB STATIC ${SOURCE_LIB})
add_executable (MAIN ${SOURCE_EXE})
target_link_libraries (MAIN BIT_ARRAY_LIB)
