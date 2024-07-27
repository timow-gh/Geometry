#ifndef GEOMETRY_TRIANGLE_HPP
#define GEOMETRY_TRIANGLE_HPP

#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/Utils/Assert.hpp"
#include <array>
#include <linal/vec.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
class Triangle {
  std::array<linal::vec<T, D>, 3> m_trianglePoints;

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

  GEO_NODISCARD constexpr const std::array<linal::vec<T, D>, 3>& get_triangle_points() const noexcept { return m_trianglePoints; }
};


template<typename T>
using Triangle2 = Triangle<T, 2>;
template <typename T>
using Triangle3 = Triangle<T, 3>;

using Triangle2f = Triangle2<float>;
using Triangle3f = Triangle3<float>;

using Triangle2d = Triangle2<double>;
using Triangle3d = Triangle3<double>;

} // namespace Geometry

#endif // GEOMETRY_TRIANGLE_HPP
