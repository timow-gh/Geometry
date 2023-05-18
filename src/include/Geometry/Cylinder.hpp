#ifndef GEOMETRY_CYLINDER_HPP
#define GEOMETRY_CYLINDER_HPP

#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>

namespace Geometry
{

template <typename T>
class Cylinder {
  Segment3<T> m_segment;
  T m_radius;

public:
  GEO_CONSTEXPR Cylinder(const Segment3<T>& segment, T radius)
      : m_segment(segment)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const Segment3<T>& getSegment() const { return m_segment; }
  GEO_NODISCARD GEO_CONSTEXPR T getRadius() const { return m_radius; }
  GEO_NODISCARD GEO_CONSTEXPR T height() const { return m_segment.length(); }

  GEO_CONSTEXPR bool operator==(const Cylinder& rhs) const { return m_segment == rhs.m_segment && m_radius == rhs.m_radius; }
  GEO_CONSTEXPR bool operator!=(const Cylinder& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_CYLINDER_HPP
