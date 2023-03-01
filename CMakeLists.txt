# -- CMAKE general settings ---

cmake_minimum_required(VERSION 3.14)

cmake_policy(SET CMP0048 NEW)

project(my-emu
    VERSION 0.1
    DESCRIPTION "My Emulator"
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter")

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

# --- Set up clang-tidy ---

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "--config-file=.clang-tidy -p ./build/compile_commands.json --fix")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Main executable ---

set(MY_EMU_RUN ${PROJECT_NAME}_run)

set(MY_EMU_LIB ${PROJECT_NAME}_lib)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp)

add_executable(${MY_EMU_RUN} ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp)
add_library(${MY_EMU_LIB} ${sources})

target_include_directories(${MY_EMU_RUN} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${MY_EMU_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${MY_EMU_RUN} ${MY_EMU_LIB})

# --- Tests ---

if(CMAKE_BUILD_TYPE STREQUAL "Test")

  # --- Set up googletest ---

  enable_testing()

  include(CTest)

  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  FetchContent_MakeAvailable(googletest)

  # --- Set up tests ---

  set(MY_EMU_TEST ${PROJECT_NAME}_test)

  file(GLOB tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

  add_executable(${MY_EMU_TEST} ${tests})

  target_include_directories(${MY_EMU_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

  target_link_libraries(${MY_EMU_TEST} gtest_main gtest ${MY_EMU_LIB})

  add_test(NAME ${MY_EMU_TEST} COMMAND ${MY_EMU_TEST})

  # --- Set up code coverage ---

  SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -coverage")
  SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -coverage")

  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)
  include(CodeCoverage)
  setup_target_for_coverage_lcov(
    NAME coverage
    EXECUTABLE ${MY_EMU_TEST}
    DEPENDENCIES ${MY_EMU_TEST}
    EXCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/tests/*
  )
endif()

# --- Debug ---

if(CMAKE_BUILD_TYPE STREQUAL "Debug")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")

endif()

# --- Release ---

if(CMAKE_BUILD_TYPE STREQUAL "Release")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

endif()
