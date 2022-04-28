cmake_minimum_required(VERSION 3.14)

project(vcplocation LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_definitions("-fPIC -Wall -Wextra -pedantic")
add_definitions("-DHAVE_DLOPEN -DUSING_MARIADB ")

include_directories("/usr/include/mysql")

add_library(vcplocation SHARED
    UTM.h
    UTM.cc
    locationtools.h
    locationtools.cc
    geo_sumdist.h
    geo_sumdist.cc
    geo_distance.h
    geo_distance.cc
    geo_azimuth.h
    geo_azimuth.cc
    convertions.cc
    convertions.h
)

target_link_libraries(vcplocation PRIVATE -lpthread)
target_compile_definitions(vcplocation PRIVATE LIBLOCATION_LIBRARY)
