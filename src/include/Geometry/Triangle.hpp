#ifndef GEOMETRY_TRIANGLE_HPP
#define GEOMETRY_TRIANGLE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <linal/containers.hpp>
#include <linal/vec.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class Triangle {

  linal::vecArray<T, D, 3> m_trianglePoints;

public:
  constexpr Triangle() noexcept = default;

  constexpr explicit Triangle(const linal::vecArray<T, D, 3>& trianglePoints) noexcept
      : m_trianglePoints(trianglePoints)
  {
  }

  constexpr Triangle(linal::vec<T, D> first, linal::vec<T, D> second, linal::vec<T, D> third) noexcept
      : m_trianglePoints({first, second, third})
  {
  }

  GEO_NODISCARD constexpr const linal::vecArray<T, D, 3>& get_triangle_points() const noexcept { return m_trianglePoints; }
};

} // namespace Geometry

#endif // GEOMETRY_TRIANGLE_HPP
