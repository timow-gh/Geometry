#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include <Core/Types/TArray.hpp>
#include <Geometry/Details/AABBDetails.hpp>
#include <Geometry/Fwd/FwdAABB.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
class AABB {
    LinAl::Vec<TFloat, D> m_origin;
    Core::TArray<TFloat, D> m_extends;

  public:
    CORE_CONSTEXPR AABB(LinAl::Vec<TFloat, D> origin, TFloat extend) : m_origin(origin) { m_extends.fill(extend); }
    CORE_CONSTEXPR AABB(LinAl::Vec<TFloat, D> origin, const Core::TArray<TFloat, D>& extend) : m_origin(origin), m_extends(extend)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec<TFloat, D> getOrigin() const { return m_origin; }
    CORE_CONSTEXPR void setOrigin(LinAl::Vec<TFloat, D> origin) { m_origin = origin; }

    CORE_NODISCARD CORE_CONSTEXPR const Core::TArray<TFloat, D>& getExtends() const { return m_extends; }
    CORE_CONSTEXPR void setExtends(const Core::TArray<TFloat, D>& extends) { m_extends = extends; }
};

template <typename TFloat, std::size_t D>
AABB<TFloat, D> makeAABB(const LinAl::VecVector<TFloat, D>& points)
{
    using namespace details;
    MinMax<TFloat> xMinMax = extremePointsAlongDirection(LinAl::Vec<TFloat, D>{1, 0, 0}, points);
    MinMax<TFloat> yMinMax = extremePointsAlongDirection(LinAl::Vec<TFloat, D>{0, 1, 0}, points);
    MinMax<TFloat> zMinMax = extremePointsAlongDirection(LinAl::Vec<TFloat, D>{0, 0, 1}, points);
    TFloat xhalfExtend = (xMinMax.max - xMinMax.min) / 2.0;
    TFloat yhalfExtend = (yMinMax.max - yMinMax.min) / 2.0;
    TFloat zhalfExtend = (zMinMax.max - zMinMax.min) / 2.0;
    return AABB<TFloat, D>{LinAl::Vec<TFloat, D>{xMinMax.min + xhalfExtend, yMinMax.min + yhalfExtend, zMinMax.min + zhalfExtend},
                           Core::TArray<TFloat, D>{xhalfExtend, yhalfExtend, zhalfExtend}};
}

} // namespace Geometry

#endif // GEOMETRY_AABB_HPP
