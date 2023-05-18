#ifndef GLFWTESTAPP_SPHERE_H
#define GLFWTESTAPP_SPHERE_H

#include <Geometry/Utils/Compiler.hpp>
#include <linal/vec3.hpp>
#include <linal/vec_operations.hpp>
#include <linal/utils/eps.hpp>

namespace Geometry
{
template <typename T>
class Sphere {
  linal::vec3<T> m_origin;
  T m_radius;

public:
  Sphere(const linal::vec3<T>& origin, T radius)
      : m_origin(origin)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::vec3<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR T getRadius() const { return m_radius; }

  GEO_CONSTEXPR void setOrigin(const linal::vec3<T>& origin) { m_origin = origin; }
  GEO_CONSTEXPR void setRadius(T radius) { m_radius = radius; }

  GEO_NODISCARD GEO_CONSTEXPR bool contains(const linal::vec3<T>& vec) const
  {
    return !linal::isGreater(linal::norm2(linal::vec3<T>{vec - m_origin}), m_radius);
  }

  GEO_CONSTEXPR bool operator==(const Sphere& rhs) const { return m_origin == rhs.m_origin && m_radius == rhs.m_radius; }
  GEO_CONSTEXPR bool operator!=(const Sphere& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GLFWTESTAPP_SPHERE_H
