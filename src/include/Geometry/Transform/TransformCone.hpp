#ifndef GEOMETRY_TRANSFORMCONE_HPP
#define GEOMETRY_TRANSFORMCONE_HPP

#include "Geometry/Cone.hpp"
#include "Geometry/Transform/TransformSegment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Cone<T> transform(const Cone<T>& cone, const linal::hmat<T>& trafo) noexcept
{
  return Cone<T>{Geometry::transform(cone.get_segment(), trafo), cone.get_radius()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCONE_HPP
