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
  GEO_CONSTEXPR Triangle() = default;

  GEO_CONSTEXPR explicit Triangle(const linal::vecArray<T, D, 3>& trianglePoints)
      : m_trianglePoints(trianglePoints)
  {
  }

  GEO_CONSTEXPR Triangle(linal::vec<T, D> first, linal::vec<T, D> second, linal::vec<T, D> third)
      : m_trianglePoints({first, second, third})
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::vecArray<T, D, 3>& getTrianglePoints() const { return m_trianglePoints; }
};

} // namespace Geometry

#endif // GEOMETRY_TRIANGLE_HPP
