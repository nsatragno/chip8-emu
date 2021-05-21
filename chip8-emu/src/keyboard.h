#pragma once

#include <cstdint>

class Keyboard {
 public:
  Keyboard() = default;
  virtual ~Keyboard() = default;

  virtual bool is_key_pressed(uint8_t key) const;
};