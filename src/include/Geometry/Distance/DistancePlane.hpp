#ifndef GEOMETRY_DISTANCEPLANE_HPP
#define GEOMETRY_DISTANCEPLANE_HPP

#include "Geometry/Plane.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Plane<T>& plane, linal::vec<T, D> vec) noexcept
{
  linal::vec<T, D> vecToPlane = plane.get_origin() - vec;
  return std::abs(linal::dot(vecToPlane, plane.get_normal()));
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(linal::vec<T, D> vec, const Plane<T>& plane) noexcept
{
  return distance(plane, vec);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCEPLANE_HPP
