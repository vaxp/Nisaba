#include <cstdarg>
#include <cstdio>
#include <nisaba/layout/core/config.hpp>
#include <nisaba/layout/core/node.hpp>
#include <nisaba/layout/debug/log.hpp>

namespace nisaba::layout {

static int consoleLogger(
    const Config*,
    const Node*,
    LogLevel,
    const char* format,
    va_list args) {
  return std::vfprintf(stderr, format, args);
}

LoggerCallback getDefaultLogger() {
  return consoleLogger;
}

void log(LogLevel level, const char* format, ...) noexcept {
  (void)level;
  char buffer[512];
  va_list args;
  va_start(args, format);
  const int written = std::vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  if (written > 0) {
    std::fputs(buffer, stderr);
  }
}

void log(const Node* node, LogLevel level, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  if (node && node->getConfig()) {
    node->getConfig()->log(node, level, format, args);
  } else {
    consoleLogger(nullptr, node, level, format, args);
  }
  va_end(args);
}

void log(const Config* config, LogLevel level, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  if (config) {
    config->log(nullptr, level, format, args);
  } else {
    consoleLogger(config, nullptr, level, format, args);
  }
  va_end(args);
}

} // namespace nisaba::layout
