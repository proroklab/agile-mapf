cmake_minimum_required(VERSION 3.16)
project(lacam-project CXX)

option(MAPFX_ENABLE_NATIVE_OPTIMIZATION
  "Enable compiler native CPU tuning flags for local builds"
  OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Choose the build type." FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  Debug Release RelWithDebInfo MinSizeRel)

find_package(Threads REQUIRED)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/argparse/include/argparse/argparse.hpp")
  message(FATAL_ERROR
    "Missing submodule: third_party/argparse\n"
    "Run:\n"
    "  git submodule update --init --recursive third_party/argparse")
endif()

# This repository vendors argparse only for local builds, not packaging.
set(ARGPARSE_INSTALL OFF CACHE BOOL "" FORCE)
set(ARGPARSE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ARGPARSE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(./lacam)
add_subdirectory(./third_party/argparse)

add_executable(main main.cpp)
target_compile_features(main PUBLIC cxx_std_17)
target_link_libraries(main lacam argparse Threads::Threads)

# test
enable_testing()
option(LACAM_BUILD_OPTIONAL_TESTS "Build tests that require extra local assets" OFF)
file(GLOB TEST_FILES "./tests/test_*.cpp")
foreach(file ${TEST_FILES})
  string(REGEX MATCH "test\_[^\.]+" name "${file}")
  if(name STREQUAL "test_action_model" AND NOT LACAM_BUILD_OPTIONAL_TESTS)
    continue()
  endif()
  add_executable(${name} ${file})
  target_link_libraries(${name} lacam)
  add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
  if(name STREQUAL "test_action_model")
    set_tests_properties(${name} PROPERTIES
      LABELS "optional"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
  endif()
endforeach()
