#include "nisaba/physics/body/body.hpp"

namespace nisaba::physics {

// Anchor translation unit and strict memory layout validation
static_assert(sizeof(Body) == 128, "Body must be exactly 128 bytes (2 cache lines)");
static_assert(alignof(Body) == 16, "Body must be 16-byte aligned");
static_assert(sizeof(MotionProperties) == 128, "MotionProperties must be exactly 128 bytes (2 cache lines)");
static_assert(alignof(MotionProperties) == 16, "MotionProperties must be 16-byte aligned");
static_assert(sizeof(BodyID) == 4, "BodyID must be 4 bytes");

} // namespace nisaba::physics
