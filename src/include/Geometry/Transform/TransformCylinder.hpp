#ifndef GEOMETRY_TRANSFORMCYLINDER_HPP
#define GEOMETRY_TRANSFORMCYLINDER_HPP

#include <Geometry/Cylinder.hpp>
#include <Geometry/Transform/TransformSegment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Cylinder<T> transform(const Cylinder<T>& cylinder, const linal::hcoord::hmat<T>& trafo)
{
  return Cylinder<T>{transform(cylinder.getSegment(), trafo), cylinder.getRadius()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCYLINDER_HPP
