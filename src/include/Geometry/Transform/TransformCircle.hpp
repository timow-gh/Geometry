#ifndef GEOMETRY_TRANSFORMCIRCLE_HPP
#define GEOMETRY_TRANSFORMCIRCLE_HPP

#include "Geometry/Circle.hpp"
#include "Geometry/Transform/TransformVec.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Circle2<T> transform(const Circle2<T>& circle, const linal::hcoord::hmat<T>& hmat) noexcept
{
  linal::vec2<T> vec = transform(circle.get_origin(), hmat);
  return Circle2<T>{vec, circle.get_radius()};
}

template <typename T>
GEO_NODISCARD constexpr Circle3<T> transform(const Circle3<T>& circle, const linal::hcoord::hmat<T>& hmat) noexcept
{
  return Circle3<T>{transform(circle.get_origin(), hmat), circle.get_radius(), circle.get_normal()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCIRCLE_HPP
