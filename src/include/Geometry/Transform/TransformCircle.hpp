#ifndef GEOMETRY_TRANSFORMCIRCLE_HPP
#define GEOMETRY_TRANSFORMCIRCLE_HPP

#include "Geometry/Circle.hpp"
#include "Geometry/Transform/TransformVec.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Circle2<T> transform(const Circle2<T>& circle, const linal::hmat<T>& hmat) noexcept
{
  const auto& origin = circle.get_origin();
  linal::vec3<T> vec = transform(linal::vec3<T>{origin[0], origin[1], 0}, hmat);
  return Circle2<T>{linal::vec2<T>{vec[0], vec[1]}, circle.get_radius()};
}

template <typename T>
GEO_NODISCARD constexpr Circle3<T> transform(const Circle3<T>& circle, const linal::hmat<T>& hmat) noexcept
{
  const linal::hvec<T> transformedNormal = hmat * linal::to_hvec<T, 3>(circle.get_normal(), T{0});
  return Circle3<T>{transform(circle.get_origin(), hmat),
                    circle.get_radius(),
                    linal::vec3<T>{transformedNormal[0], transformedNormal[1], transformedNormal[2]}};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCIRCLE_HPP
