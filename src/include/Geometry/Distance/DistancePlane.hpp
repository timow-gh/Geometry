#ifndef GEOMETRY_DISTANCEPLANE_HPP
#define GEOMETRY_DISTANCEPLANE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Plane.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Plane<T>& plane, linal::Vec<T, D> vec)
{
  linal::Vec<T, D> vecToPlane = plane.getOrigin() - vec;
  return std::abs(linal::dot(vecToPlane, plane.getNormal()));
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(linal::Vec<T, D> vec, const Plane<T>& plane)
{
  return distance(plane, vec);
}
} // namespace Geometry

#endif // GEOMETRY_DISTANCEPLANE_HPP
