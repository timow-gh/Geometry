#ifndef GEOMETRY_RAY_HPP
#define GEOMETRY_RAY_HPP

#include <Geometry/Ray.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/vec.hpp>
#include <linal/utils/eps.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Ray {
  linal::vec<T, D> m_origin;
  linal::vec<T, D> m_direction;

public:
  GEO_CONSTEXPR Ray(linal::vec<T, D> origin, linal::vec<T, D> direction)
      : m_origin(origin)
      , m_direction(direction)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR linal::vec<T, D> getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR linal::vec<T, D> getDirection() const { return m_direction; }

  GEO_CONSTEXPR bool operator==(const Ray& rhs) const { return m_origin == rhs.m_origin && m_direction == rhs.m_direction; }
  GEO_CONSTEXPR bool operator!=(const Ray& rhs) const { return !(rhs == *this); }
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
