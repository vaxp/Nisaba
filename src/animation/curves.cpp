/// @file curves.cpp
/// @brief Static instances of built-in animation curves for Nisaba.

#include "nisaba/animation/curves.hpp"

namespace nisaba::animation {

// Static curve instances — referenced as Curves::linear, etc.
const LinearCurve       Curves::linear;
const EaseInCurve       Curves::easeIn;
const EaseOutCurve      Curves::easeOut;
const EaseInOutCurve    Curves::easeInOut;
const ElasticOutCurve   Curves::elasticOut;
const BounceOutCurve    Curves::bounceOut;
const BackOutCurve      Curves::backOut;
const FastOutSlowInCurve Curves::fastOutSlowIn;

} // namespace nisaba::animation
