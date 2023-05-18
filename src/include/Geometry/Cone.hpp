#ifndef GEOMETRY_CONE_HPP
#define GEOMETRY_CONE_HPP

#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec3.hpp>

namespace Geometry
{
template <typename T>
class Cone {
  Geometry::Segment3<T> m_segment;
  T m_radius;

public:
  GEO_CONSTEXPR
  Cone(const linal::Vec3<T>& circleMidPoint, const linal::Vec3<T>& peak, const T& radius)
      : m_segment(Segment3<T>{circleMidPoint, peak})
      , m_radius(radius)
  {
  }

  GEO_CONSTEXPR Cone(const Segment3<T>& segment, const T& radius)
      : m_segment(segment)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const Segment3<T>& get_segment() const { return m_segment; }
  GEO_NODISCARD GEO_CONSTEXPR const T& getRadius() const { return m_radius; }
  GEO_NODISCARD GEO_CONSTEXPR T get_height() const { return m_segment.length(); }
  GEO_NODISCARD GEO_CONSTEXPR T get_slant_height() const
  {
    const T h = get_height();
    return std::sqrt(m_radius * m_radius + h * h);
  }
  GEO_NODISCARD GEO_CONSTEXPR T get_opening_angle() const { return std::atan(m_radius / get_height()) * 2; }

  GEO_CONSTEXPR bool operator==(const Cone& rhs) const { return m_segment == rhs.m_segment && m_radius == rhs.m_radius; }
  GEO_CONSTEXPR bool operator!=(const Cone& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_CONE_HPP
