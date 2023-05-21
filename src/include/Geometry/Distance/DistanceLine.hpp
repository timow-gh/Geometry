#ifndef GEOMETRY_DISTANCELINE_HPP
#define GEOMETRY_DISTANCELINE_HPP

#include <Geometry/Line.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(linal::vec<T, D> vec, const Line<T, D>& line)
{
  return linal::norm2(linal::rejection(linal::vec<T, D>{line.getOrigin() - vec}, line.getDirection()));
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Line<T, D>& line, linal::vec<T, D> vec)
{
  return distance(vec, line);
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Line<T, D>& lhs, const Line<T, D>& rhs)
{
  linal::vec<T, D> cross = linal::cross(lhs.getDirection(), rhs.getDirection());
  T crossLen = linal::norm2(cross);
  if (linal::isZero(crossLen))
  {
    return distance(lhs.getOrigin(), rhs);
  }
  return linal::dot(linal::vec<T, D>{lhs.getOrigin() - rhs.getOrigin()}, cross) / crossLen;
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCELINE_HPP
