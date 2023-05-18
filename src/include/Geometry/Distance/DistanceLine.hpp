#ifndef GEOMETRY_DISTANCELINE_HPP
#define GEOMETRY_DISTANCELINE_HPP

#include <linal/utils/Eps.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(linal::Vec<T, D> vec, const Line<T, D>& line)
{
  return linal::norm2(linal::rejection(linal::Vec<T, D>{line.getOrigin() - vec}, line.getDirection()));
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Line<T, D>& line, linal::Vec<T, D> vec)
{
  return distance(vec, line);
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Line<T, D>& lhs, const Line<T, D>& rhs)
{
  linal::Vec<T, D> cross = linal::cross(lhs.getDirection(), rhs.getDirection());
  T crossLen = linal::norm2(cross);
  if (linal::isZero(crossLen))
    return distance(lhs.getOrigin(), rhs);
  return linal::dot(linal::Vec<T, D>{lhs.getOrigin() - rhs.getOrigin()}, cross) / crossLen;
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCELINE_HPP
