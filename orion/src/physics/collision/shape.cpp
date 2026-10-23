#include "orion/physics/collision/shape.hpp"
#include "orion/physics/collision/sphere_shape.hpp"
#include "orion/physics/collision/box_shape.hpp"
#include "orion/physics/collision/capsule_shape.hpp"
#include "orion/physics/collision/plane_shape.hpp"
#include "orion/physics/collision/cylinder_shape.hpp"
#include "orion/physics/collision/convex_hull_shape.hpp"

namespace orion::physics {

// Anchor translation unit for collision shapes
static_assert(sizeof(MassProperties) >= 48, "MassProperties minimum layout");

} // namespace orion::physics
