#include "src/frame_buffer.h"

#include "src/logging.h"

bool FrameBuffer::get_pixel(uint8_t x, uint8_t y) {
  return (buffer[x % 64] >> (y % 32)) & 1;
}

void FrameBuffer::set_pixel(uint8_t x, uint8_t y, bool on) {
  if (on) {
    buffer[x % 64] |= 1 << y;
  } else {
    buffer[x % 64] &= ~1u << y;
  }
}