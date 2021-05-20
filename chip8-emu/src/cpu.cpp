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
  // 3xkk - SE Vx, byte.
  if (instruction >> 12 == 0x3) {
    if (v_[(instruction & 0xf00) >> 8] == (instruction & 0x0ff)) {
      pc_ += 1;
    }
    return true;
  }
  // 4xkk - SNE Vx, byte.
  if (instruction >> 12 == 0x4) {
    if (v_[(instruction & 0xf00) >> 8] != (instruction & 0x0ff)) {
      pc_ += 1;
    }
    return true;
  }
  // 5xy0 - SE Vx, Vy
  if (instruction >> 12 == 0x5 && (instruction & 0xf) == 0) {
    if (v_[(instruction & 0xf00) >> 8] == v_[(instruction & 0x0f0) >> 4]) {
      pc_ += 1;
    }
    return true;
  }
  // 6xkk - LD Vx, byte.
  if (instruction >> 12 == 0x6) {
    v_[(instruction & 0xf00) >> 8] = instruction & 0x0ff;
    return true;
  }

  logging::log(logging::Level::ERROR,
               "Unknown instruction: " + tohex(instruction));
  return false;
}
