#ifndef GEOMETRY_PLANE_HPP
#define GEOMETRY_PLANE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <linal/vec3.hpp>
#include <linal/utils/eps.hpp>

namespace Geometry
{
template <typename T>
class Plane {
private:
  linal::vec3<T> m_origin;
  linal::vec3<T> m_normal;

public:
  GEO_CONSTEXPR Plane() = default;
  GEO_CONSTEXPR Plane(const linal::vec3<T>& origin, const linal::vec3<T>& normal)
      : m_origin(origin)
      , m_normal(normal)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::vec3<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR const linal::vec3<T>& getNormal() const { return m_normal; }

  GEO_CONSTEXPR bool operator==(const Plane& rhs) const { return m_origin == rhs.m_origin && m_normal == rhs.m_normal; }
  GEO_CONSTEXPR bool operator!=(const Plane& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_PLANE_HPP
