#include "src/cpu.h"

#include <fstream>

#include "src/font_set.h"
#include "src/logging.h"
#include "src/util.h"

Cpu::Cpu(Random* random, Keyboard* keyboard)
    : random_(random),
      keyboard_(keyboard),
      buffer_(std::make_unique<FrameBuffer>()) {
  keyboard_->add_observer(this);
  for (int i = 0; i < kFontSet.size(); ++i) {
    memory_[i] = kFontSet[i];
  }
}

Cpu::~Cpu() {
  keyboard_->remove_observer(this);
}

uint8_t Cpu::peek(uint16_t address) const {
  return memory_[address];
}

void Cpu::set_memory(uint16_t address, uint8_t byte) {
  if (address > kMaxMemory) {
    logging::log(logging::Level::ERROR,
                 "Attempted to set memory exceeding " + kMaxMemory);
    return;
  }
  memory_[address] = byte;
}

bool Cpu::execute(uint16_t instruction) {
  if (waiting_for_key_press_) {
    logging::log(
        logging::Level::ERROR,
        "Attempted to execute an instruction while waiting for a key press");
    return false;
  }

  // 00e0 - CLS.
  if (instruction == 0x00e0) {
    buffer_->clear_screen();
    return true;
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
    pc_ = (instruction & 0xfff) - 2;
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
    pc_ = (instruction & 0xfff) - 2;
    return true;
  }
  // 3xkk - SE Vx, byte.
  if (instruction >> 12 == 0x3) {
    if (v_[(instruction & 0xf00) >> 8] == (instruction & 0x0ff)) {
      pc_ += 2;
    }
    return true;
  }
  // 4xkk - SNE Vx, byte.
  if (instruction >> 12 == 0x4) {
    if (v_[(instruction & 0xf00) >> 8] != (instruction & 0x0ff)) {
      pc_ += 2;
    }
    return true;
  }
  // 5xy0 - SE Vx, Vy.
  if (instruction >> 12 == 0x5 && (instruction & 0xf) == 0) {
    if (v_[(instruction & 0xf00) >> 8] == v_[(instruction & 0x0f0) >> 4]) {
      pc_ += 2;
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
    uint16_t left = v_[(instruction & 0xf00) >> 8];
    v_[(instruction & 0xf00) >> 8] += right;
    v_[0xf] = left + right > 0x00ff;
    return true;
  }
  // 8xy5 - SUB Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x5) {
    uint16_t right = v_[(instruction & 0x0f0) >> 4];
    bool no_borrow = v_[(instruction & 0xf00) >> 8] > right;
    v_[(instruction & 0xf00) >> 8] -= right;
    v_[0xf] = no_borrow;
    return true;
  }
  // 8xy6 - SHR Vx {, Vy}.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x6) {
    bool last_bit = v_[(instruction & 0xf00) >> 8] & 1;
    v_[(instruction & 0xf00) >> 8] >>= 1;
    v_[0xf] = last_bit;
    return true;
  }
  // 8xy7 - SUBN Vx, Vy.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0x7) {
    uint16_t right = v_[(instruction & 0x0f0) >> 4];
    bool no_borrow = right > v_[(instruction & 0xf00) >> 8];
    v_[(instruction & 0xf00) >> 8] = right - v_[(instruction & 0xf00) >> 8];
    v_[0xf] = no_borrow;
    return true;
  }
  // 8xyE - SHL Vx {, Vy}.
  if (instruction >> 12 == 0x8 && (instruction & 0xf) == 0xe) {
    bool first_bit = (v_[(instruction & 0xf00) >> 8] >> 7) & 1;
    v_[(instruction & 0xf00) >> 8] <<= 1;
    v_[0xf] = first_bit;
    return true;
  }
  // 9xy0 - SNE Vx, Vy.
  if (instruction >> 12 == 0x9 && (instruction & 0xf) == 0) {
    if (v_[(instruction & 0xf00) >> 8] != v_[(instruction & 0x0f0) >> 4]) {
      pc_ += 2;
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
    pc_ = (instruction & 0xfff) + v_[0] - 2;
    return true;
  }
  // Cxkk - RND Vx, byte.
  if (instruction >> 12 == 0xc) {
    v_[(instruction & 0xf00) >> 8] = random_->rand() & (instruction & 0xff);
    return true;
  }
  // Dxyn - DRW Vx, Vy, nibble.
  if (instruction >> 12 == 0xd) {
    uint8_t x = v_[(instruction & 0xf00) >> 8];
    uint8_t y = v_[(instruction & 0x0f0) >> 4];
    bool erased = false;
    for (size_t i = 0; i < (instruction & 0xf); ++i) {
      if (buffer_->paint(x, y + i, memory_[index_ + i])) {
        erased = true;
      }
    }
    v_[0xf] = erased;
    return true;
  }
  // Ex9E - SKP Vx.
  if (instruction >> 12 == 0xe && (instruction & 0xff) == 0x9e) {
    if (keyboard_->is_key_pressed(v_[(instruction & 0xf00) >> 8])) {
      pc_ += 2;
    }
    return true;
  }
  // ExA1 - SKNP Vx.
  if (instruction >> 12 == 0xe && (instruction & 0xff) == 0xa1) {
    if (!keyboard_->is_key_pressed(v_[(instruction & 0xf00) >> 8])) {
      pc_ += 2;
    }
    return true;
  }
  // Fx07 - LD Vx, DT.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x07) {
    v_[(instruction & 0xf00) >> 8] = delay_;
    return true;
  }
  // Fx0A - LD Vx, K.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x0a) {
    key_store_register_ = (instruction & 0xf00) >> 8;
    waiting_for_key_press_ = true;
    return true;
  }
  // Fx15 - LD DT, Vx.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x15) {
    delay_ = v_[(instruction & 0xf00) >> 8];
    return true;
  }
  // Fx18 - LD ST, Vx.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x18) {
    sound_ = v_[(instruction & 0xf00) >> 8];
    return true;
  }
  // Fx1E - ADD I, Vx.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x1e) {
    index_ += v_[(instruction & 0xf00) >> 8];
    return true;
  }
  // Fx29 - LD F, Vx.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x29) {
    index_ += v_[(instruction & 0xf00) >> 8] * 5;
    return true;
  }
  // Fx33 - LD B, Vx.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x33) {
    uint8_t value = v_[(instruction & 0xf00) >> 8];
    memory_[index_] = value / 100;
    memory_[index_ + 1] = (value / 10) % 10;
    memory_[index_ + 2] = value % 10;
    return true;
  }
  // Fx55 - LD [I], Vx.
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x55) {
    uint8_t registers = (instruction & 0xf00) >> 8;
    for (uint8_t reg = 0; reg <= registers; ++reg) {
      memory_[index_++] = v_[reg];
    }
    return true;
  }
  // Fx65 - LD Vx, [I].
  if (instruction >> 12 == 0xf && (instruction & 0xff) == 0x65) {
    uint8_t registers = (instruction & 0xf00) >> 8;
    for (uint8_t reg = 0; reg <= registers; ++reg) {
      v_[reg] = memory_[index_++];
    }
    return true;
  }

  logging::log(logging::Level::ERROR,
               "Unknown instruction: " + tohex(instruction));
  return false;
}

bool Cpu::step() {
  if (waiting_for_key_press_) {
    return true;
  }
  bool result = execute((static_cast<uint16_t>(memory_[pc_] << 8) | memory_[pc_ + 1]));
  if (result) {
    pc_ += 2;
  }
  return result;
}

void Cpu::update_timers() {
  if (sound_ > 0) {
    --sound_;
  }
  if (delay_ > 0) {
    --delay_;
  }
}

bool Cpu::load(const std::string& path) {
  std::ifstream file(path, std::ifstream::binary);
  if (!file) {
    logging::log(logging::Level::ERROR, "Could not open file " + path);
    return false;
  }
  file.seekg(0, file.end);
  int length = file.tellg();
  file.seekg(0, file.beg);

  const int kMaxRead = kMaxMemory - kMinAddressableMemory;
  if (length > kMaxRead) {
    logging::log(logging::Level::WARN,
                 "File " + path + " exceeds maximum size, ignoring last bytes");
  }
  file.read((char*)(memory_) + kMinAddressableMemory, kMaxMemory);
  logging::log(logging::Level::INFO, "File " + path + " loaded successfully");
  pc_ = kMinAddressableMemory;
  return true;
}

void Cpu::on_key_pressed(uint8_t key) {
  if (!waiting_for_key_press_) {
    return;
  }
  v_[key_store_register_] = key;
  waiting_for_key_press_ = false;
}
