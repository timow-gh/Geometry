#ifndef GEOMETRY_MESH_DETAIL_POLYGONTRIANGULATION_HPP
#define GEOMETRY_MESH_DETAIL_POLYGONTRIANGULATION_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstddef>
#include <limits>
#include <optional>
#include <vector>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Minimum-cost triangulation of the polygon with corners 0 .. \p count - 1, in order.
 *
 * Dynamic programming over sub-polygons (Klincsek 80; Liepa 03 uses it for hole filling):
 * \p cost(i, m, j) prices triangle (i, m, j) with i < m < j, infinity meaning forbidden, and
 * \p isDiagonalAllowed(i, j) vetoes a diagonal. The side (0, count - 1) always counts as present.
 * Exact search, so a triangulation is found whenever one of finite cost exists. O(count^3) time,
 * O(count^2) memory.
 *
 * \return The count - 2 triangles as index triples (i, m, j), or \c std::nullopt if every
 * triangulation is forbidden.
 */
template <typename TCost, typename TCostFunction, typename TDiagonalAllowed>
GEO_NODISCARD std::optional<std::vector<std::array<std::size_t, 3>>>
triangulate_polygon(std::size_t count, const TCostFunction& cost, const TDiagonalAllowed& isDiagonalAllowed)
{
  GEO_ASSERT(count >= 3);
  constexpr TCost infinity = std::numeric_limits<TCost>::infinity();
  const auto slot = [count](std::size_t i, std::size_t j) { return i * count + j; };

  std::vector<TCost> best(count * count, infinity);
  std::vector<std::size_t> split(count * count, 0);
  for (std::size_t i = 0; i + 1 < count; ++i)
  {
    best[slot(i, i + 1)] = TCost{0};
  }

  for (std::size_t length = 2; length < count; ++length)
  {
    for (std::size_t i = 0; i + length < count; ++i)
    {
      const std::size_t j = i + length;
      const bool isClosingSide = i == 0 && j == count - 1;
      if (!isClosingSide && !isDiagonalAllowed(i, j))
      {
        continue;
      }
      for (std::size_t k = i + 1; k < j; ++k)
      {
        const TCost total = best[slot(i, k)] + best[slot(k, j)] + cost(i, k, j);
        if (total < best[slot(i, j)])
        {
          best[slot(i, j)] = total;
          split[slot(i, j)] = k;
        }
      }
    }
  }

  if (!(best[slot(0, count - 1)] < infinity))
  {
    return std::nullopt;
  }

  std::vector<std::array<std::size_t, 3>> triangles;
  triangles.reserve(count - 2);
  std::vector<std::array<std::size_t, 2>> pending{{0, count - 1}};
  while (!pending.empty())
  {
    const auto [first, last] = pending.back();
    pending.pop_back();
    if (last - first < 2)
    {
      continue;
    }
    const std::size_t apex = split[slot(first, last)];
    triangles.push_back({first, apex, last});
    pending.push_back({first, apex});
    pending.push_back({apex, last});
  }
  return triangles;
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_POLYGONTRIANGULATION_HPP
