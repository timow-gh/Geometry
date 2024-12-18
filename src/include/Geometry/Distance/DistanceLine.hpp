#ifndef GEOMETRY_DISTANCELINE_HPP
#define GEOMETRY_DISTANCELINE_HPP

#include "Geometry/Line.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <Geometry/Utils/Assert.hpp>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(linal::vec<T, D> vec, const Line<T, D>& line) noexcept
{
  return linal::length(linal::rejection(linal::vec<T, D>{line.get_origin() - vec}, line.get_direction()));
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Line<T, D>& line, linal::vec<T, D> vec) noexcept
{
  return distance(vec, line);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Line<T, D>& lhs, const Line<T, D>& rhs) noexcept
{
  GEO_ASSERT(linal::isEq(linal::length(lhs.get_direction()), 1.0));
  GEO_ASSERT(linal::isEq(linal::length(rhs.get_direction()), 1.0));

  linal::vec<T, D> cross = linal::cross(lhs.get_direction(), rhs.get_direction());
  T crossLen = linal::length(cross);
  if (linal::isZero(crossLen)) // If the cross product is zero, the lines are parallel.
  {
    return distance(lhs.get_origin(), rhs);
  }
  return linal::dot(linal::vec<T, D>{lhs.get_origin() - rhs.get_origin()}, cross) / crossLen;
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCELINE_HPP
