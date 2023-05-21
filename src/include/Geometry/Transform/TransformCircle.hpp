#ifndef GEOMETRY_TRANSFORMCIRCLE_HPP
#define GEOMETRY_TRANSFORMCIRCLE_HPP

#include <Geometry/Circle.hpp>
#include <Geometry/Transform/TransformVec.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Circle2<T> transform(const Circle2<T>& circle, const linal::hcoord::hmat<T>& hmat)
{
  linal::vec2<T> vec = transform(circle.getOrigin(), hmat);
  return Circle2<T>{vec, circle.getRadius()};
}

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Circle3<T> transform(const Circle3<T>& circle, const linal::hcoord::hmat<T>& hmat)
{
  return Circle3<T>{transform(circle.getOrigin(), hmat), circle.getRadius(), circle.getNormal()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCIRCLE_HPP
