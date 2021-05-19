#include <vector>

// A 64 x 32 pixel display framebuffer.
class FrameBuffer {
public:
	// Returns the pixel at coordinates |x|, |y|.
	bool get_pixel(uint8_t x, uint8_t y);

private:
	uint32_t buffer[64] = { {0} };
};