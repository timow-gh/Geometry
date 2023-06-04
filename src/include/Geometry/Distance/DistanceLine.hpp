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
GEO_NODISCARD constexpr T distance(linal::vec<T, D> vec, const Line<T, D>& line) noexcept
{
  return linal::norm2(linal::rejection(linal::vec<T, D>{line.get_origin() - vec}, line.get_direction()));
}

template <typename T, std::size_t D>
GEO_NODISCARD constexpr T distance(const Line<T, D>& line, linal::vec<T, D> vec) noexcept
{
  return distance(vec, line);
}

template <typename T, std::size_t D>
GEO_NODISCARD constexpr T distance(const Line<T, D>& lhs, const Line<T, D>& rhs) noexcept
{
  linal::vec<T, D> cross = linal::cross(lhs.get_direction(), rhs.get_direction());
  T crossLen = linal::norm2(cross);
  if (linal::isZero(crossLen))
  {
    return distance(lhs.get_origin(), rhs);
  }
  return linal::dot(linal::vec<T, D>{lhs.get_origin() - rhs.get_origin()}, cross) / crossLen;
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCELINE_HPP
