#ifndef GEOMETRY_INTERSECTIONAABB_HPP
#define GEOMETRY_INTERSECTIONAABB_HPP

#include <Geometry/Fwd/FwdAABB.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
bool isIntersecting(AABB<TFloat, D> lhs, AABB<TFloat, D> rhs)
{
    LinAl::Vec<TFloat, D> lhsOrigin = lhs.getOrigin();
    LinAl::Vec<TFloat, D> rhsOrigin = rhs.getOrigin();

    Core::TArray<TFloat, D> lhsExtend = lhs.getExtend();
    Core::TArray<TFloat, D> rhsExtend = rhs.getExtend();

    if (Core::isGreater(std::abs(lhsOrigin[0] - rhsOrigin[0]), lhsExtend[0] + rhsExtend[0]))
        return false;
    if (Core::isGreater(std::abs(lhsOrigin[1] - rhsOrigin[1]), lhsExtend[1] + rhsExtend[1]))
        return false;
    if constexpr (D == 3)
    {
        if (Core::isGreater(std::abs(lhsOrigin[2] - rhsOrigin[2]), lhsExtend[2] + rhsExtend[2]))
            return false;
    }
    return true;
}

} // namespace Geometry
#endif // GEOMETRY_INTERSECTIONAABB_HPP
