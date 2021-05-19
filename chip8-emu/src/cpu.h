#pragma once

#include <cstdint>

#include "src/frame_buffer.h"
#include "src/registers.h"

class Cpu {
public:
	static constexpr unsigned int kMaxMemory = 4069;

	uint8_t peek(uint16_t address);

private:
	Registers registers;
	FrameBuffer buffer;
	uint8_t stack[64] = { {0} };
	uint8_t delay = 0;
	uint8_t sound = 0;
	uint16_t pc = 0;

	uint8_t memory[kMaxMemory] = { {0} };
};
