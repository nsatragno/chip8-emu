#include "src/sf_keyboard_adapter.h"

#include <array>

#include "src/logging.h"

static constexpr std::array<sf::Keyboard::Key, 16> keys = {
  sf::Keyboard::X,

  sf::Keyboard::Num1,
  sf::Keyboard::Num2,
  sf::Keyboard::Num3,

  sf::Keyboard::Q,
  sf::Keyboard::W,
  sf::Keyboard::E,

  sf::Keyboard::A,
  sf::Keyboard::S,
  sf::Keyboard::D,

  sf::Keyboard::Z,
  sf::Keyboard::C,

  sf::Keyboard::Num4,
  sf::Keyboard::R,
  sf::Keyboard::F,
  sf::Keyboard::V,
};

bool SfKeyboardAdapter::is_key_pressed(uint8_t key) const {
  if (key > 0xf) {
    logging::log(logging::Level::ERROR, "Attempted to retrieve key " + key);
    return false;
  }
  return sf::Keyboard::isKeyPressed(keys[key]);
}

void SfKeyboardAdapter::on_key_pressed(sf::Keyboard::Key key) {
  for (int i = 0; i < keys.size(); ++i) {
    if (keys[i] == key) {
      dispatch_key_pressed(i);
      break;
    }
  }
}