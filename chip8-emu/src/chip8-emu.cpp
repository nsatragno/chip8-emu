// chip8-emu.cpp : Defines the entry point for the application.
//

#include "chip8-emu.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <filesystem>
#include <iostream>

#include "src/frame_buffer.h"
#include "src/logging.h"

constexpr static int kRenderMultiplier = 16;
const static sf::Color kForegroundColor = sf::Color::Green;
const static sf::Color kBackgroundColor = sf::Color::Black;
const static std::string kRomLocation = "roms/";

namespace fs = std::filesystem;

int main() {
  std::vector<std::string> roms;
  try {
    for (const auto& file : fs::directory_iterator(kRomLocation)) {
      roms.push_back(file.path().filename().u8string());
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

  sf::Font font;
  if (!font.loadFromFile("resources/PressStart2P.ttf")) {
    logging::log(logging::Level::ERROR, "Could not open font");
    return -1;
  }

  std::vector<sf::Text> rom_labels;
  for (const auto& rom : roms) {
    sf::Text text(rom, font);
    text.setFillColor(kForegroundColor);
    rom_labels.push_back(std::move(text));
  }

  int selected_index = 0;
  sf::Text chevron(">", font);
  chevron.setFillColor(kForegroundColor);

  sf::Text title("chip8 emulator", font);
  title.setFillColor(kForegroundColor);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
        return 0;
      }

      if (event.type == sf::Event::KeyPressed) {
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

    window.clear(kBackgroundColor);

    window.draw(title);
    for (const auto& label : rom_labels) {
      window.draw(label);
    }
    window.draw(chevron);

    window.display();
  }

  return 0;
}
