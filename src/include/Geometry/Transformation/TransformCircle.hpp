#ifndef FILAPP_TRANSFORMCIRCLE_HPP
#define FILAPP_TRANSFORMCIRCLE_HPP

#include <Geometry/Circle.hpp>
#include <Geometry/Transformation/TransformVec.hpp>
#include <Geometry/Utils/Compiler.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Circle3<T> transformation(const Circle3<T>& circle, const linal::hcoord::hmat<T>& transformation)
{
  return Circle3<T>{transformation(circle.getOrigin(), transformation), circle.getRadius(), circle.getNormal()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCIRCLE_HPP
