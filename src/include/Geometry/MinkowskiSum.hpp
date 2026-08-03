#ifndef GEOMETRY_MINKOWSKISUM_HPP
#define GEOMETRY_MINKOWSKISUM_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <algorithm>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace Geometry
{

/** @file MinkowskiSum.hpp
 *
 * Linear-time Minkowski sum and difference of two convex polygons.
 *
 * The functions are templated on the scalar type @c T and the dimension @c D and work for
 * @c D == 2 as well as for @c D == 3, where the @c D == 3 case treats the input as coplanar
 * polygons that live in a common plane in 3d space. General 3d polyhedra are not supported.
 *
 * The core functions minkowski_sum() and minkowski_diff() assume that their input already is a
 * "normalized" convex polygon and only verify this via GEO_ASSERT (compiled out in release
 * builds). Callers turn arbitrary vertex ranges into normalized polygons with
 * normalize_convex_polygon() and can cheaply check conformance with
 * is_normalized_convex_polygon() and the granular predicates, none of which allocate or copy.
 *
 * A normalized convex polygon fulfils all of the following:
 * - at least three vertices and a non-zero area,
 * - no consecutive repeated vertices and no repeated closing vertex,
 * - counterclockwise winding,
 * - convex, with no collinear boundary vertices, and
 * - the vertex sequence starts at the lowest, then leftmost, vertex.
 */

namespace details
{

/** @brief An orthonormal basis of the plane a polygon lives in.
 *
 * @c origin is a point of the plane, @c u and @c v span it. For @c D == 2 the basis is the
 * identity so that projecting to plane coordinates is a zero-overhead pass-through.
 */
template <typename T, std::uint8_t D>
struct PlaneBasis
{
  linal::vec<T, D> origin{};
  linal::vec<T, D> u{};
  linal::vec<T, D> v{};
};

/** @brief The 2d scalar cross product (the z component of the 3d cross product). */
template <typename T>
GEO_NODISCARD constexpr T cross2(T ax, T ay, T bx, T by) noexcept
{
  return ax * by - ay * bx;
}

/** @brief Builds the plane basis of a polygon.
 *
 * For @c D == 2 the identity basis is returned. For @c D == 3 an orthonormal basis of the
 * polygon's plane is constructed from the first non-collinear vertex triple.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD PlaneBasis<T, D> plane_basis(std::span<const linal::vec<T, D>> polygon, T eps = linal::eps_v<T>) noexcept
{
  PlaneBasis<T, D> basis;
  if constexpr (D == 2)
  {
    basis.origin = linal::vec<T, D>{static_cast<T>(0), static_cast<T>(0)};
    basis.u = linal::vec<T, D>{static_cast<T>(1), static_cast<T>(0)};
    basis.v = linal::vec<T, D>{static_cast<T>(0), static_cast<T>(1)};
  }
  else
  {
    GEO_ASSERT(polygon.size() >= 3);
    basis.origin = polygon[0];
    const linal::vec<T, D> edge = polygon[1] - polygon[0];
    basis.u = linal::normalize(edge);

    // Find the first vertex that is not collinear with the first edge to obtain a plane normal.
    linal::vec<T, D> normal{};
    bool foundNormal = false;
    for (std::size_t i = 2; i < polygon.size(); ++i)
    {
      normal = linal::cross(edge, linal::vec<T, D>{polygon[i] - polygon[0]});
      if (linal::length(normal) > eps)
      {
        foundNormal = true;
        break;
      }
    }
    GEO_ASSERT(foundNormal);
    (void)foundNormal;

    basis.v = linal::normalize(linal::cross(normal, basis.u));
  }
  return basis;
}

/** @brief Projects a point onto plane coordinates (du, dv) relative to a basis.
 *
 * For @c D == 2 this returns the point's own coordinates.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr linal::vec<T, 2> to_2d(const linal::vec<T, D>& point, const PlaneBasis<T, D>& basis) noexcept
{
  if constexpr (D == 2)
  {
    return point;
  }
  else
  {
    const linal::vec<T, D> relative = point - basis.origin;
    return linal::vec<T, 2>{linal::dot(relative, basis.u), linal::dot(relative, basis.v)};
  }
}

/** @brief Lifts plane coordinates (du, dv) back into @c D dimensional space. */
template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr linal::vec<T, D> from_2d(const linal::vec<T, 2>& coord, const PlaneBasis<T, D>& basis) noexcept
{
  if constexpr (D == 2)
  {
    return coord;
  }
  else
  {
    return linal::vec<T, D>{basis.origin + basis.u * coord[0] + basis.v * coord[1]};
  }
}

} // namespace details

// ---------------------------------------------------------------------------------------------
// Precondition predicates. All take a span, allocate nothing and never copy the input.
// ---------------------------------------------------------------------------------------------

/** @brief Checks that no vertex equals its successor and that the polygon is not closed.
 *
 * @param polygon The polygon vertices.
 * @param eps Tolerance for the vertex comparison.
 * @return True if there are no consecutive repeated vertices and no repeated closing vertex.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool has_no_repeated_vertices(std::span<const linal::vec<T, D>> polygon, T eps = linal::eps_v<T>) noexcept
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

/** @brief Checks that the polygon is convex, allowing collinear boundary vertices.
 *
 * @param polygon The polygon vertices.
 * @param eps Tolerance for the turn direction test.
 * @return True if all boundary turns share the same (counterclockwise or clockwise) sense.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool is_convex(std::span<const linal::vec<T, D>> polygon, T eps = linal::eps_v<T>) noexcept
{
  const std::size_t size = polygon.size();
  if (size < 3)
  {
    return false;
  }

  const details::PlaneBasis<T, D> basis = details::plane_basis<T, D>(polygon, eps);

  int orientation = 0;
  for (std::size_t i = 0; i < size; ++i)
  {
    const linal::vec<T, 2> p0 = details::to_2d(polygon[i], basis);
    const linal::vec<T, 2> p1 = details::to_2d(polygon[(i + 1) % size], basis);
    const linal::vec<T, 2> p2 = details::to_2d(polygon[(i + 2) % size], basis);

    const T turn = details::cross2<T>(p1[0] - p0[0], p1[1] - p0[1], p2[0] - p1[0], p2[1] - p1[1]);
    if (std::abs(turn) < eps)
    {
      continue; // Collinear vertices are allowed.
    }

    const int currentOrientation = turn > static_cast<T>(0) ? 1 : -1;
    if (orientation == 0)
    {
      orientation = currentOrientation;
    }
    else if (currentOrientation != orientation)
    {
      return false;
    }
  }
  return orientation != 0; // A non-zero orientation implies a non-zero area.
}

/** @brief Checks that the polygon vertices wind counterclockwise (positive signed area).
 *
 * @param polygon The polygon vertices.
 * @param eps Tolerance for the area sign test.
 * @return True if the signed area is positive.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool is_counter_clockwise(std::span<const linal::vec<T, D>> polygon, T eps = linal::eps_v<T>) noexcept
{
  const std::size_t size = polygon.size();
  if (size < 3)
  {
    return false;
  }

  const details::PlaneBasis<T, D> basis = details::plane_basis<T, D>(polygon, eps);

  T signedAreaTwice = static_cast<T>(0);
  for (std::size_t i = 0; i < size; ++i)
  {
    const linal::vec<T, 2> current = details::to_2d(polygon[i], basis);
    const linal::vec<T, 2> next = details::to_2d(polygon[(i + 1) % size], basis);
    signedAreaTwice += details::cross2<T>(current[0], current[1], next[0], next[1]);
  }
  return signedAreaTwice > eps;
}

/** @brief Checks that no boundary vertex is collinear with its two neighbours.
 *
 * @param polygon The polygon vertices.
 * @param eps Tolerance for the collinearity test.
 * @return True if every vertex forms an actual corner.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool has_no_collinear_vertices(std::span<const linal::vec<T, D>> polygon, T eps = linal::eps_v<T>) noexcept
{
  const std::size_t size = polygon.size();
  if (size < 3)
  {
    return false;
  }

  const details::PlaneBasis<T, D> basis = details::plane_basis<T, D>(polygon, eps);

  for (std::size_t i = 0; i < size; ++i)
  {
    const linal::vec<T, 2> previous = details::to_2d(polygon[(i + size - 1) % size], basis);
    const linal::vec<T, 2> current = details::to_2d(polygon[i], basis);
    const linal::vec<T, 2> next = details::to_2d(polygon[(i + 1) % size], basis);

    const T turn = details::cross2<T>(current[0] - previous[0], current[1] - previous[1], next[0] - current[0],
                                      next[1] - current[1]);
    if (std::abs(turn) < eps)
    {
      return false;
    }
  }
  return true;
}

/** @brief Checks that the vertex sequence starts at the lowest, then leftmost, vertex.
 *
 * @param polygon The polygon vertices.
 * @param eps Unused, present for a uniform predicate signature.
 * @return True if @c polygon[0] is the lowest, then leftmost, vertex.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool starts_at_lowest_leftmost(std::span<const linal::vec<T, D>> polygon, T eps = linal::eps_v<T>) noexcept
{
  (void)eps;
  const std::size_t size = polygon.size();
  if (size == 0)
  {
    return false;
  }

  const details::PlaneBasis<T, D> basis = details::plane_basis<T, D>(polygon, eps);
  const linal::vec<T, 2> first = details::to_2d(polygon[0], basis);
  for (std::size_t i = 1; i < size; ++i)
  {
    const linal::vec<T, 2> candidate = details::to_2d(polygon[i], basis);
    if (candidate[1] < first[1] || (candidate[1] == first[1] && candidate[0] < first[0]))
    {
      return false;
    }
  }
  return true;
}

/** @brief Checks that the polygon is in the normalized form the Minkowski functions expect.
 *
 * This composes has_no_repeated_vertices(), is_counter_clockwise(), is_convex(),
 * has_no_collinear_vertices() and starts_at_lowest_leftmost(). It is exactly the precondition
 * that minkowski_sum() and minkowski_diff() assert.
 *
 * @param polygon The polygon vertices.
 * @param eps Tolerance shared by the composed checks.
 * @return True if the polygon is a normalized convex polygon.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool is_normalized_convex_polygon(std::span<const linal::vec<T, D>> polygon, T eps = linal::eps_v<T>) noexcept
{
  return polygon.size() >= 3 && has_no_repeated_vertices<T, D>(polygon, eps) &&
         is_counter_clockwise<T, D>(polygon, eps) && is_convex<T, D>(polygon, eps) &&
         has_no_collinear_vertices<T, D>(polygon, eps) && starts_at_lowest_leftmost<T, D>(polygon, eps);
}

// ---------------------------------------------------------------------------------------------
// Helpers users call to conform arbitrary input.
// ---------------------------------------------------------------------------------------------

/** @brief Converts a convex polygon to the normalized representation.
 *
 * The represented geometric region does not change; only redundant vertices, winding and the
 * starting vertex do. Normalization removes consecutive duplicate vertices and an optional
 * repeated closing vertex, requires at least three distinct vertices and a non-zero area,
 * reverses clockwise input to counterclockwise, verifies convexity, removes collinear boundary
 * vertices, and rotates the sequence so the lowest, then leftmost, vertex is first.
 *
 * @param polygon The polygon vertices. The input is never modified.
 * @param eps Tolerance for the area, convexity and collinearity tests.
 * @return The normalized polygon.
 *
 * @throws std::invalid_argument if the input is degenerate or non-convex.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD std::vector<linal::vec<T, D>> normalize_convex_polygon(std::span<const linal::vec<T, D>> polygon,
                                                                     T eps = linal::eps_v<T>)
{
  std::vector<linal::vec<T, D>> normalized;
  normalized.reserve(polygon.size());
  for (const auto& point: polygon)
  {
    if (normalized.empty() || !linal::is_equal(point, normalized.back(), eps))
    {
      normalized.push_back(point);
    }
  }
  if (normalized.size() > 1 && linal::is_equal(normalized.front(), normalized.back(), eps))
  {
    normalized.pop_back();
  }

  if (normalized.size() < 3)
  {
    throw std::invalid_argument("A Minkowski-sum polygon must have at least three distinct vertices");
  }

  const details::PlaneBasis<T, D> basis = details::plane_basis<T, D>(std::span<const linal::vec<T, D>>{normalized}, eps);

  T signedAreaTwice = static_cast<T>(0);
  for (std::size_t i = 0; i < normalized.size(); ++i)
  {
    const linal::vec<T, 2> current = details::to_2d(normalized[i], basis);
    const linal::vec<T, 2> next = details::to_2d(normalized[(i + 1) % normalized.size()], basis);
    signedAreaTwice += details::cross2<T>(current[0], current[1], next[0], next[1]);
  }

  if (std::abs(signedAreaTwice) <= eps)
  {
    throw std::invalid_argument("A Minkowski-sum polygon must have non-zero area");
  }
  if (signedAreaTwice < static_cast<T>(0))
  {
    std::reverse(normalized.begin(), normalized.end());
  }

  for (std::size_t i = 0; i < normalized.size(); ++i)
  {
    const linal::vec<T, 2> p0 = details::to_2d(normalized[i], basis);
    const linal::vec<T, 2> p1 = details::to_2d(normalized[(i + 1) % normalized.size()], basis);
    const linal::vec<T, 2> p2 = details::to_2d(normalized[(i + 2) % normalized.size()], basis);
    const T turn = details::cross2<T>(p1[0] - p0[0], p1[1] - p0[1], p2[0] - p1[0], p2[1] - p1[1]);
    if (turn < -eps)
    {
      throw std::invalid_argument("Minkowski sum requires convex polygons");
    }
  }

  std::vector<linal::vec<T, D>> withoutCollinear;
  withoutCollinear.reserve(normalized.size());
  for (std::size_t i = 0; i < normalized.size(); ++i)
  {
    const linal::vec<T, 2> previous = details::to_2d(normalized[(i + normalized.size() - 1) % normalized.size()], basis);
    const linal::vec<T, 2> current = details::to_2d(normalized[i], basis);
    const linal::vec<T, 2> next = details::to_2d(normalized[(i + 1) % normalized.size()], basis);
    const T turn = details::cross2<T>(current[0] - previous[0], current[1] - previous[1], next[0] - current[0],
                                      next[1] - current[1]);
    if (std::abs(turn) >= eps)
    {
      withoutCollinear.push_back(normalized[i]);
    }
  }

  if (withoutCollinear.size() < 3)
  {
    throw std::invalid_argument("A Minkowski-sum polygon must have non-zero area");
  }

  // Rotate so that the lowest, then leftmost, vertex (in plane coordinates) is first.
  std::size_t lowestLeftmost = 0;
  linal::vec<T, 2> best = details::to_2d(withoutCollinear[0], basis);
  for (std::size_t i = 1; i < withoutCollinear.size(); ++i)
  {
    const linal::vec<T, 2> candidate = details::to_2d(withoutCollinear[i], basis);
    if (candidate[1] < best[1] || (candidate[1] == best[1] && candidate[0] < best[0]))
    {
      best = candidate;
      lowestLeftmost = i;
    }
  }
  std::rotate(withoutCollinear.begin(), withoutCollinear.begin() + static_cast<std::ptrdiff_t>(lowestLeftmost),
              withoutCollinear.end());
  return withoutCollinear;
}

/** @brief Returns the polygon reflected through the origin.
 *
 * Reflecting a polygon through the origin flips its winding order, so the result usually has to
 * be re-normalized with normalize_convex_polygon() before it is passed to minkowski_sum().
 *
 * @param polygon The polygon vertices. The input is never modified.
 * @return The point-negated polygon.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD std::vector<linal::vec<T, D>> negate_polygon(std::span<const linal::vec<T, D>> polygon)
{
  std::vector<linal::vec<T, D>> negated;
  negated.reserve(polygon.size());
  for (const auto& point: polygon)
  {
    negated.push_back(-point);
  }
  return negated;
}

/** @brief Reflects a polygon through the origin in place.
 *
 * @param polygon The polygon vertices, negated in place. No allocation occurs.
 */
template <typename T, std::uint8_t D>
void negate_polygon(std::span<linal::vec<T, D>> polygon) noexcept
{
  for (auto& point: polygon)
  {
    point = -point;
  }
}

// ---------------------------------------------------------------------------------------------
// Core Minkowski sum and difference.
// ---------------------------------------------------------------------------------------------

/** @brief Computes the Minkowski sum of two normalized convex polygons.
 *
 * Both inputs must be normalized convex polygons (see is_normalized_convex_polygon()); this is
 * asserted, not validated. The result is itself a normalized convex polygon.
 *
 * For @c D == 3 both polygons are assumed to be coplanar and to share the plane spanned by @c a.
 *
 * @param a The first polygon, normalized.
 * @param b The second polygon, normalized.
 * @param eps Tolerance for the edge merge and collinearity removal.
 * @return The Minkowski sum @c a ⊕ @c b.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD std::vector<linal::vec<T, D>>
minkowski_sum(std::span<const linal::vec<T, D>> a, std::span<const linal::vec<T, D>> b, T eps = linal::eps_v<T>)
{
  GEO_ASSERT(is_normalized_convex_polygon<T, D>(a, eps));
  GEO_ASSERT(is_normalized_convex_polygon<T, D>(b, eps));

  // Project both polygons into a common plane basis. For D == 3 both must lie in a's plane.
  const details::PlaneBasis<T, D> basis = details::plane_basis<T, D>(a, eps);

  std::vector<linal::vec<T, 2>> polyA;
  std::vector<linal::vec<T, 2>> polyB;
  polyA.reserve(a.size());
  polyB.reserve(b.size());
  for (const auto& point: a)
  {
    polyA.push_back(details::to_2d(point, basis));
  }
  for (const auto& point: b)
  {
    polyB.push_back(details::to_2d(point, basis));
  }

  std::vector<linal::vec<T, 2>> merged;
  merged.reserve(polyA.size() + polyB.size());

  std::size_t i = 0;
  std::size_t j = 0;
  linal::vec<T, 2> current = polyA[0] + polyB[0];
  merged.push_back(current);

  while (i < polyA.size() || j < polyB.size())
  {
    linal::vec<T, 2> edge{};
    if (i == polyA.size())
    {
      edge = polyB[(j + 1) % polyB.size()] - polyB[j];
      ++j;
    }
    else if (j == polyB.size())
    {
      edge = polyA[(i + 1) % polyA.size()] - polyA[i];
      ++i;
    }
    else
    {
      const linal::vec<T, 2> edgeA = polyA[(i + 1) % polyA.size()] - polyA[i];
      const linal::vec<T, 2> edgeB = polyB[(j + 1) % polyB.size()] - polyB[j];
      const T turn = details::cross2<T>(edgeA[0], edgeA[1], edgeB[0], edgeB[1]);
      if (turn > eps)
      {
        edge = edgeA;
        ++i;
      }
      else if (turn < -eps)
      {
        edge = edgeB;
        ++j;
      }
      else
      {
        edge = edgeA + edgeB; // Parallel edges with the same angular position.
        ++i;
        ++j;
      }
    }

    current = current + edge;
    merged.push_back(current);
  }

  // The last point normally equals the first point.
  merged.pop_back();

  // Remove collinear vertices, then lift the result back into D dimensions.
  std::vector<linal::vec<T, D>> result;
  result.reserve(merged.size());
  for (std::size_t k = 0; k < merged.size(); ++k)
  {
    const linal::vec<T, 2> previous = merged[(k + merged.size() - 1) % merged.size()];
    const linal::vec<T, 2> currentPoint = merged[k];
    const linal::vec<T, 2> next = merged[(k + 1) % merged.size()];
    const T turn = details::cross2<T>(currentPoint[0] - previous[0], currentPoint[1] - previous[1],
                                      next[0] - currentPoint[0], next[1] - currentPoint[1]);
    if (std::abs(turn) >= eps)
    {
      result.push_back(details::from_2d(currentPoint, basis));
    }
  }

  GEO_ASSERT(is_normalized_convex_polygon<T, D>(std::span<const linal::vec<T, D>>{result}, eps));
  return result;
}

/** @brief Computes the Minkowski difference of two normalized convex polygons.
 *
 * The Minkowski difference is the Minkowski sum of @c a with @c b reflected through the origin.
 * @c a must be a normalized convex polygon (asserted). @c b is negated and re-normalized
 * internally, so @c b only has to describe the same convex region.
 *
 * @param a The first polygon, normalized.
 * @param b The polygon to subtract.
 * @param eps Tolerance for normalization and the edge merge.
 * @return The Minkowski difference @c a ⊖ @c b.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD std::vector<linal::vec<T, D>>
minkowski_diff(std::span<const linal::vec<T, D>> a, std::span<const linal::vec<T, D>> b, T eps = linal::eps_v<T>)
{
  GEO_ASSERT(is_normalized_convex_polygon<T, D>(a, eps));

  // Negation flips winding, so re-normalize the reflected polygon before summing.
  std::vector<linal::vec<T, D>> negated = negate_polygon<T, D>(b);
  const std::vector<linal::vec<T, D>> negatedNormalized =
      normalize_convex_polygon<T, D>(std::span<const linal::vec<T, D>>{negated}, eps);

  return minkowski_sum<T, D>(a, std::span<const linal::vec<T, D>>{negatedNormalized}, eps);
}

} // namespace Geometry

#endif // GEOMETRY_MINKOWSKISUM_HPP
