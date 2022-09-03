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

    LinAl::VecArray<TFloat, D, D> axis = {LinAl::Vec<TFloat, D>{1, 0, 0},
                                          LinAl::Vec<TFloat, D>{0, 1, 0},
                                          LinAl::Vec<TFloat, D>{0, 0, 1}};

    Core::TArray<MinMax<TFloat>, 3> minMaxes;
    for (std::size_t i = 0; i < D; ++i)
        minMaxes[i] = extremePointsAlongDirection(axis[i], points);

    Core::TArray<TFloat, D> extends;
    for (std::size_t i = 0; i < D; ++i)
        extends[i] = (minMaxes[i].max - minMaxes[i].min) / TFloat(2);

    return AABB<TFloat, D>{
        LinAl::Vec<TFloat, D>{minMaxes[0].min + extends[0], minMaxes[1].min + extends[1], minMaxes[2].min + extends[2]},
        extends};
}

} // namespace Geometry

#endif // GEOMETRY_AABB_HPP
