#include <cstdio>
#include <cstdlib>
#include <nisaba/layout/debug/assert.hpp>

namespace nisaba::layout {

[[noreturn]] void fatal_error(const char* message) {
  std::fprintf(stderr, "[Nisaba Layout Fatal Error]: %s\n", message ? message : "Unknown fatal error");
  std::abort();
}

void assert_fatal(bool condition, const char* message) {
  if (!condition) {
    fatal_error(message);
  }
}

void assert_with_node(const Node*, bool condition, const char* message) {
  if (!condition) {
    fatal_error(message);
  }
}

void assert_with_config(const Config*, bool condition, const char* message) {
  if (!condition) {
    fatal_error(message);
  }
}

} // namespace nisaba::layout
