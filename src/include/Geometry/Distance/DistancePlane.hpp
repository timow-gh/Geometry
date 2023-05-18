#ifndef GEOMETRY_DISTANCEPLANE_HPP
#define GEOMETRY_DISTANCEPLANE_HPP

#include <Geometry/Plane.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Plane<T>& plane, linal::vec<T, D> vec)
{
  linal::vec<T, D> vecToPlane = plane.getOrigin() - vec;
  return std::abs(linal::dot(vecToPlane, plane.getNormal()));
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(linal::vec<T, D> vec, const Plane<T>& plane)
{
  return distance(plane, vec);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCEPLANE_HPP
