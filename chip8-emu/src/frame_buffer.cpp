#include "src/frame_buffer.h"

#include "src/logging.h"

#include <bitset>
#include <iostream>

bool FrameBuffer::get_pixel(uint8_t x, uint8_t y) {
  return buffer_[x % kScreenWidth].test(y % kScreenHeight);
}

void FrameBuffer::set_pixel(uint8_t x, uint8_t y, bool on) {
  buffer_[x % kScreenWidth].set(y % kScreenHeight, on);
}

void FrameBuffer::clear_screen() {
  for (size_t i = 0; i < kScreenWidth; ++i) {
    buffer_[i].reset();
  }
}

void FrameBuffer::print() {
  for (int i = 0; i < kScreenWidth; ++i) {
    std::cout << buffer_[i] << std::endl;
  }
}