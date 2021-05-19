# CMakeList.txt : CMake project for chip8-emu, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.14)

project ("chip8-emu")
set(CMAKE_CXX_STANDARD 17)

# Add source to this project's executable.
add_executable(chip8-emu "chip8-emu.cpp" "chip8-emu.h")

# Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/eb6e9273dcf9c6535abb45306afe558aa961e3c3.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  tests
  test/test.cpp
)
target_link_libraries(
  tests
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
