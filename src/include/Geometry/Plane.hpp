#ifndef GEOMETRY_PLANE_HPP
#define GEOMETRY_PLANE_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec3.hpp>
#include <linal/vec_compare.hpp>

namespace Geometry
{
template <typename T>
class Plane {
private:
  linal::vec3<T> m_origin;
  linal::vec3<T> m_normal;

public:
  constexpr Plane() noexcept = default;
  constexpr Plane(const linal::vec3<T>& origin, const linal::vec3<T>& normal) noexcept
      : m_origin(origin)
      , m_normal(normal)
  {
  }

  GEO_NODISCARD constexpr const linal::vec3<T>& get_origin() const noexcept { return m_origin; }
  GEO_NODISCARD constexpr const linal::vec3<T>& get_normal() const noexcept { return m_normal; }

  constexpr bool operator==(const Plane& rhs) const noexcept
  {
    return linal::is_equal(m_origin, rhs.m_origin) && linal::is_equal(m_normal, rhs.m_normal);
  }
  constexpr bool operator!=(const Plane& rhs) const noexcept { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GEOMETRY_PLANE_HPP
