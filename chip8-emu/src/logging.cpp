#include "src/logging.h"

#include <ctime>
#include <iomanip>
#include <iostream>

namespace logging {

std::string printLevel(Level level) {
  switch (level) {
    case Level::INFO:
      return "INFO";
    case Level::WARN:
      return "WARN";
    case Level::ERROR:
      return "ERROR";
  }
}

void log(Level level, const std::string& text) {
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);

  std::cout << "[" << printLevel(level) << "] " << std::put_time(&tm, "%T")
            << ": " << text << std::endl;
}

}  // namespace logging