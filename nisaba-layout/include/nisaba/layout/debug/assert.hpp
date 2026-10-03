#pragma once

#include <nisaba/layout/core/types.hpp>

namespace nisaba::layout {

class Node;
class Config;

/**
 * Handles unrecoverable fatal layout errors by logging and terminating.
 */
[[noreturn]] void fatal_error(const char* message);

/**
 * Validates invariant condition and triggers fatal error handler if condition is false.
 */
void assert_fatal(bool condition, const char* message);

/**
 * Validates invariant condition in the context of a specific node.
 */
void assert_with_node(
    const Node* node,
    bool condition,
    const char* message);

/**
 * Validates invariant condition in the context of a specific engine configuration.
 */
void assert_with_config(
    const Config* config,
    bool condition,
    const char* message);

// Forwarding inline functions for backward compatibility
[[noreturn]] inline void fatalWithMessage(const char* message) {
  fatal_error(message);
}

inline void assertFatal(bool condition, const char* message) {
  assert_fatal(condition, message);
}

inline void assertFatalWithNode(
    const Node* node,
    bool condition,
    const char* message) {
  assert_with_node(node, condition, message);
}

inline void assertFatalWithConfig(
    const Config* config,
    bool condition,
    const char* message) {
  assert_with_config(config, condition, message);
}

} // namespace nisaba::layout
