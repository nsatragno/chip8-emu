# CMakeList.txt : CMake project for chip8-emu, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("chip8-emu")

# Add source to this project's executable.
add_executable (chip8-emu "chip8-emu.cpp" "chip8-emu.h")

# TODO: Add tests and install targets if needed.
