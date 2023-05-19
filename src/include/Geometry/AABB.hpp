#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include <Geometry/ExtremePointsInDirection.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <array>
#include <linal/containers.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

/**
 * @brief Axis Aligned Bounding Box
 *
 * @tparam TFloat The type of the floating point numbers
 * @tparam D The dimension of the AABB
 */
template <typename TFloat, std::size_t D>
class AABB {
  linal::vec<TFloat, D> m_min;
  linal::vec<TFloat, D> m_max;

public:
  GEO_CONSTEXPR AABB() GEO_NOEXCEPT = default;
  GEO_CONSTEXPR AABB(linal::vec<TFloat, D> min, linal::vec<TFloat, D> max) GEO_NOEXCEPT
      : m_min{min}
      , m_max{max}
  {
  }

  GEO_CONSTEXPR AABB(linal::vec<TFloat, D> min, TFloat extend) GEO_NOEXCEPT : m_min{min}
  {
    for (std::size_t i = 0; i < D; ++i)
    {
      m_max[i] = min[i] + extend;
    }
  }

  GEO_NODISCARD GEO_CONSTEXPR linal::vec<TFloat, D> get_min() const GEO_NOEXCEPT { return m_min; }
  GEO_NODISCARD GEO_CONSTEXPR linal::vec<TFloat, D> get_max() const GEO_NOEXCEPT { return m_max; }

  GEO_CONSTEXPR void set_min(linal::vec<TFloat, D> min) GEO_NOEXCEPT { m_min = min; }
  GEO_CONSTEXPR void set_max(linal::vec<TFloat, D> max) GEO_NOEXCEPT { m_max = max; }

  GEO_NODISCARD GEO_CONSTEXPR linal::vec<TFloat, D> get_center() const GEO_NOEXCEPT { return (m_min + (m_max - m_min) / TFloat{2}); }

  //  /**
  //   * @brief Check if the AABB is empty
  //   *
  //   * @return true, if one of the half extends is zero
  //   */
  //  GEO_NODISCARD GEO_CONSTEXPR bool is_empty() const GEO_NOEXCEPT {}
  //
  //  /**
  //   * @brief Increase the size of the AABB, such that the point is inside the AABB
  //   *
  //   * @param vec The point to add to the AABB
  //   * @return true, if the point is inside the AABB
  //   */
  //  void add(linal::vec<TFloat, D> vec) {}
  //
  //  /** @brief Increase the size of the AABB, such that the AABB contains the other AABB
  //   *
  //   * @param other The AABB to add to the AABB
  //   */
  //  GEO_CONSTEXPR void add(const AABB& other) const
  //  {
  //
  //  }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const AABB<TFloat, D>& rhs) const { return m_min == rhs.m_min && m_max == rhs.m_max; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const AABB<TFloat, D>& rhs) const { return !(*this == rhs); }
};

template <typename TFloat, std::size_t D>
AABB<TFloat, D> makeAABB(const linal::vecvector<TFloat, D>& points)
{
  GEO_ASSERT(!points.empty());

  linal::vecArray<TFloat, D, D> axis;
  for (std::size_t i = 0; i < D; ++i)
  {
    axis[i][i] = 1;
  }

  std::array<MinMax<TFloat>, D> minMaxes;
  for (std::size_t i = 0; i < D; ++i)
  {
    minMaxes[i] = extremePointsAlongDirection(axis[i], points);
  }

  return AABB<TFloat, D>{linal::vec<TFloat, D>{minMaxes[0].min, minMaxes[1].min, minMaxes[2].min},
                         linal::vec<TFloat, D>{minMaxes[0].max, minMaxes[1].max, minMaxes[2].max}};
}

template <typename TFloat>
using AABB2 = AABB<TFloat, 2>;
using AABB2f = AABB2<float>;
using AABB2d = AABB2<double>;

template <typename TFloat>
using AABB3 = AABB<TFloat, 3>;
using AABB3f = AABB3<float>;
using AABB3d = AABB3<double>;

} // namespace Geometry

#endif // GEOMETRY_AABB_HPP
