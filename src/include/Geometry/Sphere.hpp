#ifndef GEOMETRY_SPHERE_H
#define GEOMETRY_SPHERE_H

#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_compare.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{
template <typename T>
class Sphere {
  linal::vec3<T> m_origin;
  T m_radius;

public:
  Sphere(const linal::vec3<T>& origin, T radius) noexcept
      : m_origin(origin)
      , m_radius(radius)
  {
  }

  GEO_NODISCARD constexpr const linal::vec3<T>& get_origin() const noexcept { return m_origin; }
  GEO_NODISCARD constexpr T get_radius() const noexcept { return m_radius; }

  constexpr void set_origin(const linal::vec3<T>& origin) noexcept { m_origin = origin; }
  constexpr void set_radius(T radius) noexcept { m_radius = radius; }

  GEO_NODISCARD constexpr bool contains(const linal::vec3<T>& vec) const noexcept
  {
    return !linal::isGreater(linal::length(linal::vec3<T>{vec - m_origin}), m_radius);
  }

  constexpr bool operator==(const Sphere& rhs) const noexcept
  {
    return linal::is_equal(m_origin, rhs.m_origin) && linal::isEq(m_radius, rhs.m_radius);
  }
  constexpr bool operator!=(const Sphere& rhs) const noexcept { return !(rhs == *this); }
};

using SphereF = Sphere<float>;
using SphereD = Sphere<double>;

} // namespace Geometry

#endif // GEOMETRY_SPHERE_H
