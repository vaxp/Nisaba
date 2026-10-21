#include "nisaba/physics/collision/shape.hpp"
#include "nisaba/physics/collision/sphere_shape.hpp"
#include "nisaba/physics/collision/box_shape.hpp"
#include "nisaba/physics/collision/capsule_shape.hpp"
#include "nisaba/physics/collision/plane_shape.hpp"
#include "nisaba/physics/collision/cylinder_shape.hpp"
#include "nisaba/physics/collision/convex_hull_shape.hpp"

namespace nisaba::physics {

// Anchor translation unit for collision shapes
static_assert(sizeof(MassProperties) >= 48, "MassProperties minimum layout");

} // namespace nisaba::physics
