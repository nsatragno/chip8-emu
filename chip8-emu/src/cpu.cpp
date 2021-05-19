#include "cpu.h"

uint8_t Cpu::peek(uint16_t address) {
	return memory[address];
}
