#ifndef GEOMETRY_CIRCLE_HPP
#define GEOMETRY_CIRCLE_HPP

#include <Geometry/GeomPredicates.hpp>
#include <Geometry/Utils/Assert.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/hvec.hpp>
#include <linal/utils/eps.hpp>
#include <linal/vec2.hpp>
#include <linal/vec3.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T>
class Circle2 {
  linal::vec2<T> m_origin;
  T m_radius;

public:
  constexpr Circle2(const linal::vec2<T>& origin, T radius) noexcept
      : m_origin(origin)
      , m_radius(radius)
  {
    GEO_ASSERT(radius >= 0);
  }

  GEO_NODISCARD constexpr linal::vec2<T> get_origin() const noexcept { return m_origin; }
  GEO_NODISCARD constexpr T get_radius() const noexcept { return m_radius; }

  constexpr void set_origin(linal::vec2<T> origin) noexcept { m_origin = origin; }
  constexpr void set_radius(T radius) noexcept
  {
    GEO_ASSERT(radius >= 0);
    m_radius = radius;
  }

  GEO_NODISCARD constexpr bool operator==(const Circle2& rhs) const
  {
    return is_equal(m_origin, rhs.m_origin) && linal::isEq(m_radius, rhs.m_radius);
  }
  GEO_NODISCARD constexpr bool operator!=(const Circle2& rhs) const { return !(rhs == *this); }
};

using Circle2f = Circle2<float>;
using Circle2d = Circle2<double>;

template <typename T>
class Circle3 {
  linal::vec3<T> m_origin;
  T m_radius;
  linal::vec3<T> m_normal;

public:
  constexpr Circle3(const linal::vec3<T>& origin, T radius, const linal::vec3<T>& normal)
      : m_origin(origin)
      , m_radius(radius)
      , m_normal(normal)
  {
  }

  GEO_NODISCARD constexpr linal::vec3<T> get_origin() const noexcept { return m_origin; }
  GEO_NODISCARD constexpr T get_radius() const noexcept { return m_radius; }
  GEO_NODISCARD constexpr linal::vec3<T> get_normal() const noexcept { return m_normal; }

  constexpr void set_origin(linal::vec3<T> origin) noexcept { m_origin = origin; }
  constexpr void set_radius(T radius) noexcept
  {
    GEO_ASSERT(radius >= 0);
    m_radius = radius;
  }
  constexpr void set_normal(linal::vec3<T> normal) noexcept
  {
    GEO_ASSERT(linal::isEq(linal::norm2(normal), 1));
    m_normal = normal;
  }

  GEO_NODISCARD constexpr bool operator==(const Circle3& rhs) const noexcept
  {
    return is_equal(m_origin, rhs.m_origin) && linal::isEq(m_radius, rhs.m_radius) && is_equal(m_normal, rhs.m_normal);
  }
  GEO_NODISCARD constexpr bool operator!=(const Circle3& rhs) const noexcept { return !(rhs == *this); }
};

template <typename T>
class Circle3;
using Circle3d = Circle3<double>;

using Circle3f = Circle3<float>;
using Circle3d = Circle3<double>;

} // namespace Geometry

#endif // GEOMETRY_CIRCLE_HPP
