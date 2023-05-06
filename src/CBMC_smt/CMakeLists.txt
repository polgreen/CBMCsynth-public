cmake_minimum_required(VERSION 3.24)
project(CBMCSMT)

include(FetchContent)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wno-return-type")

#add_subdirectory(lib/cbmc)


FetchContent_Declare(cbmc
        GIT_REPOSITORY git@github.com:diffblue/cbmc.git
        GIT_TAG develop
        )
FetchContent_MakeAvailable(cbmc)

FetchContent_Declare(uuid_v4
        GIT_REPOSITORY git@github.com:crashoz/uuid_v4.git
        GIT_TAG v1.0.0
        )
FetchContent_MakeAvailable(uuid_v4)


set(sources
        create_training_data.cpp
        anti_unification.cpp
        parser.cpp
        printing_utils.cpp
        problem.cpp
        smt2_frontend.cpp
        sygus_problem.cpp
        term_position.cpp
        unification.cpp
        expr2sygus.cpp util.cpp
        subterm_heuristics.cpp)

set(headers
        create_training_data.h
        anti_unification.h
        parser.h
        printing_utils.h
        problem.h
        smt2_frontend.h
        sygus_problem.h
        term_position.h
        unification.h
        expr2sygus.h util.h
        constants.h
        progressbar.hpp
        subterm_heuristics.h)

add_executable(cbmcsmt main.cpp ${sources} ${headers})


target_include_directories(cbmcsmt
        PRIVATE ${cbmc_SOURCE_DIR}/src
        PRIVATE ${uuid_v4_SOURCE_DIR}
        )

target_link_libraries(cbmcsmt
        PRIVATE util
        PRIVATE solvers)

#probably some target_link_library(CBMCTest cprover goto .... etc.)