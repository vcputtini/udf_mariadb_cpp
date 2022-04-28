cmake_minimum_required(VERSION 3.14)

project(vcputilities LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_definitions("-fPIC -Wall -Wextra -pedantic")
add_definitions("-DHAVE_DLOPEN -DUSING_MARIADB ")

include_directories("/usr/include/mysql")


add_library(vcputilities SHARED
  vcputilities.cc
  vcputilities.h
)

target_link_libraries(vcputilities PRIVATE -lpthread)
target_compile_definitions(vcputilities PRIVATE VCPUTILITIES_LIBRARY)
