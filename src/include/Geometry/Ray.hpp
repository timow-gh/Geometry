#ifndef GLFWTESTAPP_RAY_HPP
#define GLFWTESTAPP_RAY_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Fwd/FwdRay.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Ray {
  LinAl::Vec<T, D> m_origin;
  LinAl::Vec<T, D> m_direction;

public:
  CORE_CONSTEXPR Ray(LinAl::Vec<T, D> origin, LinAl::Vec<T, D> direction)
      : m_origin(origin)
      , m_direction(direction)
  {
  }

  CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec<T, D> getOrigin() const { return m_origin; }
  CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec<T, D> getDirection() const { return m_direction; }

  CORE_CONSTEXPR bool operator==(const Ray& rhs) const { return m_origin == rhs.m_origin && m_direction == rhs.m_direction; }
  CORE_CONSTEXPR bool operator!=(const Ray& rhs) const { return !(rhs == *this); }
};

} // namespace Geometry

#endif // GLFWTESTAPP_RAY_HPP
