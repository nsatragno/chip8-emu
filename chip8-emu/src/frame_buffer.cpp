#include "src/frame_buffer.h"

#include "src/constants.h"
#include "src/logging.h"

#include <bitset>
#include <iostream>

bool FrameBuffer::paint(uint8_t x, uint8_t y, uint8_t line) {
  std::bitset<8> bits(line);
  bool erased = false;
  for (size_t i = 0; i < bits.size(); ++i) {
    bool existing = get_pixel(x + i, y);
    bool bit = bits[bits.size() - 1 - i];
    if (existing && bit) {
      erased = true;
    }
    set_pixel(x + i, y, existing ^ bit);
  }
  return erased;
}

bool FrameBuffer::get_pixel(uint8_t x, uint8_t y) const {
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

void FrameBuffer::draw(sf::RenderWindow* window) const {
  sf::RectangleShape pixel;
  pixel.setFillColor(kForegroundColor);
  pixel.setSize(sf::Vector2f(kRenderMultiplier, kRenderMultiplier));

  for (int x = 0; x < kScreenWidth; ++x) {
    for (int y = 0; y < kScreenHeight; ++y) {
      if (get_pixel(x, y)) {
        pixel.setPosition(x * kRenderMultiplier, y * kRenderMultiplier);
        window->draw(pixel);
      }
    }
  }
}

void FrameBuffer::print() {
  for (int i = 0; i < kScreenWidth; ++i) {
    std::cout << buffer_[i] << std::endl;
  }
}