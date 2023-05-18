#ifndef GEOMETRY_TRIANGLE_HPP
#define GEOMETRY_TRIANGLE_HPP

#include <LinAl/LinearAlgebra.hpp>
#include <Geometry/Utils/Compiler.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class Triangle {

  LinAl::VecArray<T, D, 3> m_trianglePoints;

public:
  GEO_CONSTEXPR Triangle() = default;

  GEO_CONSTEXPR explicit Triangle(const LinAl::VecArray<T, D, 3>& trianglePoints)
      : m_trianglePoints(trianglePoints)
  {
  }

  GEO_CONSTEXPR Triangle(LinAl::Vec<T, D> first, LinAl::Vec<T, D> second, LinAl::Vec<T, D> third)
      : m_trianglePoints({first, second, third})
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const LinAl::VecArray<T, D, 3>& getTrianglePoints() const { return m_trianglePoints; }
};

} // namespace Geometry

#endif // GEOMETRY_TRIANGLE_HPP
