#include <gtest/gtest.h>

#include "src/cpu.h"

TEST(Cpu, Initialization) {
	Cpu cpu;
	for (uint16_t i = 0; i < Cpu::kMaxMemory; ++i) {
		ASSERT_EQ(0, cpu.peek(i));
	}
}
