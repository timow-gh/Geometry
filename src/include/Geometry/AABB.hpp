#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include <Geometry/Details/AABBDetails.hpp>
#include <Geometry/ExtremePointsInDirection.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <array>
#include <linal/containers.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
class AABB {
  linal::vec<TFloat, D> m_origin;
  std::array<TFloat, D> m_extends;

public:
  GEO_CONSTEXPR AABB(linal::vec<TFloat, D> origin, TFloat extend)
      : m_origin(origin)
  {
    m_extends.fill(extend);
  }
  GEO_CONSTEXPR AABB(linal::vec<TFloat, D> origin, const std::array<TFloat, D>& extend)
      : m_origin(origin)
      , m_extends(extend)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR linal::vec<TFloat, D> getOrigin() const { return m_origin; }
  GEO_CONSTEXPR void setOrigin(linal::vec<TFloat, D> origin) { m_origin = origin; }

  GEO_NODISCARD GEO_CONSTEXPR const std::array<TFloat, D>& getExtends() const { return m_extends; }
  GEO_CONSTEXPR void setExtends(const std::array<TFloat, D>& extends) { m_extends = extends; }
};

template <typename TFloat, std::size_t D>
AABB<TFloat, D> makeAABB(const linal::vecvector<TFloat, D>& points)
{
  using namespace details;

  linal::vecArray<TFloat, D, D> axis = {linal::vec<TFloat, D>{1, 0, 0}, linal::vec<TFloat, D>{0, 1, 0}, linal::vec<TFloat, D>{0, 0, 1}};

  std::array<MinMax<TFloat>, 3> minMaxes;
  for (std::size_t i = 0; i < D; ++i)
    minMaxes[i] = extremePointsAlongDirection(axis[i], points);

  std::array<TFloat, D> extends;
  for (std::size_t i = 0; i < D; ++i)
    extends[i] = (minMaxes[i].max - minMaxes[i].min) / TFloat(2);

  return AABB<TFloat, D>{linal::vec<TFloat, D>{minMaxes[0].min + extends[0], minMaxes[1].min + extends[1], minMaxes[2].min + extends[2]},
                         extends};
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
