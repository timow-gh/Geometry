#ifndef GEOMETRY_AABBDETAILS_HPP
#define GEOMETRY_AABBDETAILS_HPP

#include <Geometry/Fwd/FwdAABB.hpp>
#include <cstdint>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

namespace details
{

template <typename TFloat>
struct MinMax
{
    TFloat min{0.0};
    TFloat max{0.0};
};

template <typename TFloat, std::size_t D>
MinMax<TFloat> extremePointsAlongDirection(LinAl::Vec<TFloat, D> dir, const LinAl::VecVector<TFloat, D>& points)
{
    TFloat minDist = std::numeric_limits<TFloat>::max();
    TFloat maxDist = std::numeric_limits<TFloat>::lowest();

    for (auto vec: points)
    {
        TFloat dist = LinAl::dot(vec, dir);
        if (dist < minDist)
        {
            minDist = dist;
        }
        if (dist > maxDist)
        {
            maxDist = dist;
        }
    }
    return {minDist, maxDist};
}

} // namespace details
}

#endif // GEOMETRY_AABBDETAILS_HPP
