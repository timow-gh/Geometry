#ifndef GEOMETRY_TRANSFORMCYLINDER_HPP
#define GEOMETRY_TRANSFORMCYLINDER_HPP

#include <Geometry/Cylinder.hpp>
#include <Geometry/Transform/TransformSegment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Cylinder<T> transform(const Cylinder<T>& cylinder, const linal::hcoord::hmat<T>& trafo) noexcept
{
  return Cylinder<T>{transform(cylinder.get_segment(), trafo), cylinder.get_radius()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCYLINDER_HPP
