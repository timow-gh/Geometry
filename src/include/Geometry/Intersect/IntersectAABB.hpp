#ifndef GEOMETRY_INTERSECTAABB_HPP
#define GEOMETRY_INTERSECTAABB_HPP

#include "Geometry/AABB.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

/** @brief Checks if two AABBs are intersecting.
 *
 * Two AABBs are intersecting if they have a non-empty intersection.
 *
 * @tparam TFloat The type of the AABB components.
 * @tparam D The dimension of the AABBs.
 * @param lhs The first AABB.
 * @param rhs The second AABB.
 * @return bool True if the AABBs are intersecting, false otherwise.
 */
template <typename TFloat, std::size_t D>
GEO_NODISCARD constexpr bool is_intersecting(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs) noexcept
{
  auto lhsMax = lhs.get_max();
  auto lhsMin = lhs.get_min();
  auto rhsMax = rhs.get_max();
  auto rhsMin = rhs.get_min();

  for (std::size_t i = 0; i < D; ++i)
  {
    if (linal::isLessEq(lhsMax[i], rhsMin[i]) || linal::isGreaterEq(lhsMin[i], rhsMax[i]))
    {
      return false;
    }
  }
  return true;
}

/** @brief Calculates the intersection of two AABBs.
 *
 * @tparam TFloat The type of the AABB components.
 * @tparam D The dimension of the AABBs.
 * @param lhs The first AABB.
 * @param rhs The second AABB.
 * @return AABB The intersection of the two AABBs or an invalid AABB if they do not intersect.
 */
template <typename TFloat, std::size_t D>
GEO_NODISCARD constexpr AABB<TFloat, D> intersect(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs) noexcept
{
  auto lhsMax = lhs.get_max();
  auto lhsMin = lhs.get_min();
  auto rhsMax = rhs.get_max();
  auto rhsMin = rhs.get_min();

  auto min = lhsMin;
  auto max = lhsMax;

  for (std::size_t i = 0; i < D; ++i)
  {
    if (linal::isLessEq(lhsMax[i], rhsMin[i]) || linal::isGreaterEq(lhsMin[i], rhsMax[i]))
    {
      return AABB<TFloat, D>{};
    }
    min[i] = std::max(lhsMin[i], rhsMin[i]);
    max[i] = std::min(lhsMax[i], rhsMax[i]);
  }

  return AABB<TFloat, D>{min, max};
}

} // namespace Geometry
#endif // GEOMETRY_INTERSECTAABB_HPP
