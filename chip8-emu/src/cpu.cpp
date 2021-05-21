#include "src/cpu.h"

#include "src/logging.h"
#include "src/util.h"

Cpu::Cpu() : random_(std::make_unique<Random>()) {}

Cpu::Cpu(std::unique_ptr<Random> random) : random_(std::move(random)) {}

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
  // 5xy0 - SE Vx, Vy.
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
  // 7xkk - ADD Vx, byte.
  if (instruction >> 12 == 0x7) {
    v_[(instruction & 0xf00) >> 8] += instruction & 0x0ff;
    return true;
  }
  // 8xy0 - LD Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0) {
    v_[(instruction & 0xf00) >> 8] = v_[(instruction & 0x0f0) >> 4];
    return true;
  }
  // 8xy1 - OR Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x1) {
    v_[(instruction & 0xf00) >> 8] |= v_[(instruction & 0x0f0) >> 4];
    return true;
  }
  // 8xy2 - AND Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x2) {
    v_[(instruction & 0xf00) >> 8] &= v_[(instruction & 0x0f0) >> 4];
    return true;
  }
  // 8xy3 - XOR Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x3) {
    v_[(instruction & 0xf00) >> 8] ^= v_[(instruction & 0x0f0) >> 4];
    return true;
  }
  // 8xy4 - ADD Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x4) {
    uint16_t right = v_[(instruction & 0x0f0) >> 4];
    v_[0xf] = v_[(instruction & 0xf00) >> 8] + right > 0x00ff;
    v_[(instruction & 0xf00) >> 8] += right;
    return true;
  }
  // 8xy5 - SUB Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x5) {
    uint16_t right = v_[(instruction & 0x0f0) >> 4];
    v_[0xf] = v_[(instruction & 0xf00) >> 8] > right;
    v_[(instruction & 0xf00) >> 8] -= right;
    return true;
  }
  // 8xy6 - SHR Vx {, Vy}.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x6) {
    v_[0xf] = v_[(instruction & 0xf00) >> 8] & 1;
    v_[(instruction & 0xf00) >> 8] >>= 1;
    return true;
  }
  // 8xy7 - SUBN Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x7) {
    uint16_t right = v_[(instruction & 0x0f0) >> 4];
    v_[0xf] = right > v_[(instruction & 0xf00) >> 8];
    v_[(instruction & 0xf00) >> 8] = right - v_[(instruction & 0xf00) >> 8];
    return true;
  }
  // 8xyE - SHL Vx {, Vy}.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0xe) {
    v_[0xf] = (v_[(instruction & 0xf00) >> 8] >> 7) & 1;
    v_[(instruction & 0xf00) >> 8] <<= 1;
    return true;
  }
  // 9xy0 - SNE Vx, Vy.
  if (instruction >> 12 == 0x9 && (instruction & 0xf) == 0) {
    if (v_[(instruction & 0xf00) >> 8] != v_[(instruction & 0x0f0) >> 4]) {
      pc_ += 1;
    }
    return true;
  }
  // annn - LD I, addr.
  if (instruction >> 12 == 0xa) {
    index_ = instruction & 0xfff;
    return true;
  }
  // bnnn - JP V0, addr.
  if (instruction >> 12 == 0xb) {
    pc_ = (instruction & 0xfff) + v_[0];
    return true;
  }
  // Cxkk - RND Vx, byte
  if (instruction >> 12 == 0xc) {
    v_[(instruction & 0xf00) >> 8] = random_->rand() & (instruction & 0xff);
    return true;
  }
  // Dxyn - DRW Vx, Vy, nibble
  if (instruction >> 12 == 0xd) {
    uint8_t x = v_[(instruction & 0xf00) >> 8];
    uint8_t y = v_[(instruction & 0x0f0) >> 4];
    for (size_t i = 0; i < instruction & 0xf; ++i) {
      buffer_.paint(x, y, memory_[index_ = i]);
    }
    return true;
  }

  logging::log(logging::Level::ERROR,
               "Unknown instruction: " + tohex(instruction));
  return false;
}
