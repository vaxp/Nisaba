#pragma once

#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/flex_direction.hpp>
#include <nisaba/layout/core/float_optional.hpp>
#include <nisaba/layout/style/value.hpp>

namespace nisaba::layout {

struct FlexDefaults {
  static constexpr float DefaultGrow = 0.0f;
  static constexpr float DefaultShrink = 0.0f;
  static constexpr float WebDefaultShrink = 1.0f;
};

} // namespace nisaba::layout
