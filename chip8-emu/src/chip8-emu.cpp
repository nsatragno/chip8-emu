// chip8-emu.cpp : Defines the entry point for the application.
//

#include "chip8-emu.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <filesystem>
#include <iostream>

#include "src/constants.h"
#include "src/cpu.h"
#include "src/frame_buffer.h"
#include "src/logging.h"
#include "src/sf_keyboard_adapter.h"

namespace fs = std::filesystem;

int main() {
  std::vector<std::filesystem::path> roms;
  try {
    for (const auto& file : fs::directory_iterator(kRomLocation)) {
      roms.push_back(file.path());
      logging::log(logging::Level::INFO,
                   "Found file " + file.path().generic_u8string());
    }
  } catch (const std::exception&) {
    logging::log(logging::Level::ERROR,
                 "Could not open ROM folder " + kRomLocation);
  }

  sf::RenderWindow window(
      sf::VideoMode(FrameBuffer::kScreenWidth * kRenderMultiplier,
                    FrameBuffer::kScreenHeight * kRenderMultiplier),
      "chip8 emu", sf::Style::Close);
  window.setFramerateLimit(60);

  sf::Font font;
  if (!font.loadFromFile("resources/PressStart2P.ttf")) {
    logging::log(logging::Level::ERROR, "Could not open font");
    return -1;
  }

  std::vector<sf::Text> rom_labels;
  for (const auto& rom : roms) {
    sf::Text text(rom.filename().u8string(), font);
    text.setFillColor(kForegroundColor);
    rom_labels.push_back(std::move(text));
  }

  int selected_index = 0;
  sf::Text chevron(">", font);
  chevron.setFillColor(kForegroundColor);

  sf::Text title("chip8 emulator", font);
  title.setFillColor(kForegroundColor);

  std::unique_ptr<Random> random = std::make_unique<Random>();
  std::unique_ptr<SfKeyboardAdapter> keyboard =
      std::make_unique<SfKeyboardAdapter>();
  std::unique_ptr<Cpu> cpu;

  bool in_menu = true;
  while (window.isOpen()) {
    window.clear(kBackgroundColor);

    if (in_menu) {
      sf::Event event;
      while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
          window.close();
          return 0;
        }

        if (event.type == sf::Event::KeyPressed) {
          if (event.key.code == sf::Keyboard::Enter) {
            in_menu = false;
            cpu = std::make_unique<Cpu>(random.get(), keyboard.get());
            if (!cpu->load(roms[selected_index].u8string())) {
              return -1;
            }
            goto loop;
          }
          if (event.key.code == sf::Keyboard::Down) {
            ++selected_index;
          }
          if (event.key.code == sf::Keyboard::Up) {
            --selected_index;
          }
          if (selected_index < 0) {
            selected_index = rom_labels.size() - 1;
          }
          selected_index %= rom_labels.size();
        }
      }
      chevron.setPosition(75, 125 + selected_index * 50);
      if (selected_index > 4) {
        chevron.move(0, -(selected_index - 4) * 50);
      }

      title.setPosition(300, 50);
      if (selected_index > 4) {
        title.move(0, -(selected_index - 4) * 50);
      }

      for (size_t i = 0; i < rom_labels.size(); ++i) {
        rom_labels[i].setPosition(100, 125 + i * 50);
        if (selected_index > 4) {
          rom_labels[i].move(0, -(selected_index - 4) * 50);
        }
      }

      window.draw(title);
      for (const auto& label : rom_labels) {
        window.draw(label);
      }
      window.draw(chevron);

    } else {
      sf::Event event;
      while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
          window.close();
          return 0;
        }
        if (event.type == sf::Event::KeyPressed) {
          if (event.key.code == sf::Keyboard::Escape) {
            cpu.reset();
            in_menu = true;
            goto loop;
          }
          keyboard->on_key_pressed(event.key.code);
        }
      }
      for (int i = 0; i < 10; ++i) {
        if (!cpu->step()) {
          return -1;
        }
      }
      cpu->update_timers();
      cpu->frame_buffer()->draw(&window);
    }

    window.display();
  loop:;
  }

  return 0;
}
