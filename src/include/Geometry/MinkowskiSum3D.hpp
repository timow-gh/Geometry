#ifndef GEOMETRY_MINKOWSKISUM3D_HPP
#define GEOMETRY_MINKOWSKISUM3D_HPP

#include "Geometry/MinkowskiSum2D.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <cstddef>
#include <span>
#include <vector>

namespace Geometry
{

/** @file MinkowskiSum3D.hpp
 *
 * Linear-time Minkowski sum and difference of two convex polygons that live in a common plane in
 * 3d space (coplanar polygons). General 3d polyhedra are not supported.
 *
 * These functions project the coplanar input into an in-plane 2d basis, delegate to the 2d
 * implementation in Geometry/MinkowskiSum2D.hpp, and lift the result back into 3d. All
 * preconditions and normalization semantics match the 2d variants; see MinkowskiSum2D.hpp.
 *
 * The @c a polygon defines the plane. Every other polygon passed alongside it (@c b, and the
 * result) is assumed to lie in that same plane.
 */

namespace details
{

/** @brief An orthonormal basis of the plane a coplanar 3d polygon lives in. */
template <typename T>
struct PlaneBasis
{
  linal::vec3<T> origin{};
  linal::vec3<T> u{};
  linal::vec3<T> v{};
};

/** @brief Builds the plane basis of a coplanar polygon from its first non-collinear triple. */
template <typename T>
GEO_NODISCARD PlaneBasis<T> plane_basis(std::span<const linal::vec3<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  GEO_ASSERT(polygon.size() >= 3);

  PlaneBasis<T> basis;
  basis.origin = polygon[0];
  const linal::vec3<T> edge = polygon[1] - polygon[0];
  basis.u = linal::normalize(edge);

  // Find the first vertex that is not collinear with the first edge to obtain a plane normal.
  linal::vec3<T> normal{};
  bool foundNormal = false;
  for (std::size_t i = 2; i < polygon.size(); ++i)
  {
    normal = linal::cross(edge, linal::vec3<T>{polygon[i] - polygon[0]});
    if (linal::length(normal) > eps)
    {
      foundNormal = true;
      break;
    }
  }
  GEO_ASSERT(foundNormal);
  (void)foundNormal;

  basis.v = linal::normalize(linal::cross(normal, basis.u));
  return basis;
}

/** @brief Projects a 3d point onto plane coordinates (du, dv) relative to a basis. */
template <typename T>
GEO_NODISCARD constexpr linal::vec2<T> to_2d(const linal::vec3<T>& point, const PlaneBasis<T>& basis) noexcept
{
  const linal::vec3<T> relative = point - basis.origin;
  return linal::vec2<T>{linal::dot(relative, basis.u), linal::dot(relative, basis.v)};
}

/** @brief Lifts plane coordinates (du, dv) back into 3d space. */
template <typename T>
GEO_NODISCARD constexpr linal::vec3<T> from_2d(const linal::vec2<T>& coord, const PlaneBasis<T>& basis) noexcept
{
  return linal::vec3<T>{basis.origin + basis.u * coord[0] + basis.v * coord[1]};
}

/** @brief Projects a coplanar 3d polygon into 2d plane coordinates. */
template <typename T>
GEO_NODISCARD std::vector<linal::vec2<T>> project(std::span<const linal::vec3<T>> polygon, const PlaneBasis<T>& basis)
{
  std::vector<linal::vec2<T>> projected;
  projected.reserve(polygon.size());
  for (const auto& point: polygon)
  {
    projected.push_back(to_2d(point, basis));
  }
  return projected;
}

/** @brief Lifts a 2d polygon back into 3d using a plane basis. */
template <typename T>
GEO_NODISCARD std::vector<linal::vec3<T>> lift(std::span<const linal::vec2<T>> polygon, const PlaneBasis<T>& basis)
{
  std::vector<linal::vec3<T>> lifted;
  lifted.reserve(polygon.size());
  for (const auto& coord: polygon)
  {
    lifted.push_back(from_2d(coord, basis));
  }
  return lifted;
}

} // namespace details

// ---------------------------------------------------------------------------------------------
// Precondition predicates for coplanar 3d polygons.
// ---------------------------------------------------------------------------------------------

/** \copydoc Geometry::has_no_repeated_vertices(std::span<const linal::vec2<T>>, T) */
template <typename T>
GEO_NODISCARD bool has_no_repeated_vertices(std::span<const linal::vec3<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  const std::size_t size = polygon.size();
  if (size < 2)
  {
    return true;
  }
  for (std::size_t i = 0; i < size; ++i)
  {
    if (linal::is_equal(polygon[i], polygon[(i + 1) % size], eps))
    {
      return false;
    }
  }
  return true;
}

/** \copydoc Geometry::is_convex(std::span<const linal::vec2<T>>, T) */
template <typename T>
GEO_NODISCARD bool is_convex(std::span<const linal::vec3<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  if (polygon.size() < 3)
  {
    return false;
  }
  const details::PlaneBasis<T> basis = details::plane_basis<T>(polygon, eps);
  const std::vector<linal::vec2<T>> projected = details::project<T>(polygon, basis);
  return is_convex<T>(std::span<const linal::vec2<T>>{projected}, eps);
}

/** \copydoc Geometry::is_counter_clockwise(std::span<const linal::vec2<T>>, T) */
template <typename T>
GEO_NODISCARD bool is_counter_clockwise(std::span<const linal::vec3<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  if (polygon.size() < 3)
  {
    return false;
  }
  const details::PlaneBasis<T> basis = details::plane_basis<T>(polygon, eps);
  const std::vector<linal::vec2<T>> projected = details::project<T>(polygon, basis);
  return is_counter_clockwise<T>(std::span<const linal::vec2<T>>{projected}, eps);
}

/** \copydoc Geometry::has_no_collinear_vertices(std::span<const linal::vec2<T>>, T) */
template <typename T>
GEO_NODISCARD bool has_no_collinear_vertices(std::span<const linal::vec3<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  if (polygon.size() < 3)
  {
    return false;
  }
  const details::PlaneBasis<T> basis = details::plane_basis<T>(polygon, eps);
  const std::vector<linal::vec2<T>> projected = details::project<T>(polygon, basis);
  return has_no_collinear_vertices<T>(std::span<const linal::vec2<T>>{projected}, eps);
}

/** \copydoc Geometry::starts_at_lowest_leftmost(std::span<const linal::vec2<T>>, T) */
template <typename T>
GEO_NODISCARD bool starts_at_lowest_leftmost(std::span<const linal::vec3<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  if (polygon.size() == 0)
  {
    return false;
  }
  const details::PlaneBasis<T> basis = details::plane_basis<T>(polygon, eps);
  const std::vector<linal::vec2<T>> projected = details::project<T>(polygon, basis);
  return starts_at_lowest_leftmost<T>(std::span<const linal::vec2<T>>{projected}, eps);
}

/** \copydoc Geometry::is_normalized_convex_polygon(std::span<const linal::vec2<T>>, T) */
template <typename T>
GEO_NODISCARD bool is_normalized_convex_polygon(std::span<const linal::vec3<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  if (polygon.size() < 3)
  {
    return false;
  }
  const details::PlaneBasis<T> basis = details::plane_basis<T>(polygon, eps);
  const std::vector<linal::vec2<T>> projected = details::project<T>(polygon, basis);
  return is_normalized_convex_polygon<T>(std::span<const linal::vec2<T>>{projected}, eps);
}

// ---------------------------------------------------------------------------------------------
// Helpers users call to conform arbitrary input.
// ---------------------------------------------------------------------------------------------

/** \copydoc Geometry::normalize_convex_polygon(std::span<const linal::vec2<T>>, T) */
template <typename T>
GEO_NODISCARD std::vector<linal::vec3<T>> normalize_convex_polygon(std::span<const linal::vec3<T>> polygon,
                                                                   T eps = linal::eps_v<T>)
{
  const details::PlaneBasis<T> basis = details::plane_basis<T>(polygon, eps);
  const std::vector<linal::vec2<T>> projected = details::project<T>(polygon, basis);
  const std::vector<linal::vec2<T>> normalized =
      normalize_convex_polygon<T>(std::span<const linal::vec2<T>>{projected}, eps);
  return details::lift<T>(std::span<const linal::vec2<T>>{normalized}, basis);
}

/** \copydoc Geometry::negate_polygon(std::span<const linal::vec2<T>>) */
template <typename T>
GEO_NODISCARD std::vector<linal::vec3<T>> negate_polygon(std::span<const linal::vec3<T>> polygon)
{
  std::vector<linal::vec3<T>> negated;
  negated.reserve(polygon.size());
  for (const auto& point: polygon)
  {
    negated.push_back(-point);
  }
  return negated;
}

/** \copydoc Geometry::negate_polygon(std::span<linal::vec2<T>>) */
template <typename T>
void negate_polygon(std::span<linal::vec3<T>> polygon) noexcept
{
  for (auto& point: polygon)
  {
    point = -point;
  }
}

// ---------------------------------------------------------------------------------------------
// Core Minkowski sum and difference for coplanar 3d polygons.
// ---------------------------------------------------------------------------------------------

/** @brief Computes the Minkowski sum of two normalized, coplanar convex polygons in 3d.
 *
 * Both inputs must be normalized convex polygons (asserted) and must be coplanar with each
 * other; @c a defines the plane. The result is a normalized convex polygon in that plane.
 *
 * @param a The first polygon, normalized. Defines the working plane.
 * @param b The second polygon, normalized and coplanar with @c a.
 * @param eps Tolerance for the edge merge and collinearity removal.
 * @return The Minkowski sum @c a ⊕ @c b, in the plane of @c a.
 */
template <typename T>
GEO_NODISCARD std::vector<linal::vec3<T>>
minkowski_sum(std::span<const linal::vec3<T>> a, std::span<const linal::vec3<T>> b, T eps = linal::eps_v<T>)
{
  GEO_ASSERT(is_normalized_convex_polygon<T>(a, eps));
  GEO_ASSERT(is_normalized_convex_polygon<T>(b, eps));

  const details::PlaneBasis<T> basis = details::plane_basis<T>(a, eps);
  const std::vector<linal::vec2<T>> projectedA = details::project<T>(a, basis);
  const std::vector<linal::vec2<T>> projectedB = details::project<T>(b, basis);

  const std::vector<linal::vec2<T>> summed = minkowski_sum<T>(std::span<const linal::vec2<T>>{projectedA},
                                                              std::span<const linal::vec2<T>>{projectedB}, eps);

  return details::lift<T>(std::span<const linal::vec2<T>>{summed}, basis);
}

/** @brief Computes the Minkowski difference of two normalized, coplanar convex polygons in 3d.
 *
 * @c a must be a normalized convex polygon (asserted) and defines the plane. @c b is negated and
 * re-normalized internally, so @c b only has to describe the same coplanar convex region.
 *
 * @param a The first polygon, normalized. Defines the working plane.
 * @param b The polygon to subtract, coplanar with @c a.
 * @param eps Tolerance for normalization and the edge merge.
 * @return The Minkowski difference @c a ⊖ @c b, in the plane of @c a.
 */
template <typename T>
GEO_NODISCARD std::vector<linal::vec3<T>>
minkowski_diff(std::span<const linal::vec3<T>> a, std::span<const linal::vec3<T>> b, T eps = linal::eps_v<T>)
{
  GEO_ASSERT(is_normalized_convex_polygon<T>(a, eps));

  // Negation flips winding, so re-normalize the reflected polygon before summing.
  std::vector<linal::vec3<T>> negated = negate_polygon<T>(b);
  const std::vector<linal::vec3<T>> negatedNormalized =
      normalize_convex_polygon<T>(std::span<const linal::vec3<T>>{negated}, eps);

  return minkowski_sum<T>(a, std::span<const linal::vec3<T>>{negatedNormalized}, eps);
}

} // namespace Geometry

#endif // GEOMETRY_MINKOWSKISUM3D_HPP
