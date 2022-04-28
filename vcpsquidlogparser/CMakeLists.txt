cmake_minimum_required(VERSION 3.14)

project(vcpsquidlogparser LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_definitions("-fPIC -Wall -Wextra -pedantic")
add_definitions("-DHAVE_DLOPEN -DUSING_MARIADB ")

# These macros make the 'parser' functions print the values of the retrieved
# fields to stdout.
# In this case the outputs will be recorded in the mariadb.log file.
# Important: Only activate these macros for testing. In production environment
# they must be disabled.

#add_definitions("-DDEBUG_PARSER_SQUID")
#add_definitions("-DDEBUG_PARSER_COMMON")
#add_definitions("-DDEBUG_PARSER_COMBINED")
#add_definitions("-DDEBUG_PARSER_REFERRER")
#add_definitions("-DDEBUG_PARSER_USERAGENT")


include_directories("/usr/include/mysql")

add_library(vcpsquidlogparser SHARED
  squidlogparser_udf.cc
  squidlogparser_udf.h
  squidlogparser.cc
  squidlogparser.h
)

# Required to compile the SquidLogParser object.
find_package(tinyxml2 REQUIRED)

target_link_libraries(vcpsquidlogparser PRIVATE -ltinyxml2 -lboost_regex -lpthread)
target_compile_definitions(vcpsquidlogparser PRIVATE VCPSQUIDLOGPARSER_LIBRARY)
