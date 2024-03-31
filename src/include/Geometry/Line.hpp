#ifndef GEOMETRY_LINE_H
#define GEOMETRY_LINE_H

#include "Geometry/Utils/Compiler.hpp"
#include <linal/vec.hpp>
#include <linal/vec_compare.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
class Line {
  linal::vec<T, D> m_origin;
  linal::vec<T, D> m_direction;

public:
  constexpr Line(linal::vec<T, D> origin, linal::vec<T, D> direction) noexcept
      : m_origin(origin)
      , m_direction(direction)
  {
  }

  GEO_NODISCARD constexpr linal::vec<T, D> get_origin() const noexcept { return m_origin; }
  GEO_NODISCARD constexpr linal::vec<T, D> get_direction() const noexcept { return m_direction; }

  constexpr bool operator==(const Line& rhs) const noexcept
  {
    return linal::is_equal(m_origin, rhs.m_origin) && linal::is_equal(m_direction, rhs.m_direction);
  }
  constexpr bool operator!=(const Line& rhs) const noexcept { return !(rhs == *this); }
};

template <typename T>
using Line3 = Line<T, 3>;

template <typename T>
using Line2 = Line<T, 2>;
using Line2f = Line2<float>;
using Line2d = Line2<double>;

template <typename T>
using Line3 = Line<T, 3>;
using Line3f = Line3<float>;
using Line3d = Line3<double>;

} // namespace Geometry

#endif // GEOMETRY_LINE_H
