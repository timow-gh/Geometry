#ifndef GEOMETRY_INTERVAL_HPP
#define GEOMETRY_INTERVAL_HPP

#include <Core/Utils/Compiler.hpp>
#include <cstdint>

namespace Geometry
{
template <typename T>
class Interval {
    T m_start;
    T m_end;

  public:
    CORE_CONSTEXPR Interval() = default;
    CORE_CONSTEXPR Interval(T start, T end) : m_start(start), m_end(end) {}

    CORE_NODISCARD CORE_CONSTEXPR T getStart() const { return m_start; }
    CORE_NODISCARD CORE_CONSTEXPR T getEnd() const { return m_end; }
    CORE_CONSTEXPR void setStart(T start) { m_start = start; }
    CORE_CONSTEXPR void setEnd(T end) { m_end = end; }

    CORE_CONSTEXPR bool operator==(const Interval& rhs) const { return m_start == rhs.m_start && m_end == rhs.m_end; }
    CORE_CONSTEXPR bool operator!=(const Interval& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_INTERVAL_HPP
