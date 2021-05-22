#include "src/keyboard.h"

#include "src/logging.h"

bool Keyboard::is_key_pressed(uint8_t key) const {
  return false;
}

void Keyboard::add_observer(KeyboardObserver* observer) {
  for (const auto* existing : observers_) {
    if (existing == observer) {
      logging::log(logging::Level::WARN,
                   "Attempting to add existing observer");
      return;
    }
  }
  observers_.push_back(observer);
}

void Keyboard::remove_observer(KeyboardObserver* observer) {
  for (auto it = observers_.begin(); it != observers_.end(); it++) {
    if (*it == observer) {
      observers_.erase(it);
      return;
    }
  }
  logging::log(logging::Level::WARN,
               "Attempting to remove observer that was not added");
}

void Keyboard::dispatch_key_pressed(uint8_t key) { 
  for (auto* observer : observers_) {
    observer->on_key_pressed(key);
  }
}