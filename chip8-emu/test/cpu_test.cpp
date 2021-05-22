#include <gtest/gtest.h>

#include "src/cpu.h"

class RandomMock : public Random {
 public:
  explicit RandomMock(std::vector<int> numbers)
      : numbers_(std::move(numbers)) {}

  ~RandomMock() override = default;

  int rand() override { return numbers_.at(index_++ % numbers_.size()); };

 private:
  std::vector<int> numbers_;
  size_t index_ = 0;
};

class KeyboardMock : public Keyboard {
 public:
  KeyboardMock() : Keyboard() {}
  ~KeyboardMock() override = default;

  bool is_key_pressed(uint8_t key) const override {
    if (key >= 0xf) {
      return false;
    }
    return keys_[key];
  }

  void set_key_pressed(uint8_t key, bool value) {
    keys_[key] = value;
    if (value) {
      Keyboard::dispatch_key_pressed(key);
    }
  }

 private:
  bool keys_[0xf] = {{false}};
};

class CpuTest : public testing::Test {
 protected:
  CpuTest() {
    random_mock_ = std::make_unique<RandomMock>(std::vector<int>{0xaf, 0x11, 0x30});
    keyboard_mock_ = std::make_unique<KeyboardMock>();
    cpu_ = std::make_unique<Cpu>(random_mock_.get(), keyboard_mock_.get());
  }

  std::unique_ptr<RandomMock> random_mock_;
  std::unique_ptr<KeyboardMock> keyboard_mock_;
  std::unique_ptr<Cpu> cpu_;
};

TEST_F(CpuTest, Initialization) {
  for (uint16_t i = Cpu::kMinAddressableMemory; i <= Cpu::kMaxMemory; ++i) {
    EXPECT_EQ(0, cpu_->peek(i));
  }
}

TEST_F(CpuTest, BadInstruction) {
  ASSERT_FALSE(cpu_->execute(0x9001));
}

TEST_F(CpuTest, SysInstruction) {
  for (uint16_t i = 0; i <= 0x0fff; ++i) {
    if (i == 0x00e0 || i == 0x00ee) {
      continue;
    }
    ASSERT_TRUE(cpu_->execute(i));
  }
}

TEST_F(CpuTest, JmpInstruction) {
  for (uint16_t i = 0x1000, pc = 0; i <= 0x1fff; ++i, ++pc) {
    ASSERT_TRUE(cpu_->execute(i));
    EXPECT_EQ(pc, cpu_->pc());
  }
}

TEST_F(CpuTest, CallAndRetInstructions) {
  ASSERT_TRUE(cpu_->execute(0x2100));
  EXPECT_EQ(0x100, cpu_->pc());
  ASSERT_TRUE(cpu_->execute(0x2200));
  EXPECT_EQ(0x200, cpu_->pc());
  ASSERT_TRUE(cpu_->execute(0x2300));
  EXPECT_EQ(0x300, cpu_->pc());

  ASSERT_TRUE(cpu_->execute(0x00ee));
  EXPECT_EQ(0x200, cpu_->pc());
  ASSERT_TRUE(cpu_->execute(0x00ee));
  EXPECT_EQ(0x100, cpu_->pc());
  ASSERT_TRUE(cpu_->execute(0x00ee));
  EXPECT_EQ(0x000, cpu_->pc());
}

TEST_F(CpuTest, CallOverflow) {
  for (size_t i = 0; i < Cpu::kStackSize; ++i) {
    ASSERT_TRUE(cpu_->execute(0x2100));
  }
  ASSERT_FALSE(cpu_->execute(0x2100));
}

TEST_F(CpuTest, RetUnderflow) {
  ASSERT_TRUE(cpu_->execute(0x2100));
  ASSERT_TRUE(cpu_->execute(0x00ee));
  ASSERT_FALSE(cpu_->execute(0x00ee));
}

TEST_F(CpuTest, SkipInstructionIfEqual) {
  EXPECT_EQ(0, cpu_->pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu_->execute(0x6090));

  // Skip if register 0 contains the value 90.
  ASSERT_TRUE(cpu_->execute(0x3090));
  EXPECT_EQ(1, cpu_->pc());

  // Skip if register 0 contains the value ff.
  ASSERT_TRUE(cpu_->execute(0x30ff));
  EXPECT_EQ(1, cpu_->pc());

  // Load into register f the value ff.
  ASSERT_TRUE(cpu_->execute(0x6fff));

  // Skip if register f contains the value ff.
  ASSERT_TRUE(cpu_->execute(0x3fff));
  EXPECT_EQ(2, cpu_->pc());

  // Skip if register f contains the value 0.
  ASSERT_TRUE(cpu_->execute(0x3f00));
  EXPECT_EQ(2, cpu_->pc());

  // Skip if register a contains the value ff.
  ASSERT_TRUE(cpu_->execute(0x3aff));
  EXPECT_EQ(2, cpu_->pc());
}

TEST_F(CpuTest, SkipInstructionIfNotEqual) {
  EXPECT_EQ(0, cpu_->pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu_->execute(0x6090));

  // Skip if register 0 does not contain the value 90.
  ASSERT_TRUE(cpu_->execute(0x4090));
  EXPECT_EQ(0, cpu_->pc());

  // Skip if register 0 does not contain the value ff.
  ASSERT_TRUE(cpu_->execute(0x40ff));
  EXPECT_EQ(1, cpu_->pc());

  // Load into register f the value ff.
  ASSERT_TRUE(cpu_->execute(0x6fff));

  // Skip if register f does not contain the value ff.
  ASSERT_TRUE(cpu_->execute(0x4fff));
  EXPECT_EQ(1, cpu_->pc());

  // Skip if register f does not contain the value 0.
  ASSERT_TRUE(cpu_->execute(0x4f00));
  EXPECT_EQ(2, cpu_->pc());

  // Skip if register a does not contain the value ff.
  ASSERT_TRUE(cpu_->execute(0x4aff));
  EXPECT_EQ(3, cpu_->pc());
}

TEST_F(CpuTest, SkipInstructionIfEqualsRegister) {
  EXPECT_EQ(0, cpu_->pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu_->execute(0x6090));

  // Load into register f the value 90.
  ASSERT_TRUE(cpu_->execute(0x6f90));

  // Skip if registers 0 and f are equal.
  ASSERT_TRUE(cpu_->execute(0x50f0));
  EXPECT_EQ(1, cpu_->pc());

  // Skip if registers 0 and e are equal.
  ASSERT_TRUE(cpu_->execute(0x50e0));
  EXPECT_EQ(1, cpu_->pc());
}

TEST_F(CpuTest, Add) {
  // Add 0x30 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7030));
  EXPECT_EQ(0x30, cpu_->v(0));
  EXPECT_EQ(0, cpu_->v(0xf));

  // Add 0x03 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7003));
  EXPECT_EQ(0x33, cpu_->v(0));
  EXPECT_EQ(0, cpu_->v(0xf));

  // Add 0xff to the register e.
  ASSERT_TRUE(cpu_->execute(0x7eff));
  EXPECT_EQ(0xff, cpu_->v(0xe));

  // Add 0x02 to the register e.
  ASSERT_TRUE(cpu_->execute(0x7e02));
  EXPECT_EQ(0x01, cpu_->v(0xe));
  EXPECT_EQ(0, cpu_->v(0xf));
}

TEST_F(CpuTest, LoadFromRegister) {
  // Add 0x30 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7030));
  EXPECT_EQ(0x30, cpu_->v(0));
  EXPECT_EQ(0, cpu_->v(0x1));

  // Set register 1 to register 0.
  ASSERT_TRUE(cpu_->execute(0x8100));
  EXPECT_EQ(0x30, cpu_->v(0));
  EXPECT_EQ(0x30, cpu_->v(0x1));
}

TEST_F(CpuTest, Or) {
  // Add 0x34 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7034));
  EXPECT_EQ(0x34, cpu_->v(0));

  // Add 0x33 to the register 1.
  ASSERT_TRUE(cpu_->execute(0x7133));
  EXPECT_EQ(0x33, cpu_->v(0x1));

  // Set register 1 to register 0 OR register 1.
  ASSERT_TRUE(cpu_->execute(0x8101));
  EXPECT_EQ(0x34, cpu_->v(0));
  EXPECT_EQ(0x37, cpu_->v(0x1));
}

TEST_F(CpuTest, And) {
  // Add 0x34 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7034));
  EXPECT_EQ(0x34, cpu_->v(0));

  // Add 0x33 to the register 1.
  ASSERT_TRUE(cpu_->execute(0x7133));
  EXPECT_EQ(0x33, cpu_->v(0x1));

  // Set register 1 to register 0 AND register 1.
  ASSERT_TRUE(cpu_->execute(0x8102));
  EXPECT_EQ(0x34, cpu_->v(0));
  EXPECT_EQ(0x30, cpu_->v(0x1));
}

TEST_F(CpuTest, Xor) {
  // Add 0x1F to the register 0.
  ASSERT_TRUE(cpu_->execute(0x701f));
  EXPECT_EQ(0x1f, cpu_->v(0));

  // Add 0xf0 to the register 1.
  ASSERT_TRUE(cpu_->execute(0x71f0));
  EXPECT_EQ(0xf0, cpu_->v(0x1));

  // Set register 1 to register 0 XOR register 1.
  ASSERT_TRUE(cpu_->execute(0x8103));
  EXPECT_EQ(0x1f, cpu_->v(0));
  EXPECT_EQ(0xef, cpu_->v(0x1));
}

TEST_F(CpuTest, MathAdd) {
  // Add 0x10 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7010));
  EXPECT_EQ(0x10, cpu_->v(0));

  // Add 0xef to the register 1.
  ASSERT_TRUE(cpu_->execute(0x71ef));
  EXPECT_EQ(0xef, cpu_->v(0x1));

  // Set register 1 to register 0 plus register 1.
  ASSERT_TRUE(cpu_->execute(0x8104));
  EXPECT_EQ(0x10, cpu_->v(0));
  EXPECT_EQ(0xff, cpu_->v(0x1));
  EXPECT_EQ(0x00, cpu_->v(0xf));

  // Set register 0 to 2.
  ASSERT_TRUE(cpu_->execute(0x6002));
  EXPECT_EQ(0x02, cpu_->v(0));

  // Set register 1 to register 0 plus register 1.
  ASSERT_TRUE(cpu_->execute(0x8104));
  EXPECT_EQ(0x02, cpu_->v(0));
  EXPECT_EQ(0x01, cpu_->v(0x1));
  EXPECT_EQ(0x01, cpu_->v(0xf));

  // Set register 1 to register 0 plus register 1.
  ASSERT_TRUE(cpu_->execute(0x8104));
  EXPECT_EQ(0x02, cpu_->v(0));
  EXPECT_EQ(0x03, cpu_->v(0x1));
  EXPECT_EQ(0x00, cpu_->v(0xf));
}

TEST_F(CpuTest, MathSub) {
  // Add 0x01 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7001));
  EXPECT_EQ(0x01, cpu_->v(0));

  // Add 0x10 to the register 1.
  ASSERT_TRUE(cpu_->execute(0x7110));
  EXPECT_EQ(0x10, cpu_->v(0x1));

  // Set register 1 to register 1 minus register 0.
  ASSERT_TRUE(cpu_->execute(0x8105));
  EXPECT_EQ(0x01, cpu_->v(0));
  EXPECT_EQ(0x0f, cpu_->v(0x1));
  EXPECT_EQ(0x01, cpu_->v(0xf));

  // Set register 0 to f.
  ASSERT_TRUE(cpu_->execute(0x600f));
  EXPECT_EQ(0x0f, cpu_->v(0));

  // Set register 1 to register 1 minus register 0.
  ASSERT_TRUE(cpu_->execute(0x8105));
  EXPECT_EQ(0x0f, cpu_->v(0));
  EXPECT_EQ(0x00, cpu_->v(0x1));
  EXPECT_EQ(0x00, cpu_->v(0xf));

  // Set register 1 to register 1 minus register 0.
  ASSERT_TRUE(cpu_->execute(0x8105));
  EXPECT_EQ(0x0f, cpu_->v(0));
  EXPECT_EQ(0xf1, cpu_->v(0x1));
  EXPECT_EQ(0x00, cpu_->v(0xf));
}

TEST_F(CpuTest, ShiftRight) {
  // Add 0x02 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7002));
  EXPECT_EQ(0x02, cpu_->v(0));
  EXPECT_EQ(0x00, cpu_->v(0xf));

  // Shift right.
  ASSERT_TRUE(cpu_->execute(0x8006));
  EXPECT_EQ(0x01, cpu_->v(0));
  EXPECT_EQ(0x00, cpu_->v(0xf));

  // Shift right.
  ASSERT_TRUE(cpu_->execute(0x8006));
  EXPECT_EQ(0x00, cpu_->v(0));
  EXPECT_EQ(0x01, cpu_->v(0xf));
}

TEST_F(CpuTest, MathSubn) {
  // Add 0x10 to the register 0.
  ASSERT_TRUE(cpu_->execute(0x7010));
  EXPECT_EQ(0x10, cpu_->v(0));

  // Add 0x01 to the register 1.
  ASSERT_TRUE(cpu_->execute(0x7101));
  EXPECT_EQ(0x01, cpu_->v(0x1));

  // Set register 1 to register 0 minus register 1.
  ASSERT_TRUE(cpu_->execute(0x8107));
  EXPECT_EQ(0x10, cpu_->v(0));
  EXPECT_EQ(0x0f, cpu_->v(0x1));
  EXPECT_EQ(0x01, cpu_->v(0xf));

  // Set register 0 to f.
  ASSERT_TRUE(cpu_->execute(0x600f));
  EXPECT_EQ(0x0f, cpu_->v(0));

  // Set register 1 to register 0 minus register 1.
  ASSERT_TRUE(cpu_->execute(0x8107));
  EXPECT_EQ(0x0f, cpu_->v(0));
  EXPECT_EQ(0x00, cpu_->v(0x1));
  EXPECT_EQ(0x00, cpu_->v(0xf));

  // Set register 0 to 0.
  ASSERT_TRUE(cpu_->execute(0x6000));

  // Set register 1 to 1.
  ASSERT_TRUE(cpu_->execute(0x6101));

  // Set register 1 to register 0 minus register 1.
  ASSERT_TRUE(cpu_->execute(0x8107));
  EXPECT_EQ(0x00, cpu_->v(0));
  EXPECT_EQ(0xff, cpu_->v(0x1));
  EXPECT_EQ(0x00, cpu_->v(0xf));
}

TEST_F(CpuTest, ShiftLeft) {
  // Add 0x7f to the register 0.
  ASSERT_TRUE(cpu_->execute(0x707f));
  EXPECT_EQ(0x7f, cpu_->v(0));
  EXPECT_EQ(0x00, cpu_->v(0x7f));

  // Shift left.
  ASSERT_TRUE(cpu_->execute(0x800e));
  EXPECT_EQ(0xfe, cpu_->v(0));
  EXPECT_EQ(0x00, cpu_->v(0xf));

  // Shift left.
  ASSERT_TRUE(cpu_->execute(0x800e));
  EXPECT_EQ(0xfc, cpu_->v(0));
  EXPECT_EQ(0x01, cpu_->v(0xf));
}

TEST_F(CpuTest, SkipInstructionIfNotEqualsRegister) {
  EXPECT_EQ(0, cpu_->pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu_->execute(0x6090));

  // Load into register f the value 90.
  ASSERT_TRUE(cpu_->execute(0x6f90));

  // Skip if registers 0 and f are not equal.
  ASSERT_TRUE(cpu_->execute(0x90f0));
  EXPECT_EQ(0, cpu_->pc());

  // Skip if registers 0 and e are not equal.
  ASSERT_TRUE(cpu_->execute(0x90e0));
  EXPECT_EQ(1, cpu_->pc());
}

TEST_F(CpuTest, LoadIndex) {
  EXPECT_EQ(0, cpu_->index());
  ASSERT_TRUE(cpu_->execute(0xa123));
  EXPECT_EQ(0x123, cpu_->index());
}

TEST_F(CpuTest, JmpV0) {
  EXPECT_EQ(0, cpu_->pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu_->execute(0x6090));

  // Jump V0 + 105.
  ASSERT_TRUE(cpu_->execute(0xb105));
  EXPECT_EQ(0x195, cpu_->pc());
}

TEST_F(CpuTest, Rnd) {
  // Get a random number masking the last 4 bits.
  ASSERT_TRUE(cpu_->execute(0xc0f0));
  EXPECT_EQ(0xa0, cpu_->v(0x0));

  // Get a random number without masking.
  ASSERT_TRUE(cpu_->execute(0xc1ff));
  EXPECT_EQ(0x11, cpu_->v(0x1));

  // Get a random number but mask everything.
  ASSERT_TRUE(cpu_->execute(0xc200));
  EXPECT_EQ(0x0, cpu_->v(0x2));
}

TEST_F(CpuTest, Paint) {
  // Set the I register to 0xfae.
  ASSERT_TRUE(cpu_->execute(0xafae));

  // Set V0 to 0x10.
  ASSERT_TRUE(cpu_->execute(0x6010));

  // Set V1 to 0x20.
  ASSERT_TRUE(cpu_->execute(0x6120));

  // Load a two-byte sprite into memory location 0xfae.
  cpu_->set_memory(0xfae, 0b10001000);
  cpu_->set_memory(0xfaf, 0b01111110);

  // Draw a two bytes tall sprite at { V0, V1 } from position I.
  ASSERT_TRUE(cpu_->execute(0xd012));

  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x10, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x11, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x12, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x13, 0x20));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x14, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x15, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x16, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x17, 0x20));

  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x10, 0x21));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x11, 0x21));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x12, 0x21));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x13, 0x21));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x14, 0x21));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x15, 0x21));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x16, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x17, 0x21));

  EXPECT_EQ(0, cpu_->v(0xf));

  // Draw the same sprite. This should erase all bits.
  ASSERT_TRUE(cpu_->execute(0xd012));

  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x10, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x11, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x12, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x13, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x14, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x15, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x16, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x17, 0x20));

  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x10, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x11, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x12, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x13, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x14, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x15, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x16, 0x21));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x17, 0x21));

  EXPECT_EQ(1, cpu_->v(0xf));

  // Draw a one byte tall sprite at { V0, V1 } from position I.
  ASSERT_TRUE(cpu_->execute(0xd011));

  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x10, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x11, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x12, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x13, 0x20));
  EXPECT_EQ(true, cpu_->frame_buffer()->get_pixel(0x14, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x15, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x16, 0x20));
  EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(0x17, 0x20));

  EXPECT_EQ(0, cpu_->v(0xf));

  // Clear the screen.
  ASSERT_TRUE(cpu_->execute(0x00e0));
  for (size_t x = 0; x < FrameBuffer::kScreenWidth; ++x) {
    for (size_t y = 0; y < FrameBuffer::kScreenHeight; ++y) {
      EXPECT_EQ(false, cpu_->frame_buffer()->get_pixel(x, y));
    }
  }
}

TEST_F(CpuTest, SkipIfKey) {
  keyboard_mock_->set_key_pressed(0xa, true);

  // Set V0 to 0x00.
  ASSERT_TRUE(cpu_->execute(0x6000));

  // Set V1 to 0x0a.
  ASSERT_TRUE(cpu_->execute(0x610a));

  // Set V2 to 0x10.
  ASSERT_TRUE(cpu_->execute(0x6210));

  // Skip if key 0 is pressed.
  ASSERT_TRUE(cpu_->execute(0xe09e));
  EXPECT_EQ(0, cpu_->pc());

  // Skip if key a is pressed.
  ASSERT_TRUE(cpu_->execute(0xe19e));
  EXPECT_EQ(1, cpu_->pc());

  // Skip if an invalid key is pressed.
  ASSERT_TRUE(cpu_->execute(0xe29e));
  EXPECT_EQ(1, cpu_->pc());
}

TEST_F(CpuTest, SkipIfNotKey) {
  keyboard_mock_->set_key_pressed(0xa, true);

  // Set V0 to 0x00.
  ASSERT_TRUE(cpu_->execute(0x6000));

  // Set V1 to 0x0a.
  ASSERT_TRUE(cpu_->execute(0x610a));

  // Set V2 to 0x10.
  ASSERT_TRUE(cpu_->execute(0x6210));

  // Skip if key 0 is pressed.
  ASSERT_TRUE(cpu_->execute(0xe0a1));
  EXPECT_EQ(1, cpu_->pc());

  // Skip if key a is pressed.
  ASSERT_TRUE(cpu_->execute(0xe1a1));
  EXPECT_EQ(1, cpu_->pc());

  // Skip if an invalid key is pressed.
  ASSERT_TRUE(cpu_->execute(0xe2a1));
  EXPECT_EQ(2, cpu_->pc());
}

TEST_F(CpuTest, LoadDelayTimer) {
  // Set V0 to 0x10.
  ASSERT_TRUE(cpu_->execute(0x6010));

  // Copy register V0 into the delay timer.
  ASSERT_TRUE(cpu_->execute(0xf015));

  // Copy the delay timer into register V1.
  ASSERT_TRUE(cpu_->execute(0xf107));
  EXPECT_EQ(0x10, cpu_->v(0x1));
}

TEST_F(CpuTest, WaitForKeyboard) {
  // Wait for a keypress and store the result on V0.
  ASSERT_TRUE(cpu_->execute(0xf00a));
  ASSERT_FALSE(cpu_->execute(0xf00a));  // Already waiting for a keypress.
  EXPECT_EQ(0, cpu_->v(0x0));
  keyboard_mock_->set_key_pressed(0xb, true);
  EXPECT_EQ(0xb, cpu_->v(0x0));
  ASSERT_TRUE(cpu_->execute(0xf00a));  // No longer waiting for a keypress.
}

TEST_F(CpuTest, LoadSound) {
  // Set Ve to 0x10.
  ASSERT_TRUE(cpu_->execute(0x6e10));

  // Set sound timer to Ve.
  ASSERT_TRUE(cpu_->execute(0xfe18));
  ASSERT_EQ(0x10, cpu_->sound());
}

TEST_F(CpuTest, AddIndex) {
  // Set Ve to 0x10.
  ASSERT_TRUE(cpu_->execute(0x6e10));

  // Add Ve.
  ASSERT_TRUE(cpu_->execute(0xfe1e));
  EXPECT_EQ(0x10, cpu_->index());

  // Add Ve.
  ASSERT_TRUE(cpu_->execute(0xfe1e));
  EXPECT_EQ(0x20, cpu_->index());
}

TEST_F(CpuTest, LoadDigit) {
  // Set Ve to 0xf.
  ASSERT_TRUE(cpu_->execute(0x6e0f));

  // Load the address for sprite f.
  ASSERT_TRUE(cpu_->execute(0xfe29));
  EXPECT_EQ(0x4b, cpu_->index());
}

TEST_F(CpuTest, LoadBcd) {
  // Set Ve to 123;
  ASSERT_TRUE(cpu_->execute(0x6e7b));

  // Set I to 123.
  ASSERT_TRUE(cpu_->execute(0xa123));

  // Load the BCD representation of 123 into 0x123, 0x124, and 0x125.
  ASSERT_TRUE(cpu_->execute(0xfe33));
  EXPECT_EQ(1, cpu_->peek(0x123));
  EXPECT_EQ(2, cpu_->peek(0x124));
  EXPECT_EQ(3, cpu_->peek(0x125));
}

TEST_F(CpuTest, StoreRegisters) {
  // Set V0 to 1;
  ASSERT_TRUE(cpu_->execute(0x6001));

  // Set V1 to 2;
  ASSERT_TRUE(cpu_->execute(0x6102));

  // Set V2 to 3;
  ASSERT_TRUE(cpu_->execute(0x6203));

  // Set V3 to 4;
  ASSERT_TRUE(cpu_->execute(0x6304));

  // Set I to 123.
  ASSERT_TRUE(cpu_->execute(0xa123));

  // Store V0 through V2 into memory address 0x123.
  ASSERT_TRUE(cpu_->execute(0xf255));
  EXPECT_EQ(1, cpu_->peek(0x123));
  EXPECT_EQ(2, cpu_->peek(0x124));
  EXPECT_EQ(3, cpu_->peek(0x125));
  EXPECT_EQ(0, cpu_->peek(0x126));
  EXPECT_EQ(0x126, cpu_->index());
}

TEST_F(CpuTest, LoadRegisters) {
  // Set I to 123.
  ASSERT_TRUE(cpu_->execute(0xa123));

  // Store 1 through 4 contiguously starting at 0x123.
  cpu_->set_memory(0x123, 1);
  cpu_->set_memory(0x124, 2);
  cpu_->set_memory(0x125, 3);
  cpu_->set_memory(0x126, 4);

  // Load V0 through V2 from 0x123.
  ASSERT_TRUE(cpu_->execute(0xf265));

  EXPECT_EQ(1, cpu_->v(0));
  EXPECT_EQ(2, cpu_->v(1));
  EXPECT_EQ(3, cpu_->v(2));
  EXPECT_EQ(0, cpu_->v(3));
  EXPECT_EQ(0x126, cpu_->index());
}
