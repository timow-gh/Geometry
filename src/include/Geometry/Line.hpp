#ifndef GLFWTESTAPP_LINE_H
#define GLFWTESTAPP_LINE_H

#include <Core/Math/Eps.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Fwd/FwdLine.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class Line {
  LinAl::Vec<T, D> m_origin;
  LinAl::Vec<T, D> m_direction;

public:
  GEO_CONSTEXPR Line(LinAl::Vec<T, D> origin, LinAl::Vec<T, D> direction)
      : m_origin(origin)
      , m_direction(LinAl::normalize(LinAl::Vec<T, D>{direction}))
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec<T, D> getOrigin() const { return m_origin; }
  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec<T, D> getDirection() const { return m_direction; }

  GEO_CONSTEXPR bool operator==(const Line& rhs) const { return m_origin == rhs.m_origin && m_direction == rhs.m_direction; }
  GEO_CONSTEXPR bool operator!=(const Line& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GLFWTESTAPP_LINE_H
