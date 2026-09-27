#ifndef GEOMETRY_POLYGONTRIANGULATIONCOSTS_HPP
#define GEOMETRY_POLYGONTRIANGULATIONCOSTS_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/detail/TriangleOrientation.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <optional>
#include <span>
#include <utility>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Summed orientation of the fan from corner 0: the signed doubled area of the polygon in 2D,
 * its area vector in 3D.
 *
 * Summing over the whole polygon makes the reference robust for concave and non-planar polygons,
 * where a single corner can turn the wrong way. O(n).
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD auto polygon_orientation(std::span<const linal::vec<T, D>> corners) noexcept
{
  GEO_ASSERT(corners.size() >= 3);
  auto orientation = triangle_orientation(corners[0], corners[1], corners[2]);
  for (std::size_t i = 3; i < corners.size(); ++i)
  {
    orientation = orientation + triangle_orientation(corners[0], corners[i - 1], corners[i]);
  }
  return orientation;
}

/**
 * \internal
 * \brief The polygon corners together with their reference orientation, shared by the built-in costs.
 *
 * Rejecting every triangle that disagrees with the polygon's orientation is what keeps a
 * triangulation of a concave polygon inside it, so every built-in cost starts with this test.
 */
template <typename T, std::uint8_t D>
class OrientedPolygon
{
public:
  using vec_t = linal::vec<T, D>;

  explicit OrientedPolygon(std::span<const vec_t> corners) noexcept
      : m_corners(corners)
      , m_reference(polygon_orientation(corners))
  {
    GEO_ASSERT(orientation_dot(m_reference, m_reference) > T{0});
  }

  GEO_NODISCARD const vec_t& corner(std::size_t index) const noexcept
  {
    GEO_ASSERT(index < m_corners.size());
    return m_corners[index];
  }

  // Doubled area of the triangle, or nothing if it is degenerate or turned against the polygon.
  GEO_NODISCARD std::optional<T> doubled_area(std::size_t first, std::size_t apex, std::size_t last) const noexcept
  {
    const auto orientation = triangle_orientation(corner(first), corner(apex), corner(last));
    if (!(orientation_dot(orientation, m_reference) > T{0}))
    {
      return std::nullopt;
    }
    return std::sqrt(orientation_dot(orientation, orientation));
  }

private:
  std::span<const vec_t> m_corners;
  decltype(triangle_orientation(std::declval<const vec_t&>(), std::declval<const vec_t&>(), std::declval<const vec_t&>())) m_reference;
};

template <typename T, std::uint8_t D>
GEO_NODISCARD T squared_distance(const linal::vec<T, D>& from, const linal::vec<T, D>& to) noexcept
{
  return linal::length_squared(linal::vec<T, D>{to - from});
}

} // namespace detail

/**
 * \brief Triangle cost for \c minimum_cost_triangulation favouring well-shaped triangles; the
 * recommended default.
 *
 * Prices a triangle as its summed squared edge lengths over its doubled area, which is smallest for
 * an equilateral triangle and grows without bound for slivers, so the triangulation avoids thin
 * triangles. Scale-invariant, so it can be combined across polygons of any size. Triangles turned
 * against the polygon's orientation, or degenerate, are forbidden. Holds a view of the corners, which
 * must outlive it.
 */
template <typename T, std::uint8_t D>
class AspectRatioCost
{
public:
  explicit AspectRatioCost(std::span<const linal::vec<T, D>> corners) noexcept
      : m_polygon(corners)
  {
  }

  GEO_NODISCARD T operator()(std::size_t first, std::size_t apex, std::size_t last) const noexcept
  {
    const std::optional<T> doubledArea = m_polygon.doubled_area(first, apex, last);
    if (!doubledArea)
    {
      return std::numeric_limits<T>::infinity();
    }
    const auto& firstCorner = m_polygon.corner(first);
    const auto& apexCorner = m_polygon.corner(apex);
    const auto& lastCorner = m_polygon.corner(last);
    const T squaredEdges = detail::squared_distance(firstCorner, apexCorner) + detail::squared_distance(apexCorner, lastCorner)
                           + detail::squared_distance(lastCorner, firstCorner);
    return squaredEdges / *doubledArea;
  }

private:
  detail::OrientedPolygon<T, D> m_polygon;
};

/**
 * \brief Triangle cost for \c minimum_cost_triangulation giving the triangulation of least total area.
 *
 * For a planar polygon every valid triangulation has the same area, so this only matters in 3D, where
 * it spans a hole with the tautest, "soap film" surface. Triangles turned against the polygon's
 * orientation, or degenerate, are forbidden. Holds a view of the corners, which must outlive it.
 */
template <typename T, std::uint8_t D>
class AreaCost
{
public:
  explicit AreaCost(std::span<const linal::vec<T, D>> corners) noexcept
      : m_polygon(corners)
  {
  }

  GEO_NODISCARD T operator()(std::size_t first, std::size_t apex, std::size_t last) const noexcept
  {
    const std::optional<T> doubledArea = m_polygon.doubled_area(first, apex, last);
    return doubledArea ? *doubledArea / T{2} : std::numeric_limits<T>::infinity();
  }

private:
  detail::OrientedPolygon<T, D> m_polygon;
};

/**
 * \brief Triangle cost for \c minimum_cost_triangulation giving the shortest total diagonal length.
 *
 * Prices a triangle by its perimeter; the polygon sides are in every triangulation, so the result is
 * the minimum-weight triangulation, which tends to connect nearby corners. Triangles turned against
 * the polygon's orientation, or degenerate, are forbidden. Holds a view of the corners, which must
 * outlive it.
 */
template <typename T, std::uint8_t D>
class EdgeLengthCost
{
public:
  explicit EdgeLengthCost(std::span<const linal::vec<T, D>> corners) noexcept
      : m_polygon(corners)
  {
  }

  GEO_NODISCARD T operator()(std::size_t first, std::size_t apex, std::size_t last) const noexcept
  {
    if (!m_polygon.doubled_area(first, apex, last))
    {
      return std::numeric_limits<T>::infinity();
    }
    const auto& firstCorner = m_polygon.corner(first);
    const auto& apexCorner = m_polygon.corner(apex);
    const auto& lastCorner = m_polygon.corner(last);
    return std::sqrt(detail::squared_distance(firstCorner, apexCorner)) + std::sqrt(detail::squared_distance(apexCorner, lastCorner))
           + std::sqrt(detail::squared_distance(lastCorner, firstCorner));
  }

private:
  detail::OrientedPolygon<T, D> m_polygon;
};

} // namespace Geometry

#endif // GEOMETRY_POLYGONTRIANGULATIONCOSTS_HPP
