#include "nisaba/physics/math.hpp"

namespace nisaba::physics {

// Explicit instantiation and anchor translation unit for physics math symbols
static_assert(sizeof(Vec3) == 16, "Vec3 must be 16 bytes");
static_assert(alignof(Vec3) == 16, "Vec3 must be 16-byte aligned");
static_assert(sizeof(Quat) == 16, "Quat must be 16 bytes");
static_assert(alignof(Quat) == 16, "Quat must be 16-byte aligned");
static_assert(sizeof(Mat44) == 64, "Mat44 must be 64 bytes");
static_assert(alignof(Mat44) == 16, "Mat44 must be 16-byte aligned");
static_assert(sizeof(AABB) == 32, "AABB must be 32 bytes");
static_assert(alignof(AABB) == 16, "AABB must be 16-byte aligned");
static_assert(sizeof(Transform3D) == 32, "Transform3D must be 32 bytes");
static_assert(alignof(Transform3D) == 16, "Transform3D must be 16-byte aligned");

} // namespace nisaba::physics
