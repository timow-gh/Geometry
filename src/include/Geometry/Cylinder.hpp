#ifndef GEOMETRY_CYLINDER_HPP
#define GEOMETRY_CYLINDER_HPP

#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>

namespace Geometry
{

template <typename T>
class Cylinder {
  Segment3<T> m_segment;
  T m_radius;

public:
  constexpr Cylinder(const Segment3<T>& segment, T radius) noexcept
      : m_segment(segment)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD constexpr const Segment3<T>& get_segment() const noexcept { return m_segment; }
  GEO_NODISCARD constexpr T get_radius() const noexcept { return m_radius; }
  GEO_NODISCARD constexpr T height() const noexcept { return m_segment.length(); }

  constexpr bool operator==(const Cylinder& rhs) const noexcept
  {
    return m_segment == rhs.m_segment && linal::isEq(m_radius, rhs.m_radius);
  }
  constexpr bool operator!=(const Cylinder& rhs) const noexcept { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_CYLINDER_HPP
