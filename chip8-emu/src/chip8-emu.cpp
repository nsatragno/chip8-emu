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
  for (size_t i = 0; i < roms.size();  ++i) {
    sf::Text text(roms[i], font);
    text.setPosition(100, 125 + i * 50);
    text.setFillColor(kForegroundColor);
    rom_labels.push_back(std::move(text));
  }

  size_t selected_index = 0;
  sf::Text chevron(">", font);
  chevron.setFillColor(kForegroundColor);

  sf::Text title("chip8 emulator", font);
  title.setPosition(300, 50);
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
          selected_index -= 1;
        }
        if (event.key.code == sf::Keyboard::Up) {
          selected_index += 1;
        }
        selected_index %= rom_labels.size();
      }
    }
    chevron.setPosition(75, 125 + selected_index * 50);

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
