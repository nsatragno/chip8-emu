#include <string>

namespace logging {

enum class Level { INFO, WARN, ERROR };

void log(Level level, const std::string& text);

}  // namespace logging