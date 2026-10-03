#pragma once

/// @file animation.hpp
/// @brief Master umbrella header for Nisaba Sovereign Animation Subsystem.
///
/// Features:
///   - Easing Curves (Linear, EaseIn/Out, Bounce, Elastic, BackOut, CubicBezier)
///   - Generic Typed Tweens (Float, Color, Point, Rect, Size, Transform)
///   - AnimationController & AnimatedValue with Ticker bindings
///   - Exact Analytical Damped Spring Simulation & Interactive SpringController
///   - Multi-Track Sequenced Animation Timeline & Keyframing
///   - Staggered Entrance Cascades & Interval Curves
///   - 2D Particle Simulation Engine with Native Canvas Batch Rendering
///   - 100% Sovereign (Zero Third-Party Dependencies, Modern C++20)

#include "nisaba/animation/signal.hpp"
#include "nisaba/animation/curves.hpp"
#include "nisaba/animation/tween.hpp"
#include "nisaba/animation/ticker.hpp"
#include "nisaba/animation/animation_controller.hpp"
#include "nisaba/animation/spring_simulation.hpp"
#include "nisaba/animation/spring_controller.hpp"
#include "nisaba/animation/timeline.hpp"
#include "nisaba/animation/stagger.hpp"
#include "nisaba/animation/particle_system.hpp"
