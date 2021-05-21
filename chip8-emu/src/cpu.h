#pragma once

#include <cstdint>
#include <memory>

#include "src/frame_buffer.h"
#include "src/keyboard.h"
#include "src/random.h"

// A CHIP-8 complete CPU.
class Cpu {
 public:
  // The position of memory from which user program data can start.
  static constexpr unsigned int kMinAddressableMemory = 0x200;

  // The highest valid memory position.
  static constexpr unsigned int kMaxMemory = 0xfff;

  // The size of the program stack.
  static constexpr unsigned int kStackSize = 32;

  // |random| and |keyboard| must outlive this instance.
  Cpu(Random* random, Keyboard* keyboard);

  // Returns the contents of memory at |address|.
  uint8_t peek(uint16_t address) const;

  // Sets the memory position |address| to |byte|.
  void set_memory(uint16_t address, uint8_t byte);

  // Executes |instruction| without incrementing the program counter. Returns
  // true if the machine was able to execute the instruction successfully, false
  // otherwise.
  bool execute(uint16_t instruction);

  uint16_t pc() const { return pc_; }

  uint16_t v(uint8_t index) const { return v_[index]; }

  uint16_t index() const { return index_; }

  FrameBuffer const * frame_buffer() const { return buffer_.get(); }

 private:
  uint8_t v_[16] = {{0}};
  uint16_t index_ = 0;
  const std::unique_ptr<FrameBuffer> buffer_;
  uint16_t stack_[kStackSize];
  uint8_t sp_ = 0;
  uint8_t delay_ = 0;
  uint8_t sound_ = 0;
  uint16_t pc_ = 0;

  uint8_t memory_[kMaxMemory + 1] = {{0}};

  Random* random_;
  Keyboard* keyboard_;
};
