cmake_minimum_required(VERSION 3.24)
project(CBMCSMT)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/cbmc)


add_executable(CBMCTest src/CBMC_smt/main.cpp)

#probably some target_link_library(CBMCTest cprover goto .... etc.)