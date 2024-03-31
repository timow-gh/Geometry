#ifndef GEOMETRY_CONE_HPP
#define GEOMETRY_CONE_HPP

#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/vec.hpp>

namespace Geometry
{
template <typename T>
class Cone {
  Geometry::Segment3<T> m_segment;
  T m_radius;

public:
  constexpr Cone(const linal::vec3<T>& circleMidPoint, const linal::vec3<T>& peak, const T& radius) noexcept
      : m_segment(Segment3<T>{circleMidPoint, peak})
      , m_radius(radius)
  {
  }

  constexpr Cone(const Segment3<T>& segment, const T& radius) noexcept
      : m_segment(segment)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD constexpr const Segment3<T>& get_segment() const noexcept { return m_segment; }
  GEO_NODISCARD constexpr const T& get_radius() const noexcept { return m_radius; }

  constexpr void set_segment(const Segment3<T>& segment) noexcept { m_segment = segment; }
  constexpr void set_radius(const T& radius) noexcept { m_radius = radius; }

  GEO_NODISCARD constexpr T get_height() const noexcept { return m_segment.length(); }
  GEO_NODISCARD constexpr T get_slant_height() const noexcept
  {
    const T h = get_height();
    return std::sqrt(m_radius * m_radius + h * h);
  }
  GEO_NODISCARD constexpr T get_opening_angle() const noexcept { return std::atan(m_radius / get_height()) * 2; }

  constexpr bool operator==(const Cone& rhs) const noexcept { return m_segment == rhs.m_segment && linal::isEq(m_radius, rhs.m_radius); }
  constexpr bool operator!=(const Cone& rhs) const noexcept { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_CONE_HPP
