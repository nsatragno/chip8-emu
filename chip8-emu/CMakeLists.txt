﻿# CMakeList.txt : CMake project for chip8-emu, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.14)

project ("chip8-emu")
set(CMAKE_CXX_STANDARD 17)

# Add source to this project's executable.
add_executable(chip8-emu "src/chip8-emu.cpp" "src/chip8-emu.h" "src/cpu.h" "src/cpu.cpp" "src/logging.h" "src/logging.cpp" "src/util.h" "src/util.cpp" "src/frame_buffer.cpp" "src/random.h" "src/random.cpp" "src/keyboard.h"  "src/keyboard.cpp" "src/sf_keyboard_adapter.h" "src/sf_keyboard_adapter.cpp" "src/constants.h" "src/font_set.h")
set(BASEPATH "${CMAKE_SOURCE_DIR}")
include_directories("${BASEPATH}")
include_directories("${BASEPATH}/lib/sfml/include")

install(TARGETS chip8-emu DESTINATION bin)

# SFML.
add_subdirectory("lib/sfml/")
find_package(SFML 2.5.1
  COMPONENTS 
    system window graphics audio REQUIRED)
target_link_libraries(chip8-emu sfml-window sfml-graphics)

file(COPY "${BASEPATH}/roms" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${BASEPATH}/resources" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(TARGET chip8-emu POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/lib/sfml/lib/sfml-window-d-2.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/lib/sfml/lib/sfml-system-d-2.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/lib/sfml/lib/sfml-graphics-d-2.dll"
        ${CMAKE_CURRENT_BINARY_DIR})

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
 chip8-tests
 "test/cpu_test.cpp"
 "src/cpu.h" "src/cpu.cpp" "src/logging.h" "src/logging.cpp" "src/util.h" "src/util.cpp" "src/frame_buffer.cpp" "test/frame_buffer_test.cpp" "src/random.h" "src/random.cpp" "src/keyboard.h"  "src/keyboard.cpp" "src/sf_keyboard_adapter.h" "src/sf_keyboard_adapter.cpp" "src/constants.h" "src/font_set.h")
target_link_libraries(
  chip8-tests
  gtest_main
  sfml-window
  sfml-graphics
)

add_custom_command(TARGET chip8-tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/lib/sfml/lib/sfml-window-d-2.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/lib/sfml/lib/sfml-system-d-2.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/lib/sfml/lib/sfml-graphics-d-2.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/gmock_maind.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/gmockd.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/gtest_maind.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/gtestd.dll"
        ${CMAKE_CURRENT_BINARY_DIR})

include(GoogleTest)
gtest_discover_tests(chip8-tests)
