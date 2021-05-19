#include <gtest/gtest.h>

#include "src/frame_buffer.h"

class FrameBufferTest : public testing::Test {
protected:
	FrameBuffer frame_buffer;
};

TEST_F(FrameBufferTest, Initialization) {
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 32; ++j) {
			EXPECT_FALSE(frame_buffer.get_pixel(i, j));
		}
	}
}