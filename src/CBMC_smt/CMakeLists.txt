cmake_minimum_required(VERSION 3.24)
project(CBMCSMT)

include(FetchContent)


set(CMAKE_CXX_STANDARD 20)


#add_subdirectory(lib/cbmc)


FetchContent_Declare(cbmc
        GIT_REPOSITORY git@github.com:jparsert/cbmc.git
        GIT_TAG develop
        )
FetchContent_MakeAvailable(cbmc)



set(sources
        create_training_data.cpp
        anti_unification.cpp
        parser.cpp
        printing_utils.cpp
        problem.cpp
        smt2_frontend.cpp
        sygus_problem.cpp
        term_position.cpp)

set(headers
        create_training_data.h
        anti_unification.h
        parser.h
        printing_utils.h
        problem.h
        smt2_frontend.h
        sygus_problem.h
        term_position.h
        )

add_executable(cbmcsmt main.cpp ${sources} ${headers})


target_include_directories(cbmcsmt
        PRIVATE ${cbmc_SOURCE_DIR}/src
        )

target_link_libraries(cbmcsmt
        PRIVATE util
        PRIVATE solvers)

#probably some target_link_library(CBMCTest cprover goto .... etc.)