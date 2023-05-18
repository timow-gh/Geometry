#ifndef GEOMETRY_INTERVAL_HPP
#define GEOMETRY_INTERVAL_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <cstdint>

namespace Geometry
{
template <typename T>
class Interval {
  T m_start;
  T m_end;

public:
  GEO_CONSTEXPR Interval() = default;
  GEO_CONSTEXPR Interval(T start, T end)
      : m_start(start)
      , m_end(end)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR T getStart() const { return m_start; }
  GEO_NODISCARD GEO_CONSTEXPR T getEnd() const { return m_end; }
  GEO_CONSTEXPR void setStart(T start) { m_start = start; }
  GEO_CONSTEXPR void setEnd(T end) { m_end = end; }

  GEO_CONSTEXPR bool operator==(const Interval& rhs) const { return m_start == rhs.m_start && m_end == rhs.m_end; }
  GEO_CONSTEXPR bool operator!=(const Interval& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_INTERVAL_HPP
