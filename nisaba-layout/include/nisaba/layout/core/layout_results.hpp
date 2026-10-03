#pragma once

#include <array>
#include <cstdint>

#include <nisaba/layout/core/cached_measurement.hpp>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/float_optional.hpp>

namespace nisaba::layout {

/**
 * Sovereign layout geometry outputs and measurements for a node.
 */
struct LayoutResults {
  static constexpr size_t MaxCachedMeasurements = 8;

  uint32_t computedFlexBasisGeneration{0};
  FloatOptional computedFlexBasis{};
  FloatOptional computedAutoMinMainSize{};

  uint32_t generationCount{0};
  uint32_t configVersion{0};
  Direction lastOwnerDirection{Direction::Inherit};

  uint32_t nextCachedMeasurementsIndex{0};
  std::array<CachedMeasurement, MaxCachedMeasurements> cachedMeasurements{};
  CachedMeasurement cachedLayout{};

  // Direct physical geometry accessors
  [[nodiscard]] constexpr float left() const noexcept { return position_[0]; }
  [[nodiscard]] constexpr float top() const noexcept { return position_[1]; }
  [[nodiscard]] constexpr float right() const noexcept { return position_[2]; }
  [[nodiscard]] constexpr float bottom() const noexcept { return position_[3]; }
  [[nodiscard]] constexpr float width() const noexcept { return dimensions_[0]; }
  [[nodiscard]] constexpr float height() const noexcept { return dimensions_[1]; }

  [[nodiscard]] constexpr Direction direction() const noexcept { return direction_; }
  constexpr void setDirection(Direction dir) noexcept { direction_ = dir; }

  [[nodiscard]] constexpr bool hadOverflow() const noexcept { return hadOverflow_; }
  constexpr void setHadOverflow(bool overflow) noexcept { hadOverflow_ = overflow; }

  [[nodiscard]] constexpr float dimension(Dimension axis) const noexcept {
    const size_t idx = static_cast<size_t>(axis);
    return idx < dimensions_.size() ? dimensions_[idx] : 0.0f;
  }
  constexpr void setDimension(Dimension axis, float dim) noexcept {
    const size_t idx = static_cast<size_t>(axis);
    if (idx < dimensions_.size()) dimensions_[idx] = dim;
  }

  [[nodiscard]] constexpr float measuredDimension(Dimension axis) const noexcept {
    const size_t idx = static_cast<size_t>(axis);
    return idx < measuredDimensions_.size() ? measuredDimensions_[idx] : 0.0f;
  }
  constexpr void setMeasuredDimension(Dimension axis, float dim) noexcept {
    const size_t idx = static_cast<size_t>(axis);
    if (idx < measuredDimensions_.size()) measuredDimensions_[idx] = dim;
  }

  [[nodiscard]] constexpr float rawDimension(Dimension axis) const noexcept {
    const size_t idx = static_cast<size_t>(axis);
    return idx < rawDimensions_.size() ? rawDimensions_[idx] : 0.0f;
  }
  constexpr void setRawDimension(Dimension axis, float dim) noexcept {
    const size_t idx = static_cast<size_t>(axis);
    if (idx < rawDimensions_.size()) rawDimensions_[idx] = dim;
  }

  [[nodiscard]] constexpr float position(PhysicalEdge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < position_.size() ? position_[idx] : 0.0f;
  }
  constexpr void setPosition(PhysicalEdge edge, float pos) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < position_.size()) position_[idx] = pos;
  }

  [[nodiscard]] constexpr float margin(PhysicalEdge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < margin_.size() ? margin_[idx] : 0.0f;
  }
  constexpr void setMargin(PhysicalEdge edge, float val) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < margin_.size()) margin_[idx] = val;
  }

  [[nodiscard]] constexpr float border(PhysicalEdge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < border_.size() ? border_[idx] : 0.0f;
  }
  constexpr void setBorder(PhysicalEdge edge, float val) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < border_.size()) border_[idx] = val;
  }

  [[nodiscard]] constexpr float padding(PhysicalEdge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < padding_.size() ? padding_[idx] : 0.0f;
  }
  constexpr void setPadding(PhysicalEdge edge, float val) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < padding_.size()) padding_[idx] = val;
  }

  bool operator==(const LayoutResults& rhs) const noexcept;

 private:
  friend class Node;
  std::array<float, 4> position_{0.0f, 0.0f, 0.0f, 0.0f};
  std::array<float, 2> dimensions_{0.0f, 0.0f};
  std::array<float, 2> measuredDimensions_{0.0f, 0.0f};
  std::array<float, 2> rawDimensions_{0.0f, 0.0f};
  std::array<float, 4> margin_{0.0f, 0.0f, 0.0f, 0.0f};
  std::array<float, 4> border_{0.0f, 0.0f, 0.0f, 0.0f};
  std::array<float, 4> padding_{0.0f, 0.0f, 0.0f, 0.0f};
  Direction direction_{Direction::LTR};
  bool hadOverflow_{false};
};

} // namespace nisaba::layout
