#pragma once

#include <vector>
#include <bitset>

// A 64 x 32 pixel display framebuffer.
class FrameBuffer {
 public:
  static constexpr unsigned int kScreenWidth = 64;
  static constexpr unsigned int kScreenHeight = 32;

  FrameBuffer() = default;
  virtual ~FrameBuffer() = default;

  // "Paints" a single sprite |line| at position |x|, |y|. Returns true if
  // paiting caused a screen bit to be flipped off.
  bool paint(uint8_t x, uint8_t y, uint8_t line);

  // Returns the pixel at coordinates |x|, |y|, wrapping the screen if
  // necessary.
  bool get_pixel(uint8_t x, uint8_t y) const;

  // Sets the pixel at coordinates |x|, |y|, wrapping the screen if necessary.
  void set_pixel(uint8_t x, uint8_t y, bool on);

  // Clears the framebuffer.
  void clear_screen();

  void print();

 private:
  std::bitset<kScreenHeight> buffer_[kScreenWidth];
};