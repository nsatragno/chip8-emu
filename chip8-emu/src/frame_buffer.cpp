#include "src/frame_buffer.h"

#include "src/logging.h"

bool FrameBuffer::get_pixel(uint8_t x, uint8_t y) {
	return (buffer[x % 64] >> (y % 32)) & 1;
}