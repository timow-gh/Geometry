#ifndef GEOMETRY_TRIANGLE_HPP
#define GEOMETRY_TRIANGLE_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <linal/vec.hpp>

namespace Geometry
{
template <typename T, std::uint8_t D>
class Triangle {
  std::array<linal::vec3<T>, 3> m_trianglePoints;

public:
  constexpr Triangle() noexcept = default;
  constexpr explicit Triangle(const std::array<linal::vec<T, D>, 3>& trianglePoints) noexcept
      : m_trianglePoints(trianglePoints)
  {
  }
  constexpr Triangle(linal::vec<T, D> first, linal::vec<T, D> second, linal::vec<T, D> third) noexcept
      : m_trianglePoints({first, second, third})
  {
  }

  GEO_NODISCARD constexpr const std::array<linal::vec3<T>, 3>& get_triangle_points() const noexcept { return m_trianglePoints; }
};

using Triangle2f = Triangle<float, 2>;
using Triangle3f = Triangle<float, 3>;

using Triangle2d = Triangle<double, 2>;
using Triangle3d = Triangle<double, 3>;

} // namespace Geometry

#endif // GEOMETRY_TRIANGLE_HPP
