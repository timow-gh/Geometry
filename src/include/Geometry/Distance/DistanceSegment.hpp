#ifndef GEOMETRY_DISTANCESEGMENT_HPP
#define GEOMETRY_DISTANCESEGMENT_HPP

#include "Core/Math/Eps.hpp"
#include "Geometry/FwdGeometry.hpp"
#include "LinAl/LinearAlgebra.hpp"

namespace Geometry
{
template <typename T, std::size_t D>
T distance(const Segment<T, D>& segment, const LinAl::Vec<T, D>& vec)
{
    const LinAl::Vec<T, D>& source = segment.getSource();
    LinAl::Vec<T, D> ps = vec - source;
    LinAl::Vec<T, D> ts = segment.getTarget() - source;
    T parameter = LinAl::dot(ps, ts) / LinAl::norm2(ts);
    T projParamter = std::min(std::max(parameter, T(0.0)), T(1.0));
    LinAl::Vec<T, D> projVec{source + projParamter * ts};
    return LinAl::norm2(LinAl::Vec<T, D>{projVec - vec});
}

template <typename T, std::size_t D>
T distance(const LinAl::Vec<T, D>& vec, const Segment<T, D>& segment)
{
    return distance(segment, vec);
}
} // namespace Geometry

#endif // GEOMETRY_DISTANCESEGMENT_HPP
