#include <gtest/gtest.h>

#include "src/cpu.h"

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
