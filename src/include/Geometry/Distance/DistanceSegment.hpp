#ifndef GEOMETRY_DISTANCESEGMENT_HPP
#define GEOMETRY_DISTANCESEGMENT_HPP

#include "Geometry/Line.hpp"
#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Segment<T, D>& segment, linal::vec<T, D> vec) noexcept
{
  linal::vec<T, D> source = segment.get_source();
  linal::vec<T, D> ps = vec - source;
  linal::vec<T, D> ts = segment.get_target() - source;
  T parameter = linal::dot(ps, ts) / linal::length(ts);
  T projParameter = std::min(std::max(parameter, T(0.0)), T(1.0));
  linal::vec<T, D> projVec{source + projParameter * ts};
  return linal::length(linal::vec<T, D>{projVec - vec});
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(linal::vec<T, D> vec, const Segment<T, D>& segment) noexcept
{
  return distance(segment, vec);
}

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr T distance(const Line<T, D>& line, const Segment<T, D>& segment) noexcept
{
  return distance(segment, line);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCESEGMENT_HPP
