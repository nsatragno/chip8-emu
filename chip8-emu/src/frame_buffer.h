#include <vector>

// A 64 x 32 pixel display framebuffer.
class FrameBuffer {
 public:
  static constexpr unsigned int kScreenWidth = 64;
  static constexpr unsigned int kScreenHeight = 32;

  // Returns the pixel at coordinates |x|, |y|, wrapping the screen if
  // necessary.
  bool get_pixel(uint8_t x, uint8_t y);

  // Sets the pixel at coordinates |x|, |y|, wrapping the screen if necessary.
  void set_pixel(uint8_t x, uint8_t y, bool on);

 private:
  uint32_t buffer[64] = {{0}};
};