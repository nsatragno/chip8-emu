#include <gtest/gtest.h>

#include "src/frame_buffer.h"

class FrameBufferTest : public testing::Test {
 protected:
  FrameBuffer frame_buffer;
};

TEST_F(FrameBufferTest, Initialization) {
  for (int i = 0; i < FrameBuffer::kScreenWidth; ++i) {
    for (int j = 0; j < FrameBuffer::kScreenHeight; ++j) {
      EXPECT_FALSE(frame_buffer.get_pixel(i, j));
    }
  }
}

TEST_F(FrameBufferTest, SetPixels) {
  frame_buffer.set_pixel(10, 20, true);
  for (int i = 0; i < FrameBuffer::kScreenWidth; ++i) {
    for (int j = 0; j < FrameBuffer::kScreenHeight; ++j) {
      if (i == 10 && j == 20) {
        EXPECT_TRUE(frame_buffer.get_pixel(i, j));
      } else {
        EXPECT_FALSE(frame_buffer.get_pixel(i, j));
      }
    }
  }

  frame_buffer.set_pixel(10, 20, false);
  frame_buffer.set_pixel(10, 21, true);
  for (int i = 0; i < FrameBuffer::kScreenWidth; ++i) {
    for (int j = 0; j < FrameBuffer::kScreenHeight; ++j) {
      if (i == 10 && j == 21) {
        EXPECT_TRUE(frame_buffer.get_pixel(i, j));
      } else {
        EXPECT_FALSE(frame_buffer.get_pixel(i, j));
      }
    }
  }
}

TEST_F(FrameBufferTest, SetPixelsWraparound) {
  // Wraparound for set.
  frame_buffer.set_pixel(FrameBuffer::kScreenWidth + 3,
                         FrameBuffer::kScreenHeight + 5, true);
  for (int i = 0; i < FrameBuffer::kScreenWidth; ++i) {
    for (int j = 0; j < FrameBuffer::kScreenHeight; ++j) {
      if (i == 3 && j == 5) {
        EXPECT_TRUE(frame_buffer.get_pixel(i, j));
      } else {
        EXPECT_FALSE(frame_buffer.get_pixel(i, j));
      }
    }
  }

  // Wraparound for get.
  for (int i = 0; i < FrameBuffer::kScreenWidth; ++i) {
    for (int j = 0; j < FrameBuffer::kScreenHeight; ++j) {
      if (i == 3 && j == 5) {
        EXPECT_TRUE(frame_buffer.get_pixel(i + FrameBuffer::kScreenWidth,
                                           j + FrameBuffer::kScreenHeight));
      } else {
        EXPECT_FALSE(frame_buffer.get_pixel(i + FrameBuffer::kScreenWidth,
                                            j + FrameBuffer::kScreenHeight));
      }
    }
  }
}