#ifndef GEOMETRY_TRIANGLE_HPP
#define GEOMETRY_TRIANGLE_HPP

#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/PointQueries.hpp"
#include <array>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
namespace Geometry
{

template <typename T, std::uint8_t D>
class Triangle {
  std::array<linal::vec<T, D>, 3> m_trianglePoints;

public:
  using value_type = T;
  using container_type = std::array<linal::vec<T, D>, 3>;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  constexpr Triangle() noexcept = default;
  constexpr explicit Triangle(const std::array<linal::vec<T, D>, 3>& trianglePoints) noexcept
      : m_trianglePoints(trianglePoints)
  {
  }
  constexpr Triangle(linal::vec<T, D> first, linal::vec<T, D> second, linal::vec<T, D> third) noexcept
      : m_trianglePoints({first, second, third})
  {
  }

  GEO_NODISCARD constexpr bool is_degenerate() const noexcept
  {
    return points_are_collinear(m_trianglePoints[0], m_trianglePoints[1], m_trianglePoints[2]);
  }

  GEO_NODISCARD constexpr iterator begin() noexcept { return m_trianglePoints.begin(); }
  GEO_NODISCARD constexpr const_iterator begin() const noexcept { return m_trianglePoints.begin(); }
  GEO_NODISCARD constexpr const_iterator cbegin() const noexcept { return m_trianglePoints.begin(); }
  GEO_NODISCARD constexpr iterator end() noexcept { return m_trianglePoints.end(); }
  GEO_NODISCARD constexpr const_iterator end() const noexcept { return m_trianglePoints.end(); }
  GEO_NODISCARD constexpr const_iterator cend() const noexcept { return m_trianglePoints.end(); }

  GEO_NODISCARD
  constexpr linal::vec<T, D>& operator[](std::size_t index) noexcept
  {
    GEO_ASSERT(index < 3);
    return m_trianglePoints[index];
  }
  constexpr const linal::vec<T, D>& operator[](std::size_t index) const noexcept
  {
    GEO_ASSERT(index < 3);
    return m_trianglePoints[index];
  }
  constexpr linal::vec<T, D>& get_point(std::size_t index) noexcept
  {
    GEO_ASSERT(index < 3);
    return m_trianglePoints[index];
  }
  constexpr const linal::vec<T, D>& get_point(std::size_t index) const noexcept
  {
    GEO_ASSERT(index < 3);
    return m_trianglePoints[index];
  }
  constexpr void set_point(std::size_t index, const linal::vec<T, D>& point) noexcept
  {
    GEO_ASSERT(index < 3);
    m_trianglePoints[index] = point;
  }
  constexpr void set_points(const std::array<linal::vec<T, D>, 3>& points) noexcept
  {
    m_trianglePoints = points;
  }
  constexpr void set_points(linal::vec<T, D> first, linal::vec<T, D> second, linal::vec<T, D> third) noexcept
  {
    m_trianglePoints = {first, second, third};
  }

  GEO_NODISCARD constexpr const std::array<linal::vec<T, D>, 3>& get_triangle_points() const noexcept { return m_trianglePoints; }

  GEO_NODISCARD constexpr linal::vec<T, D> calc_normal() const noexcept
  {
    return linal::cross(m_trianglePoints[1] - m_trianglePoints[0], m_trianglePoints[2] - m_trianglePoints[0]);
  }
  GEO_NODISCARD constexpr linal::vec<T, D> calc_center() const noexcept
  {
    return (m_trianglePoints[0] + m_trianglePoints[1] + m_trianglePoints[2]) / static_cast<T>(3);
  }
  GEO_NODISCARD constexpr T calc_area() const noexcept
  {
    return linal::length(calc_normal()) / static_cast<T>(2);
  }
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
