#ifndef GEOMETRY_DISTANCESEGMENT_HPP
#define GEOMETRY_DISTANCESEGMENT_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
CORE_CONSTEXPR T distance(const Segment<T, D>& segment, LinAl::Vec<T, D> vec)
{
  LinAl::Vec<T, D> source = segment.getSource();
  LinAl::Vec<T, D> ps = vec - source;
  LinAl::Vec<T, D> ts = segment.getTarget() - source;
  T parameter = LinAl::dot(ps, ts) / LinAl::norm2(ts);
  T projParameter = std::min(std::max(parameter, T(0.0)), T(1.0));
  LinAl::Vec<T, D> projVec{source + projParameter * ts};
  return LinAl::norm2(LinAl::Vec<T, D>{projVec - vec});
}

template <typename T, std::size_t D>
CORE_CONSTEXPR T distance(LinAl::Vec<T, D> vec, const Segment<T, D>& segment)
{
  return distance(segment, vec);
}

template <typename T, std::size_t D>
CORE_CONSTEXPR T distance(const Line<T, D>& line, const Segment<T, D>& segment)
{
  return distance(segment, line);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCESEGMENT_HPP
