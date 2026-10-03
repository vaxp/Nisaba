#pragma once

#include <cstdarg>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/types.hpp>

namespace nisaba::layout {

class Node;
class Config;

/**
 * Emits a formatted log entry at the specified log level.
 */
void log(LogLevel level, const char* format, ...) noexcept;

/**
 * Emits a formatted log entry associated with a specific node.
 */
void log(
    const Node* node,
    LogLevel level,
    const char* format,
    ...) noexcept;

/**
 * Emits a formatted log entry associated with a specific configuration.
 */
void log(
    const Config* config,
    LogLevel level,
    const char* format,
    ...) noexcept;

/**
 * Returns the default system logger implementation for the current platform.
 */
LoggerCallback getDefaultLogger();

} // namespace nisaba::layout
