#ifndef GEOMETRY_POLYGONTRIANGULATION_HPP
#define GEOMETRY_POLYGONTRIANGULATION_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>

namespace Geometry
{

// Corner indices (first, apex, last) of one triangle of a polygon triangulation, first < apex < last.
using PolygonTriangle = std::array<std::size_t, 3>;

/**
 * \brief One entry of the scratch table \c minimum_cost_triangulation works in: the best cost of a
 * sub-polygon and the apex of the triangle on its closing side.
 *
 * Exposed so callers can own the table and choose where it lives, e.g. a \c std::array on the stack
 * for small polygons or a reused \c std::vector for large ones.
 */
template <typename TCost>
struct TriangulationCell
{
  TCost cost{};
  std::size_t apex{0};
};

// Number of \c TriangulationCell entries \c minimum_cost_triangulation needs for \p cornerCount corners.
GEO_NODISCARD constexpr std::size_t triangulation_scratch_size(std::size_t cornerCount) noexcept
{
  return cornerCount < 2 ? 0 : cornerCount * (cornerCount - 1) / 2;
}

// Number of triangles in any triangulation of a polygon with \p cornerCount corners.
GEO_NODISCARD constexpr std::size_t triangulation_triangle_count(std::size_t cornerCount) noexcept
{
  return cornerCount < 3 ? 0 : cornerCount - 2;
}

/**
 * \brief What a triangulation objective provides: how triangle costs add up to the cost of a
 * triangulation, which value means "forbidden", and what a polygon side contributes.
 *
 * The search is only exact if \c combine never decreases when either argument grows and a forbidden
 * argument makes the result forbidden, since it discards every sub-polygon result except the best.
 * Costs are compared with \c operator<, so a \c std::pair cost with a combine that works element-wise
 * orders triangulations first by one criterion, then by another.
 */
template <typename TObjective, typename TCost>
concept TriangulationObjective =
    std::totally_ordered<TCost> && requires(const TObjective& objective, const TCost& lhs, const TCost& rhs) {
      { objective.identity() } -> std::convertible_to<TCost>;
      { objective.forbidden() } -> std::convertible_to<TCost>;
      { objective.combine(lhs, rhs) } -> std::convertible_to<TCost>;
    };

/**
 * \brief Minimizes the sum of the triangle costs; the default objective.
 *
 * Every triangle counts, so a large improvement in one triangle can outweigh slightly worse ones
 * elsewhere.
 */
template <std::floating_point TCost>
struct SumObjective
{
  GEO_NODISCARD static constexpr TCost identity() noexcept { return TCost{0}; }
  GEO_NODISCARD static constexpr TCost forbidden() noexcept { return std::numeric_limits<TCost>::infinity(); }
  GEO_NODISCARD static constexpr TCost combine(TCost lhs, TCost rhs) noexcept { return lhs + rhs; }
};

/**
 * \brief Minimizes the largest triangle cost, i.e. makes the worst triangle as good as possible.
 *
 * Use when a single bad triangle is what hurts (e.g. a sliver breaking a later simulation). Many
 * triangulations can share the same worst triangle, so ties are common and the others are left
 * unoptimized.
 */
template <std::floating_point TCost>
struct MaxObjective
{
  GEO_NODISCARD static constexpr TCost identity() noexcept { return -std::numeric_limits<TCost>::infinity(); }
  GEO_NODISCARD static constexpr TCost forbidden() noexcept { return std::numeric_limits<TCost>::infinity(); }
  GEO_NODISCARD static constexpr TCost combine(TCost lhs, TCost rhs) noexcept { return std::max(lhs, rhs); }
};

// Diagonal predicate allowing every diagonal, the default of \c minimum_cost_triangulation.
struct AllowAllDiagonals
{
  GEO_NODISCARD constexpr bool operator()(std::size_t /*first*/, std::size_t /*last*/) const noexcept { return true; }
};

namespace detail
{

template <typename TCostFunction>
using triangulation_cost_t =
    std::remove_cvref_t<std::invoke_result_t<const TCostFunction&, std::size_t, std::size_t, std::size_t>>;

// Only sub-polygons (first, last) with first < last exist, so the table is packed as a triangle.
GEO_NODISCARD constexpr std::size_t triangulation_slot(std::size_t first, std::size_t last) noexcept
{
  return last * (last - 1) / 2 + first;
}

template <typename TCostFunction, typename TDiagonalAllowed, typename TObjective>
inline constexpr bool is_nothrow_triangulation_v = [] {
  using TCost = triangulation_cost_t<TCostFunction>;
  return std::is_nothrow_invocable_v<const TCostFunction&, std::size_t, std::size_t, std::size_t>
         && std::is_nothrow_invocable_v<const TDiagonalAllowed&, std::size_t, std::size_t>
         && noexcept(std::declval<const TObjective&>().combine(std::declval<const TCost&>(), std::declval<const TCost&>()))
         && noexcept(std::declval<const TCost&>() < std::declval<const TCost&>())
         && std::is_nothrow_copy_assignable_v<TCost>;
}();

} // namespace detail

/**
 * \brief Finds the cheapest triangulation of a polygon, given only a cost per candidate triangle.
 *
 * The polygon is described purely by its corners 0 .. \p cornerCount - 1 in boundary order; the
 * algorithm never sees coordinates, so it works equally for planar polygons, holes in 3D surfaces,
 * or anything else a cost can be defined on. The sides (i, i + 1) and the closing side
 * (0, \p cornerCount - 1) are always part of the result; every other pair (i, j) is a candidate
 * diagonal. All triangulations are searched exactly by dynamic programming, so a triangulation is
 * found whenever one that is not forbidden exists.
 *
 * Getting good results:
 * - Put constraints on a single edge into \p isDiagonalAllowed (e.g. "this edge already exists in
 *   my mesh", which would otherwise duplicate it) and everything else into \p cost, returning
 *   \c objective.forbidden() (infinity for the built-in objectives) for triangles to reject.
 * - Reject triangles whose orientation disagrees with the polygon's. For a simple planar polygon
 *   this alone keeps every triangle inside it, so diagonals leaving a concave polygon need no
 *   separate test, and the result never overlaps itself. The cost functions in
 *   \c PolygonTriangulationCosts.hpp do this.
 * - Choose the cost for the goal: \c AspectRatioCost avoids slivers and is a good default,
 *   \c AreaCost gives a minimal surface for filling holes in 3D, \c EdgeLengthCost gives the
 *   shortest diagonals. Smoothly filling a hole in a mesh needs the neighbouring faces, so write a
 *   cost that also penalizes the angle to them.
 * - Pass \c MaxObjective to optimize the worst triangle instead of the sum.
 *
 * The caller provides all memory, so nothing is allocated; size it with
 * \c triangulation_scratch_size and \c triangulation_triangle_count:
 * \code
 * std::array<TriangulationCell<double>, triangulation_scratch_size(8)> scratch;  // on the stack
 * std::array<PolygonTriangle, triangulation_triangle_count(8)> triangles;
 * const bool found = minimum_cost_triangulation(8, AspectRatioCost<double, 2>{corners}, scratch, triangles);
 * \endcode
 * With a \c std::vector of each instead, the same buffers can be reused for many polygons.
 *
 * O(n^3) time, O(n^2) scratch for n corners.
 *
 * \param cornerCount Number of polygon corners, at least 3.
 * \param cost \c cost(first, apex, last) with first < apex < last prices the triangle on those
 * corners, or returns \c objective.forbidden() to reject it. Called at most once per triangle.
 * \param scratch At least \c triangulation_scratch_size(cornerCount) cells, overwritten.
 * \param triangles At least \c triangulation_triangle_count(cornerCount) entries; on success the first
 * that many hold the triangles as (first, apex, last), otherwise their content is unspecified.
 * \param isDiagonalAllowed \c isDiagonalAllowed(first, last) with first < last vetoes a diagonal;
 * never asked about a side. Allows every diagonal by default.
 * \param objective How triangle costs combine; minimizes their sum by default.
 * \return \c true if a triangulation was found, \c false if every one is forbidden.
 */
template <typename TCostFunction, typename TDiagonalAllowed = AllowAllDiagonals,
          typename TObjective = SumObjective<detail::triangulation_cost_t<TCostFunction>>>
  requires std::predicate<const TDiagonalAllowed&, std::size_t, std::size_t>
           && TriangulationObjective<TObjective, detail::triangulation_cost_t<TCostFunction>>
GEO_NODISCARD constexpr bool minimum_cost_triangulation(std::size_t cornerCount, const TCostFunction& cost,
                                                        std::span<TriangulationCell<detail::triangulation_cost_t<TCostFunction>>> scratch,
                                                        std::span<PolygonTriangle> triangles,
                                                        const TDiagonalAllowed& isDiagonalAllowed = {},
                                                        const TObjective& objective = {})
    noexcept(detail::is_nothrow_triangulation_v<TCostFunction, TDiagonalAllowed, TObjective>)
{
  using TCost = detail::triangulation_cost_t<TCostFunction>;
  using Cell = TriangulationCell<TCost>;

  GEO_ASSERT(cornerCount >= 3);
  GEO_ASSERT(scratch.size() >= triangulation_scratch_size(cornerCount));
  GEO_ASSERT(triangles.size() >= triangulation_triangle_count(cornerCount));

  const TCost forbidden = objective.forbidden();
  const auto cell = [scratch](std::size_t first, std::size_t last) -> Cell& {
    return scratch[detail::triangulation_slot(first, last)];
  };

  for (std::size_t first = 0; first + 1 < cornerCount; ++first)
  {
    cell(first, first + 1) = Cell{objective.identity(), first};
  }

  for (std::size_t length = 2; length < cornerCount; ++length)
  {
    for (std::size_t first = 0; first + length < cornerCount; ++first)
    {
      const std::size_t last = first + length;
      Cell& best = cell(first, last);
      best = Cell{forbidden, first};
      const bool isClosingSide = first == 0 && last == cornerCount - 1;
      if (!isClosingSide && !isDiagonalAllowed(first, last))
      {
        continue;
      }
      for (std::size_t apex = first + 1; apex < last; ++apex)
      {
        const TCost& before = cell(first, apex).cost;
        const TCost& after = cell(apex, last).cost;
        // A forbidden part keeps the whole forbidden, so skip the cost call, which may be expensive.
        if (!(before < forbidden) || !(after < forbidden))
        {
          continue;
        }
        const TCost total = objective.combine(objective.combine(before, after), cost(first, apex, last));
        if (total < best.cost)
        {
          best = Cell{total, apex};
        }
      }
    }
  }

  if (!(cell(0, cornerCount - 1).cost < forbidden))
  {
    return false;
  }

  // The output doubles as the work queue: each triangle is later read back to emit the triangles of
  // the two sub-polygons it leaves, and every sub-polygon yields exactly one triangle, so the queue
  // never outgrows the output and no stack is needed.
  std::size_t writeCount = 0;
  const auto emit = [&](std::size_t first, std::size_t last) {
    if (last - first >= 2)
    {
      triangles[writeCount++] = PolygonTriangle{first, cell(first, last).apex, last};
    }
  };
  emit(0, cornerCount - 1);
  for (std::size_t readIndex = 0; readIndex < writeCount; ++readIndex)
  {
    const auto [first, apex, last] = triangles[readIndex];
    emit(first, apex);
    emit(apex, last);
  }
  GEO_ASSERT(writeCount == triangulation_triangle_count(cornerCount));
  return true;
}

} // namespace Geometry

#endif // GEOMETRY_POLYGONTRIANGULATION_HPP
