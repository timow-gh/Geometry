#ifndef GEOMETRY_INTERVAL_HPP
#define GEOMETRY_INTERVAL_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <cstdint>
#include <linal/utils/eps.hpp>

namespace Geometry
{

template <typename T>
class Interval {
  T m_start;
  T m_end;

public:
  constexpr Interval() noexcept = default;
  constexpr Interval(T start, T end) noexcept
      : m_start(start)
      , m_end(end)
  {
  }

  GEO_NODISCARD constexpr T get_start() const noexcept { return m_start; }
  GEO_NODISCARD constexpr T get_end() const noexcept { return m_end; }
  constexpr void set_start(T start) noexcept { m_start = start; }
  constexpr void set_end(T end) noexcept { m_end = end; }

  constexpr bool operator==(const Interval& rhs) const noexcept
  {
    return linal::isEq(m_start, rhs.m_start) && linal::isEq(m_end, rhs.m_end);
  }
  constexpr bool operator!=(const Interval& rhs) const noexcept { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_INTERVAL_HPP
