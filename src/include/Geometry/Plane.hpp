#ifndef GLFWTESTAPP_PLANE_HPP
#define GLFWTESTAPP_PLANE_HPP

#include <linal/utils/Eps.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec3.hpp>

namespace Geometry
{
template <typename T>
class Plane {
private:
  linal::Vec3<T> m_origin;
  linal::Vec3<T> m_normal;

public:
  GEO_CONSTEXPR Plane() = default;
  GEO_CONSTEXPR Plane(const linal::Vec3<T>& origin, const linal::Vec3<T>& normal)
      : m_origin(origin)
      , m_normal(normal)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::Vec3<T>& getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR const linal::Vec3<T>& getNormal() const { return m_normal; }

  GEO_CONSTEXPR bool operator==(const Plane& rhs) const { return m_origin == rhs.m_origin && m_normal == rhs.m_normal; }
  GEO_CONSTEXPR bool operator!=(const Plane& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GLFWTESTAPP_PLANE_HPP
