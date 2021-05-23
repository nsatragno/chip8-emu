#pragma once

#include <cstdint>
#include <memory>

#include "src/frame_buffer.h"
#include "src/keyboard.h"
#include "src/random.h"

// A CHIP-8 complete CPU.
class Cpu : Keyboard::KeyboardObserver {
 public:
  // The position of memory from which user program data can start.
  static constexpr unsigned int kMinAddressableMemory = 0x200;

  // The highest valid memory position.
  static constexpr unsigned int kMaxMemory = 0xfff;

  // The size of the program stack.
  static constexpr unsigned int kStackSize = 32;

  // |random| and |keyboard| must outlive this instance.
  Cpu(Random* random, Keyboard* keyboard);

  virtual ~Cpu();

  // Returns the contents of memory at |address|.
  uint8_t peek(uint16_t address) const;

  // Sets the memory position |address| to |byte|.
  void set_memory(uint16_t address, uint8_t byte);

  // Executes |instruction| without incrementing the program counter. Returns
  // true if the machine was able to execute the instruction successfully, false
  // otherwise. Jump instructions set the PC to their target - 1 to allow
  // unconditionally incrementing the PC when stepping.
  bool execute(uint16_t instruction);

  // Executes the next instruction and updates the program counter.
  bool step();

  // Updates the delay and sound timers, decrementing them if necessary.
  void update_timers();

  // Attempts to load the chip 8 file |path|. Returns true if successful, false
  // otherwise.
  bool load(const std::string& path);

  uint16_t pc() const { return pc_; }

  uint16_t v(uint8_t index) const { return v_[index]; }

  uint16_t index() const { return index_; }

  uint16_t sound() const { return sound_; }

  FrameBuffer const * frame_buffer() const { return buffer_.get(); }

 protected:
  // Keyboard::KeyboardObserver:
  void on_key_pressed(uint8_t key) override;

 private:
  uint8_t v_[16] = {{0}};
  uint16_t index_ = 0;
  const std::unique_ptr<FrameBuffer> buffer_;
  uint16_t stack_[kStackSize];
  uint8_t sp_ = 0;
  uint8_t delay_ = 0;
  uint8_t sound_ = 0;
  uint16_t pc_ = kMinAddressableMemory;
  uint8_t memory_[kMaxMemory + 1] = {{0}};

  bool waiting_for_key_press_ = false;
  uint8_t key_store_register_;

  Random* random_;
  Keyboard* keyboard_;
};
