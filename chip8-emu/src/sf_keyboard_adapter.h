#pragma once

#include "src/keyboard.h"

class SfKeyboardAdapter : public Keyboard {
 public:
  bool is_key_pressed(uint8_t key) const override;
};