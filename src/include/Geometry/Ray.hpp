#ifndef GEOMETRY_RAY_HPP
#define GEOMETRY_RAY_HPP

#include "Geometry/Ray.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_compare.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
class Ray {
  linal::vec<T, D> m_origin;
  linal::vec<T, D> m_direction;

public:
  constexpr Ray(linal::vec<T, D> origin, linal::vec<T, D> direction) noexcept
      : m_origin(origin)
      , m_direction(linal::normalize(direction))
  {
    GEO_ASSERT(linal::length(direction) > linal::eps<T>::value);
  }

  GEO_NODISCARD constexpr linal::vec<T, D> get_origin() const noexcept { return m_origin; }
  GEO_NODISCARD constexpr linal::vec<T, D> get_direction() const noexcept { return m_direction; }

  constexpr void set_origin(linal::vec<T, D> origin) noexcept { m_origin = origin; }
  constexpr void set_direction(linal::vec<T, D> direction) noexcept { m_direction = direction; }

  constexpr bool operator==(const Ray& rhs) const noexcept
  {
    return linal::is_equal(m_origin, rhs.m_origin) && linal::is_equal(m_direction, rhs.m_direction);
  }
  constexpr bool operator!=(const Ray& rhs) const noexcept { return !(rhs == *this); }
};

template <typename T>
using Ray3 = class Ray<T, 3>;

template <typename T>
using Ray2 = Ray<T, 2>;
using Ray2f = Ray2<float>;
using Ray2d = Ray2<double>;

template <typename T>
using Ray3 = Ray<T, 3>;
using Ray3f = Ray3<float>;
using Ray3d = Ray3<double>;

} // namespace Geometry

#endif // GEOMETRY_RAY_HPP
