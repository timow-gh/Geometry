#ifndef GEOMETRY_MINKOWSKISUM2D_HPP
#define GEOMETRY_MINKOWSKISUM2D_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_compare.hpp>
#include <algorithm>
#include <cmath>
#include <span>
#include <stdexcept>
#include <vector>

namespace Geometry
{

/** @file MinkowskiSum2D.hpp
 *
 * Linear-time Minkowski sum and difference of two convex polygons in 2d.
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
 *
 * The coplanar 3d variants of these functions live in Geometry/MinkowskiSum3D.hpp.
 */

namespace details
{

/** @brief The 2d scalar cross product (the z component of the 3d cross product). */
template <typename T>
GEO_NODISCARD constexpr T cross2(const linal::vec2<T>& a, const linal::vec2<T>& b) noexcept
{
  return a[0] * b[1] - a[1] * b[0];
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
template <typename T>
GEO_NODISCARD bool has_no_repeated_vertices(std::span<const linal::vec2<T>> polygon, T eps = linal::eps_v<T>) noexcept
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
template <typename T>
GEO_NODISCARD bool is_convex(std::span<const linal::vec2<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  const std::size_t size = polygon.size();
  if (size < 3)
  {
    return false;
  }

  int orientation = 0;
  for (std::size_t i = 0; i < size; ++i)
  {
    const linal::vec2<T> edge1 = polygon[(i + 1) % size] - polygon[i];
    const linal::vec2<T> edge2 = polygon[(i + 2) % size] - polygon[(i + 1) % size];

    const T turn = details::cross2<T>(edge1, edge2);
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
template <typename T>
GEO_NODISCARD bool is_counter_clockwise(std::span<const linal::vec2<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  const std::size_t size = polygon.size();
  if (size < 3)
  {
    return false;
  }

  T signedAreaTwice = static_cast<T>(0);
  for (std::size_t i = 0; i < size; ++i)
  {
    signedAreaTwice += details::cross2<T>(polygon[i], polygon[(i + 1) % size]);
  }
  return signedAreaTwice > eps;
}

/** @brief Checks that no boundary vertex is collinear with its two neighbours.
 *
 * @param polygon The polygon vertices.
 * @param eps Tolerance for the collinearity test.
 * @return True if every vertex forms an actual corner.
 */
template <typename T>
GEO_NODISCARD bool has_no_collinear_vertices(std::span<const linal::vec2<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  const std::size_t size = polygon.size();
  if (size < 3)
  {
    return false;
  }

  for (std::size_t i = 0; i < size; ++i)
  {
    const linal::vec2<T> edge1 = polygon[i] - polygon[(i + size - 1) % size];
    const linal::vec2<T> edge2 = polygon[(i + 1) % size] - polygon[i];
    if (std::abs(details::cross2<T>(edge1, edge2)) < eps)
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
template <typename T>
GEO_NODISCARD bool starts_at_lowest_leftmost(std::span<const linal::vec2<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  (void)eps;
  const std::size_t size = polygon.size();
  if (size == 0)
  {
    return false;
  }

  const linal::vec2<T>& first = polygon[0];
  for (std::size_t i = 1; i < size; ++i)
  {
    const linal::vec2<T>& candidate = polygon[i];
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
template <typename T>
GEO_NODISCARD bool is_normalized_convex_polygon(std::span<const linal::vec2<T>> polygon, T eps = linal::eps_v<T>) noexcept
{
  return polygon.size() >= 3 && has_no_repeated_vertices<T>(polygon, eps) && is_counter_clockwise<T>(polygon, eps) &&
         is_convex<T>(polygon, eps) && has_no_collinear_vertices<T>(polygon, eps) &&
         starts_at_lowest_leftmost<T>(polygon, eps);
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
template <typename T>
GEO_NODISCARD std::vector<linal::vec2<T>> normalize_convex_polygon(std::span<const linal::vec2<T>> polygon,
                                                                   T eps = linal::eps_v<T>)
{
  std::vector<linal::vec2<T>> normalized;
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

  T signedAreaTwice = static_cast<T>(0);
  for (std::size_t i = 0; i < normalized.size(); ++i)
  {
    signedAreaTwice += details::cross2<T>(normalized[i], normalized[(i + 1) % normalized.size()]);
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
    const linal::vec2<T> edge1 = normalized[(i + 1) % normalized.size()] - normalized[i];
    const linal::vec2<T> edge2 = normalized[(i + 2) % normalized.size()] - normalized[(i + 1) % normalized.size()];
    if (details::cross2<T>(edge1, edge2) < -eps)
    {
      throw std::invalid_argument("Minkowski sum requires convex polygons");
    }
  }

  std::vector<linal::vec2<T>> withoutCollinear;
  withoutCollinear.reserve(normalized.size());
  for (std::size_t i = 0; i < normalized.size(); ++i)
  {
    const linal::vec2<T> edge1 = normalized[i] - normalized[(i + normalized.size() - 1) % normalized.size()];
    const linal::vec2<T> edge2 = normalized[(i + 1) % normalized.size()] - normalized[i];
    if (std::abs(details::cross2<T>(edge1, edge2)) >= eps)
    {
      withoutCollinear.push_back(normalized[i]);
    }
  }

  if (withoutCollinear.size() < 3)
  {
    throw std::invalid_argument("A Minkowski-sum polygon must have non-zero area");
  }

  const auto lowestLeftmost = std::min_element(withoutCollinear.begin(), withoutCollinear.end(),
                                               [](const linal::vec2<T>& lhs, const linal::vec2<T>& rhs)
                                               { return lhs[1] < rhs[1] || (lhs[1] == rhs[1] && lhs[0] < rhs[0]); });
  std::rotate(withoutCollinear.begin(), lowestLeftmost, withoutCollinear.end());
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
template <typename T>
GEO_NODISCARD std::vector<linal::vec2<T>> negate_polygon(std::span<const linal::vec2<T>> polygon)
{
  std::vector<linal::vec2<T>> negated;
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
template <typename T>
void negate_polygon(std::span<linal::vec2<T>> polygon) noexcept
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
 * @param a The first polygon, normalized.
 * @param b The second polygon, normalized.
 * @param eps Tolerance for the edge merge and collinearity removal.
 * @return The Minkowski sum @c a ⊕ @c b.
 */
template <typename T>
GEO_NODISCARD std::vector<linal::vec2<T>>
minkowski_sum(std::span<const linal::vec2<T>> a, std::span<const linal::vec2<T>> b, T eps = linal::eps_v<T>)
{
  GEO_ASSERT(is_normalized_convex_polygon<T>(a, eps));
  GEO_ASSERT(is_normalized_convex_polygon<T>(b, eps));

  std::vector<linal::vec2<T>> merged;
  merged.reserve(a.size() + b.size());

  std::size_t i = 0;
  std::size_t j = 0;
  linal::vec2<T> current = a[0] + b[0];
  merged.push_back(current);

  while (i < a.size() || j < b.size())
  {
    linal::vec2<T> edge{};
    if (i == a.size())
    {
      edge = b[(j + 1) % b.size()] - b[j];
      ++j;
    }
    else if (j == b.size())
    {
      edge = a[(i + 1) % a.size()] - a[i];
      ++i;
    }
    else
    {
      const linal::vec2<T> edgeA = a[(i + 1) % a.size()] - a[i];
      const linal::vec2<T> edgeB = b[(j + 1) % b.size()] - b[j];
      const T turn = details::cross2<T>(edgeA, edgeB);
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

  // Remove collinear vertices.
  std::vector<linal::vec2<T>> result;
  result.reserve(merged.size());
  for (std::size_t k = 0; k < merged.size(); ++k)
  {
    const linal::vec2<T> edge1 = merged[k] - merged[(k + merged.size() - 1) % merged.size()];
    const linal::vec2<T> edge2 = merged[(k + 1) % merged.size()] - merged[k];
    if (std::abs(details::cross2<T>(edge1, edge2)) >= eps)
    {
      result.push_back(merged[k]);
    }
  }

  GEO_ASSERT(is_normalized_convex_polygon<T>(std::span<const linal::vec2<T>>{result}, eps));
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
template <typename T>
GEO_NODISCARD std::vector<linal::vec2<T>>
minkowski_diff(std::span<const linal::vec2<T>> a, std::span<const linal::vec2<T>> b, T eps = linal::eps_v<T>)
{
  GEO_ASSERT(is_normalized_convex_polygon<T>(a, eps));

  // Negation flips winding, so re-normalize the reflected polygon before summing.
  std::vector<linal::vec2<T>> negated = negate_polygon<T>(b);
  const std::vector<linal::vec2<T>> negatedNormalized =
      normalize_convex_polygon<T>(std::span<const linal::vec2<T>>{negated}, eps);

  return minkowski_sum<T>(a, std::span<const linal::vec2<T>>{negatedNormalized}, eps);
}

} // namespace Geometry

#endif // GEOMETRY_MINKOWSKISUM2D_HPP
