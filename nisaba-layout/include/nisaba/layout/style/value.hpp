#pragma once

#include <cmath>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/float_optional.hpp>
#include <nisaba/layout/core/comparison.hpp>
#include <nisaba/layout/core/types.hpp>

namespace nisaba::layout {

struct LayoutValue {
  float value{Undefined};
  Unit unit{Unit::Undefined};

  constexpr bool operator==(const LayoutValue& other) const noexcept {
    if (unit != other.unit) return false;
    if (unit == Unit::Undefined || unit == Unit::Auto) return true;
    return value == other.value || (isUndefined(value) && isUndefined(other.value));
  }
};
using StyleValue = LayoutValue;

/**
 * Sovereign CSS length-percentage value representation.
 */
class StyleLength {
 public:
  constexpr StyleLength() noexcept = default;
  constexpr StyleLength(float val, Unit u) noexcept : value_(val), unit_(u) {}

  [[nodiscard]] static constexpr StyleLength points(float val) noexcept {
    return std::isnan(val) || std::isinf(val) ? undefined() : StyleLength{val, Unit::Point};
  }
  [[nodiscard]] static constexpr StyleLength percent(float val) noexcept {
    return std::isnan(val) || std::isinf(val) ? undefined() : StyleLength{val, Unit::Percent};
  }
  [[nodiscard]] static constexpr StyleLength ofAuto() noexcept {
    return StyleLength{0.0f, Unit::Auto};
  }
  [[nodiscard]] static constexpr StyleLength auto_() noexcept {
    return ofAuto();
  }
  [[nodiscard]] static constexpr StyleLength undefined() noexcept {
    return StyleLength{Undefined, Unit::Undefined};
  }

  [[nodiscard]] constexpr bool isPoints() const noexcept { return unit_ == Unit::Point; }
  [[nodiscard]] constexpr bool isPercent() const noexcept { return unit_ == Unit::Percent; }
  [[nodiscard]] constexpr bool isAuto() const noexcept { return unit_ == Unit::Auto; }
  [[nodiscard]] constexpr bool isUndefined() const noexcept { return unit_ == Unit::Undefined; }
  [[nodiscard]] constexpr bool isDefined() const noexcept { return unit_ != Unit::Undefined; }

  [[nodiscard]] constexpr Unit unit() const noexcept { return unit_; }
  [[nodiscard]] constexpr FloatOptional value() const noexcept { return FloatOptional{value_}; }

  [[nodiscard]] constexpr FloatOptional resolve(float baseLength) const noexcept {
    if (unit_ == Unit::Point) return FloatOptional{value_};
    if (unit_ == Unit::Percent) return FloatOptional{value_ * baseLength * 0.01f};
    return FloatOptional{};
  }

  explicit constexpr operator LayoutValue() const noexcept {
    return LayoutValue{value_, unit_};
  }

  [[nodiscard]] constexpr bool operator==(const StyleLength& rhs) const noexcept {
    if (unit_ != rhs.unit_) return false;
    if (unit_ == Unit::Undefined || unit_ == Unit::Auto) return true;
    return value_ == rhs.value_ || (nisaba::layout::isUndefined(value_) && nisaba::layout::isUndefined(rhs.value_));
  }

 private:
  float value_{Undefined};
  Unit unit_{Unit::Undefined};
};

/**
 * Sovereign CSS size value representation (points, percent, auto, stretch, keywords).
 */
class StyleSizeLength {
 public:
  constexpr StyleSizeLength() noexcept = default;
  constexpr StyleSizeLength(float val, Unit u) noexcept : value_(val), unit_(u) {}

  [[nodiscard]] static constexpr StyleSizeLength points(float val) noexcept {
    return std::isnan(val) || std::isinf(val) ? undefined() : StyleSizeLength{val, Unit::Point};
  }
  [[nodiscard]] static constexpr StyleSizeLength percent(float val) noexcept {
    return std::isnan(val) || std::isinf(val) ? undefined() : StyleSizeLength{val, Unit::Percent};
  }
  [[nodiscard]] static constexpr StyleSizeLength ofAuto() noexcept {
    return StyleSizeLength{0.0f, Unit::Auto};
  }
  [[nodiscard]] static constexpr StyleSizeLength auto_() noexcept {
    return ofAuto();
  }
  [[nodiscard]] static constexpr StyleSizeLength undefined() noexcept {
    return StyleSizeLength{Undefined, Unit::Undefined};
  }
  [[nodiscard]] static constexpr StyleSizeLength ofMaxContent() noexcept {
    return StyleSizeLength{0.0f, Unit::MaxContent};
  }
  [[nodiscard]] static constexpr StyleSizeLength ofFitContent() noexcept {
    return StyleSizeLength{0.0f, Unit::FitContent};
  }
  [[nodiscard]] static constexpr StyleSizeLength stretch(float fraction = 1.0f) noexcept {
    return std::isnan(fraction) || std::isinf(fraction) ? undefined() : StyleSizeLength{fraction, Unit::Stretch};
  }
  [[nodiscard]] static constexpr StyleSizeLength ofStretch() noexcept {
    return stretch(1.0f);
  }

  [[nodiscard]] constexpr bool isPoints() const noexcept { return unit_ == Unit::Point; }
  [[nodiscard]] constexpr bool isPercent() const noexcept { return unit_ == Unit::Percent; }
  [[nodiscard]] constexpr bool isAuto() const noexcept { return unit_ == Unit::Auto; }
  [[nodiscard]] constexpr bool isUndefined() const noexcept { return unit_ == Unit::Undefined; }
  [[nodiscard]] constexpr bool isDefined() const noexcept { return unit_ != Unit::Undefined; }
  [[nodiscard]] constexpr bool isMaxContent() const noexcept { return unit_ == Unit::MaxContent; }
  [[nodiscard]] constexpr bool isFitContent() const noexcept { return unit_ == Unit::FitContent; }
  [[nodiscard]] constexpr bool isStretch() const noexcept { return unit_ == Unit::Stretch; }

  [[nodiscard]] constexpr Unit unit() const noexcept { return unit_; }
  [[nodiscard]] constexpr FloatOptional value() const noexcept { return FloatOptional{value_}; }

  [[nodiscard]] constexpr FloatOptional resolve(float baseLength) const noexcept {
    if (unit_ == Unit::Point) return FloatOptional{value_};
    if (unit_ == Unit::Percent) return FloatOptional{value_ * baseLength * 0.01f};
    return FloatOptional{};
  }

  explicit constexpr operator LayoutValue() const noexcept {
    return LayoutValue{value_, unit_};
  }

  [[nodiscard]] constexpr bool operator==(const StyleSizeLength& rhs) const noexcept {
    if (unit_ != rhs.unit_) return false;
    if (unit_ == Unit::Undefined || unit_ == Unit::Auto) return true;
    return value_ == rhs.value_ || (nisaba::layout::isUndefined(value_) && nisaba::layout::isUndefined(rhs.value_));
  }

 private:
  float value_{Undefined};
  Unit unit_{Unit::Undefined};
};

} // namespace nisaba::layout
