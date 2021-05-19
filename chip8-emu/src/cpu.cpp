#include "src/cpu.h"

#include <iostream>

#include "src/logging.h"
#include "src/util.h"

uint8_t Cpu::peek(uint16_t address) const {
	return memory_[address];
}

bool Cpu::execute(uint16_t instruction) {
	// 00e0 - CLS.
	if (instruction == 0x00e0) {
		// TODO: not implemented.
		return false;
	}
	// 00ee - RET.
	if (instruction == 0x00ee) {
		if (sp_ <= 0) {
			logging::log(logging::Level::ERROR, "Stack underflow");
			return false;
		}
		--sp_;
		pc_ = stack_[sp_];
		return true;
	}
	// 0nnn - SYS addr.
	// This instruction is ignored.
	if (instruction >> 12 == 0x0) {
		return true;
	}
	// 1nnn - JP addr.
	if (instruction >> 12 == 0x1) {
		pc_ = instruction & 0xfff;
		return true;
	}
	// 2nnn - CALL addr.
	if (instruction >> 12 == 0x2) {
		if (sp_ >= kStackSize) {
			logging::log(logging::Level::ERROR, "Stack overflow");
			return false;
		}
		stack_[sp_] = pc_;
		++sp_;
		pc_ = instruction & 0xfff;
		return true;
	}

	logging::log(logging::Level::ERROR, "Unknown instruction: " + tohex(instruction));
	return false;
}
