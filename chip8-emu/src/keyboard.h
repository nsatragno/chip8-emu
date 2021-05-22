#pragma once

#include <cstdint>
#include <list>

class Keyboard {
 public:
  class KeyboardObserver {
   public:
    virtual void on_key_pressed(uint8_t key) = 0;
  };

  Keyboard() = default;
  virtual ~Keyboard() = default;

  virtual bool is_key_pressed(uint8_t key) const;

  void add_observer(KeyboardObserver* observer);
  void remove_observer(KeyboardObserver* observer);

 protected:
  void dispatch_key_pressed(uint8_t key);

 private:
  std::list<KeyboardObserver*> observers_;
};