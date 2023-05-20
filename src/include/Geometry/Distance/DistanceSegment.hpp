#ifndef GEOMETRY_DISTANCESEGMENT_HPP
#define GEOMETRY_DISTANCESEGMENT_HPP

#include <Geometry/Line.hpp>
#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Segment<T, D>& segment, linal::vec<T, D> vec)
{
  linal::vec<T, D> source = segment.getSource();
  linal::vec<T, D> ps = vec - source;
  linal::vec<T, D> ts = segment.getTarget() - source;
  T parameter = linal::dot(ps, ts) / linal::norm2(ts);
  T projParameter = std::min(std::max(parameter, T(0.0)), T(1.0));
  linal::vec<T, D> projVec{source + projParameter * ts};
  return linal::norm2(linal::vec<T, D>{projVec - vec});
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(linal::vec<T, D> vec, const Segment<T, D>& segment)
{
  return distance(segment, vec);
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Line<T, D>& line, const Segment<T, D>& segment)
{
  return distance(segment, line);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCESEGMENT_HPP
