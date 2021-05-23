#pragma once

#include "src/keyboard.h"

#include <SFML/Window/Keyboard.hpp>

class SfKeyboardAdapter : public Keyboard {
 public:
  bool is_key_pressed(uint8_t key) const override;

  void on_key_pressed(sf::Keyboard::Key key);
};