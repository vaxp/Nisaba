#include <cstdarg>
#include <cstdio>
#include <nisaba/layout/core/config.hpp>
#include <nisaba/layout/core/node.hpp>

namespace nisaba::layout {

namespace {

int defaultConsoleLogger(
    const Config*,
    const Node*,
    LogLevel,
    const char* format,
    va_list args) {
  return vfprintf(stderr, format, args);
}

} // namespace

Config::Config(LoggerCallback logger) noexcept
    : logger_{logger ? logger : defaultConsoleLogger} {}

bool configUpdateInvalidatesLayout(
    const Config& oldConfig,
    const Config& newConfig) noexcept {
  return oldConfig.getPointScaleFactor() != newConfig.getPointScaleFactor() ||
         oldConfig.useWebDefaults() != newConfig.useWebDefaults();
}

void Config::setPointScaleFactor(float factor) noexcept {
  pointScaleFactor_ = factor;
  version_++;
}

void Config::log(
    const Node* node,
    LogLevel level,
    const char* format,
    ...) const {
  if (!logger_) return;
  va_list args;
  va_start(args, format);
  logger_(this, node, level, format, args);
  va_end(args);
}

Node* Config::cloneNode(
    const Node* node,
    const Node* owner,
    size_t childIndex) const {
  if (cloneNode_) {
    return cloneNode_(node, owner, childIndex);
  }
  return nullptr;
}

static const Config kGlobalDefaultConfig{};

const Config& Config::getDefault() noexcept {
  return kGlobalDefaultConfig;
}

} // namespace nisaba::layout
