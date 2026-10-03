#pragma once

/// @file lottie_types.hpp
/// @brief Core types and enumerations for the sovereign Lottie subsystem in Nisaba.

#include <cstdint>

namespace nisaba::lottie {

enum class LayerType : uint8_t {
    Precomp = 0,
    Solid = 1,
    Image = 2,
    Null = 3,
    Shape = 4,
    Text = 5,
    Unknown = 255
};

enum class MatteType : uint8_t {
    None = 0,
    Alpha = 1,
    InvertedAlpha = 2,
    Luma = 3,
    InvertedLuma = 4
};

enum class LineCap : uint8_t {
    Butt = 1,
    Round = 2,
    Square = 3
};

enum class LineJoin : uint8_t {
    Miter = 1,
    Round = 2,
    Bevel = 3
};

enum class FillRule : uint8_t {
    NonZero = 1,
    EvenOdd = 2
};

enum class GradientType : uint8_t {
    Linear = 1,
    Radial = 2
};

enum class TrimType : uint8_t {
    Simultaneously = 1,
    Individually = 2
};

enum class MaskMode : char {
    None = 'n',
    Add = 'a',
    Subtract = 's',
    Intersect = 'i',
    Difference = 'f'
};

} // namespace nisaba::lottie
