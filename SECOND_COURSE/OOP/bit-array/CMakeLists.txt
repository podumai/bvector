cmake_minimum_required(VERSION 3.5)
project(BVECTOR VERSION 1.0 LANGUAGES CXX)
include_directories("headers/")

add_compile_options(
  -Werror

  -Wall
  -Wextra
  -Wpedantic

  -Wcast-align
  -Wcast-equal
  -Wconversion
  -Wctor-dctor-privacy
  -Wold-style-cast
  -Wredundant-decls
  -Wsign-conversion
  -Wsign-promo
)

if (NOT CMAKE_CXX_EXTENSIONS)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

set(SOURCE_EXE source/main.cpp)
target_compile_features(SOURCE_EXE cxx_std_20)

add_executable(MAIN ${SOURCE_EXE})
