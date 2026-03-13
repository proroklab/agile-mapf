cmake_minimum_required(VERSION 3.16)
file(GLOB SRCS "./src/*.cpp")
project(lacam)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cnpy/CMakeLists.txt")
  message(FATAL_ERROR
    "Missing submodule: third_party/cnpy\n"
    "Run:\n"
    "  git submodule update --init --recursive third_party/cnpy")
endif()

find_package(yaml-cpp QUIET)
if(NOT yaml-cpp_FOUND)
  message(FATAL_ERROR
    "Missing dependency: yaml-cpp\n"
    "Install yaml-cpp and rerun CMake.\n"
    "Examples:\n"
    "  macOS:  brew install yaml-cpp\n"
    "  Ubuntu: sudo apt-get install libyaml-cpp-dev")
endif()

find_package(Eigen3 QUIET)
if(NOT Eigen3_FOUND)
  message(FATAL_ERROR
    "Missing dependency: Eigen3\n"
    "Install Eigen3 and rerun CMake.\n"
    "Examples:\n"
    "  macOS:  brew install eigen\n"
    "  Ubuntu: sudo apt-get install libeigen3-dev")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Keep the vendored cnpy buildable on modern CMake without patching the submodule.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(../third_party/cnpy ./cnpy)
unset(CMAKE_POLICY_VERSION_MINIMUM)

add_library(${PROJECT_NAME} STATIC ${SRCS})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
if(MAPFX_ENABLE_NATIVE_OPTIMIZATION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${PROJECT_NAME} PRIVATE -mtune=native -march=native)
  else()
    message(WARNING
      "MAPFX_ENABLE_NATIVE_OPTIMIZATION is enabled, but native tuning flags "
      "are not configured for compiler ${CMAKE_CXX_COMPILER_ID}.")
  endif()
endif()
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE ${YAML_CPP_LIBRARIES} cnpy)
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)
target_include_directories(${PROJECT_NAME} INTERFACE ./include ${YAML_CPP_INCLUDE_DIR})
