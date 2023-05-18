#ifndef GEOMETRY_TRANSFORMCIRCLE_HPP
#define GEOMETRY_TRANSFORMCIRCLE_HPP

#include <Geometry/Circle.hpp>
#include <Geometry/Transformation/TransformVec.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Circle2<T> transformation(const Circle2<T>& circle, const linal::hcoord::hmat<T>& hmat)
{
  linal::vec2<T> vec = transformation(circle.getOrigin(), hmat);
  return Circle2<T>{vec, circle.getRadius()};
}

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Circle3<T> transformation(const Circle3<T>& circle, const linal::hcoord::hmat<T>& hmat)
{
  return Circle3<T>{transformation(circle.getOrigin(), hmat), circle.getRadius(), circle.getNormal()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCIRCLE_HPP
