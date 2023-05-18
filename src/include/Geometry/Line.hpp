#ifndef GLFWTESTAPP_LINE_H
#define GLFWTESTAPP_LINE_H

#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Line {
  linal::Vec<T, D> m_origin;
  linal::Vec<T, D> m_direction;

public:
  GEO_CONSTEXPR Line(linal::Vec<T, D> origin, linal::Vec<T, D> direction)
      : m_origin(origin)
      , m_direction(direction)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR linal::Vec<T, D> getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR linal::Vec<T, D> getDirection() const { return m_direction; }

  GEO_CONSTEXPR bool operator==(const Line& rhs) const { return m_origin == rhs.m_origin && m_direction == rhs.m_direction; }
  GEO_CONSTEXPR bool operator!=(const Line& rhs) const { return !(rhs == *this); }
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

#endif // GLFWTESTAPP_LINE_H
