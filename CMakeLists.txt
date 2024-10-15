cmake_minimum_required(VERSION 3.5)
project(BVECTOR VERSION 1.0 LANGUAGES CXX)
include_directories("headers/")

add_compile_options(
  -Werror

  -Wall
  -Wextra
  -Wpedantic

  -Wcast-align
  -Wold-style-cast
  -Wredundant-decls
  -Wsign-conversion
  -Wsign-promo

  -std=c++20
)

if (NOT CMAKE_CXX_EXTENSIONS)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

set(SOURCE_EXE source/main.cpp)

add_executable(MAIN ${SOURCE_EXE})
