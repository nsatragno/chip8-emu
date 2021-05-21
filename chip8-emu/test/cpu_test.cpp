#include <gtest/gtest.h>

#include "src/cpu.h"

class RandomMock : public Random {
 public:
  explicit RandomMock(std::vector<int> numbers)
      : numbers_(std::move(numbers)) {}

  int rand() override { return numbers_.at(index_++ % numbers_.size()); };

 private:
  std::vector<int> numbers_;
  size_t index_ = 0;
};

class CpuTest : public testing::Test {
 protected:
  Cpu cpu;
};

TEST_F(CpuTest, Initialization) {
  for (uint16_t i = Cpu::kMinAddressableMemory; i <= Cpu::kMaxMemory; ++i) {
    EXPECT_EQ(0, cpu.peek(i));
  }
}

TEST_F(CpuTest, BadInstruction) {
  ASSERT_FALSE(cpu.execute(0x9001));
}

TEST_F(CpuTest, SysInstruction) {
  for (uint16_t i = 0; i <= 0x0fff; ++i) {
    if (i == 0x00e0 || i == 0x00ee) {
      continue;
    }
    ASSERT_TRUE(cpu.execute(i));
  }
}

TEST_F(CpuTest, JmpInstruction) {
  for (uint16_t i = 0x1000, pc = 0; i <= 0x1fff; ++i, ++pc) {
    ASSERT_TRUE(cpu.execute(i));
    EXPECT_EQ(pc, cpu.pc());
  }
}

TEST_F(CpuTest, CallAndRetInstructions) {
  ASSERT_TRUE(cpu.execute(0x2100));
  EXPECT_EQ(0x100, cpu.pc());
  ASSERT_TRUE(cpu.execute(0x2200));
  EXPECT_EQ(0x200, cpu.pc());
  ASSERT_TRUE(cpu.execute(0x2300));
  EXPECT_EQ(0x300, cpu.pc());

  ASSERT_TRUE(cpu.execute(0x00ee));
  EXPECT_EQ(0x200, cpu.pc());
  ASSERT_TRUE(cpu.execute(0x00ee));
  EXPECT_EQ(0x100, cpu.pc());
  ASSERT_TRUE(cpu.execute(0x00ee));
  EXPECT_EQ(0x000, cpu.pc());
}

TEST_F(CpuTest, CallOverflow) {
  for (size_t i = 0; i < Cpu::kStackSize; ++i) {
    ASSERT_TRUE(cpu.execute(0x2100));
  }
  ASSERT_FALSE(cpu.execute(0x2100));
}

TEST_F(CpuTest, RetUnderflow) {
  ASSERT_TRUE(cpu.execute(0x2100));
  ASSERT_TRUE(cpu.execute(0x00ee));
  ASSERT_FALSE(cpu.execute(0x00ee));
}

TEST_F(CpuTest, SkipInstructionIfEqual) {
  EXPECT_EQ(0, cpu.pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu.execute(0x6090));

  // Skip if register 0 contains the value 90.
  ASSERT_TRUE(cpu.execute(0x3090));
  EXPECT_EQ(1, cpu.pc());

  // Skip if register 0 contains the value ff.
  ASSERT_TRUE(cpu.execute(0x30ff));
  EXPECT_EQ(1, cpu.pc());

  // Load into register f the value ff.
  ASSERT_TRUE(cpu.execute(0x6fff));

  // Skip if register f contains the value ff.
  ASSERT_TRUE(cpu.execute(0x3fff));
  EXPECT_EQ(2, cpu.pc());

  // Skip if register f contains the value 0.
  ASSERT_TRUE(cpu.execute(0x3f00));
  EXPECT_EQ(2, cpu.pc());

  // Skip if register a contains the value ff.
  ASSERT_TRUE(cpu.execute(0x3aff));
  EXPECT_EQ(2, cpu.pc());
}

TEST_F(CpuTest, SkipInstructionIfNotEqual) {
  EXPECT_EQ(0, cpu.pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu.execute(0x6090));

  // Skip if register 0 does not contain the value 90.
  ASSERT_TRUE(cpu.execute(0x4090));
  EXPECT_EQ(0, cpu.pc());

  // Skip if register 0 does not contain the value ff.
  ASSERT_TRUE(cpu.execute(0x40ff));
  EXPECT_EQ(1, cpu.pc());

  // Load into register f the value ff.
  ASSERT_TRUE(cpu.execute(0x6fff));

  // Skip if register f does not contain the value ff.
  ASSERT_TRUE(cpu.execute(0x4fff));
  EXPECT_EQ(1, cpu.pc());

  // Skip if register f does not contain the value 0.
  ASSERT_TRUE(cpu.execute(0x4f00));
  EXPECT_EQ(2, cpu.pc());

  // Skip if register a does not contain the value ff.
  ASSERT_TRUE(cpu.execute(0x4aff));
  EXPECT_EQ(3, cpu.pc());
}

TEST_F(CpuTest, SkipInstructionIfEqualsRegister) {
  EXPECT_EQ(0, cpu.pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu.execute(0x6090));

  // Load into register f the value 90.
  ASSERT_TRUE(cpu.execute(0x6f90));

  // Skip if registers 0 and f are equal.
  ASSERT_TRUE(cpu.execute(0x50f0));
  EXPECT_EQ(1, cpu.pc());

  // Skip if registers 0 and e are equal.
  ASSERT_TRUE(cpu.execute(0x50e0));
  EXPECT_EQ(1, cpu.pc());
}

TEST_F(CpuTest, Add) {
  // Add 0x30 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7030));
  EXPECT_EQ(0x30, cpu.v(0));
  EXPECT_EQ(0, cpu.v(0xf));

  // Add 0x03 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7003));
  EXPECT_EQ(0x33, cpu.v(0));
  EXPECT_EQ(0, cpu.v(0xf));

  // Add 0xff to the register e.
  ASSERT_TRUE(cpu.execute(0x7eff));
  EXPECT_EQ(0xff, cpu.v(0xe));

  // Add 0x02 to the register e.
  ASSERT_TRUE(cpu.execute(0x7e02));
  EXPECT_EQ(0x01, cpu.v(0xe));
  EXPECT_EQ(0, cpu.v(0xf));
}

TEST_F(CpuTest, LoadFromRegister) {
  // Add 0x30 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7030));
  EXPECT_EQ(0x30, cpu.v(0));
  EXPECT_EQ(0, cpu.v(0x1));

  // Set register 1 to register 0.
  ASSERT_TRUE(cpu.execute(0x8100));
  EXPECT_EQ(0x30, cpu.v(0));
  EXPECT_EQ(0x30, cpu.v(0x1));
}

TEST_F(CpuTest, Or) {
  // Add 0x34 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7034));
  EXPECT_EQ(0x34, cpu.v(0));

  // Add 0x33 to the register 1.
  ASSERT_TRUE(cpu.execute(0x7133));
  EXPECT_EQ(0x33, cpu.v(0x1));

  // Set register 1 to register 0 OR register 1.
  ASSERT_TRUE(cpu.execute(0x8101));
  EXPECT_EQ(0x34, cpu.v(0));
  EXPECT_EQ(0x37, cpu.v(0x1));
}

TEST_F(CpuTest, And) {
  // Add 0x34 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7034));
  EXPECT_EQ(0x34, cpu.v(0));

  // Add 0x33 to the register 1.
  ASSERT_TRUE(cpu.execute(0x7133));
  EXPECT_EQ(0x33, cpu.v(0x1));

  // Set register 1 to register 0 AND register 1.
  ASSERT_TRUE(cpu.execute(0x8102));
  EXPECT_EQ(0x34, cpu.v(0));
  EXPECT_EQ(0x30, cpu.v(0x1));
}

TEST_F(CpuTest, Xor) {
  // Add 0x1F to the register 0.
  ASSERT_TRUE(cpu.execute(0x701f));
  EXPECT_EQ(0x1f, cpu.v(0));

  // Add 0xf0 to the register 1.
  ASSERT_TRUE(cpu.execute(0x71f0));
  EXPECT_EQ(0xf0, cpu.v(0x1));

  // Set register 1 to register 0 XOR register 1.
  ASSERT_TRUE(cpu.execute(0x8103));
  EXPECT_EQ(0x1f, cpu.v(0));
  EXPECT_EQ(0xef, cpu.v(0x1));
}

TEST_F(CpuTest, MathAdd) {
  // Add 0x10 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7010));
  EXPECT_EQ(0x10, cpu.v(0));

  // Add 0xef to the register 1.
  ASSERT_TRUE(cpu.execute(0x71ef));
  EXPECT_EQ(0xef, cpu.v(0x1));

  // Set register 1 to register 0 plus register 1.
  ASSERT_TRUE(cpu.execute(0x8104));
  EXPECT_EQ(0x10, cpu.v(0));
  EXPECT_EQ(0xff, cpu.v(0x1));
  EXPECT_EQ(0x00, cpu.v(0xf));

  // Set register 0 to 2.
  ASSERT_TRUE(cpu.execute(0x6002));
  EXPECT_EQ(0x02, cpu.v(0));

  // Set register 1 to register 0 plus register 1.
  ASSERT_TRUE(cpu.execute(0x8104));
  EXPECT_EQ(0x02, cpu.v(0));
  EXPECT_EQ(0x01, cpu.v(0x1));
  EXPECT_EQ(0x01, cpu.v(0xf));

  // Set register 1 to register 0 plus register 1.
  ASSERT_TRUE(cpu.execute(0x8104));
  EXPECT_EQ(0x02, cpu.v(0));
  EXPECT_EQ(0x03, cpu.v(0x1));
  EXPECT_EQ(0x00, cpu.v(0xf));
}

TEST_F(CpuTest, MathSub) {
  // Add 0x01 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7001));
  EXPECT_EQ(0x01, cpu.v(0));

  // Add 0x10 to the register 1.
  ASSERT_TRUE(cpu.execute(0x7110));
  EXPECT_EQ(0x10, cpu.v(0x1));

  // Set register 1 to register 1 minus register 0.
  ASSERT_TRUE(cpu.execute(0x8105));
  EXPECT_EQ(0x01, cpu.v(0));
  EXPECT_EQ(0x0f, cpu.v(0x1));
  EXPECT_EQ(0x01, cpu.v(0xf));

  // Set register 0 to f.
  ASSERT_TRUE(cpu.execute(0x600f));
  EXPECT_EQ(0x0f, cpu.v(0));

  // Set register 1 to register 1 minus register 0.
  ASSERT_TRUE(cpu.execute(0x8105));
  EXPECT_EQ(0x0f, cpu.v(0));
  EXPECT_EQ(0x00, cpu.v(0x1));
  EXPECT_EQ(0x00, cpu.v(0xf));

  // Set register 1 to register 1 minus register 0.
  ASSERT_TRUE(cpu.execute(0x8105));
  EXPECT_EQ(0x0f, cpu.v(0));
  EXPECT_EQ(0xf1, cpu.v(0x1));
  EXPECT_EQ(0x00, cpu.v(0xf));
}

TEST_F(CpuTest, ShiftRight) {
  // Add 0x02 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7002));
  EXPECT_EQ(0x02, cpu.v(0));
  EXPECT_EQ(0x00, cpu.v(0xf));

  // Shift right.
  ASSERT_TRUE(cpu.execute(0x8006));
  EXPECT_EQ(0x01, cpu.v(0));
  EXPECT_EQ(0x00, cpu.v(0xf));

  // Shift right.
  ASSERT_TRUE(cpu.execute(0x8006));
  EXPECT_EQ(0x00, cpu.v(0));
  EXPECT_EQ(0x01, cpu.v(0xf));
}

TEST_F(CpuTest, MathSubn) {
  // Add 0x10 to the register 0.
  ASSERT_TRUE(cpu.execute(0x7010));
  EXPECT_EQ(0x10, cpu.v(0));

  // Add 0x01 to the register 1.
  ASSERT_TRUE(cpu.execute(0x7101));
  EXPECT_EQ(0x01, cpu.v(0x1));

  // Set register 1 to register 0 minus register 1.
  ASSERT_TRUE(cpu.execute(0x8107));
  EXPECT_EQ(0x10, cpu.v(0));
  EXPECT_EQ(0x0f, cpu.v(0x1));
  EXPECT_EQ(0x01, cpu.v(0xf));

  // Set register 0 to f.
  ASSERT_TRUE(cpu.execute(0x600f));
  EXPECT_EQ(0x0f, cpu.v(0));

  // Set register 1 to register 0 minus register 1.
  ASSERT_TRUE(cpu.execute(0x8107));
  EXPECT_EQ(0x0f, cpu.v(0));
  EXPECT_EQ(0x00, cpu.v(0x1));
  EXPECT_EQ(0x00, cpu.v(0xf));

  // Set register 0 to 0.
  ASSERT_TRUE(cpu.execute(0x6000));

  // Set register 1 to 1.
  ASSERT_TRUE(cpu.execute(0x6101));

  // Set register 1 to register 0 minus register 1.
  ASSERT_TRUE(cpu.execute(0x8107));
  EXPECT_EQ(0x00, cpu.v(0));
  EXPECT_EQ(0xff, cpu.v(0x1));
  EXPECT_EQ(0x00, cpu.v(0xf));
}

TEST_F(CpuTest, ShiftLeft) {
  // Add 0x7f to the register 0.
  ASSERT_TRUE(cpu.execute(0x707f));
  EXPECT_EQ(0x7f, cpu.v(0));
  EXPECT_EQ(0x00, cpu.v(0x7f));

  // Shift left.
  ASSERT_TRUE(cpu.execute(0x800e));
  EXPECT_EQ(0xfe, cpu.v(0));
  EXPECT_EQ(0x00, cpu.v(0xf));

  // Shift left.
  ASSERT_TRUE(cpu.execute(0x800e));
  EXPECT_EQ(0xfc, cpu.v(0));
  EXPECT_EQ(0x01, cpu.v(0xf));
}

TEST_F(CpuTest, SkipInstructionIfNotEqualsRegister) {
  EXPECT_EQ(0, cpu.pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu.execute(0x6090));

  // Load into register f the value 90.
  ASSERT_TRUE(cpu.execute(0x6f90));

  // Skip if registers 0 and f are not equal.
  ASSERT_TRUE(cpu.execute(0x90f0));
  EXPECT_EQ(0, cpu.pc());

  // Skip if registers 0 and e are not equal.
  ASSERT_TRUE(cpu.execute(0x90e0));
  EXPECT_EQ(1, cpu.pc());
}

TEST_F(CpuTest, LoadIndex) {
  EXPECT_EQ(0, cpu.index());
  ASSERT_TRUE(cpu.execute(0xa123));
  EXPECT_EQ(0x123, cpu.index());
}

TEST_F(CpuTest, JmpV0) {
  EXPECT_EQ(0, cpu.pc());

  // Load into register 0 the value 90.
  ASSERT_TRUE(cpu.execute(0x6090));

  // Jump V0 + 105.
  ASSERT_TRUE(cpu.execute(0xb105));
  EXPECT_EQ(0x195, cpu.pc());
}

TEST_F(CpuTest, Rnd) {
  auto random_mock = std::make_unique<RandomMock>(std::vector { 0xaf, 0x11, 0x30 });
  Cpu cpu(std::move(random_mock));

  // Get a random number masking the last 4 bits.
  cpu.execute(0xc0f0);
  EXPECT_EQ(0xa0, cpu.v(0x0));

  // Get a random number without masking.
  cpu.execute(0xc1ff);
  EXPECT_EQ(0x11, cpu.v(0x1));

  // Get a random number but mask everything.
  cpu.execute(0xc200);
  EXPECT_EQ(0x0, cpu.v(0x2));
}
