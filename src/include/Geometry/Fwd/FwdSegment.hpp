#ifndef GEOMETRY_FWDSEGMENT_HPP
#define GEOMETRY_FWDSEGMENT_HPP

#include <cmath>
#include <cstdint>

namespace Geometry
{
template <typename T, std::size_t D>
class Segment;
template <typename T>
using Segment3 = class Segment<T, 3>;

template <typename T>
using Segment2 = Segment<T, 2>;
using Segment2f = Segment2<float>;
using Segment2d = Segment2<double>;

template <typename T>
using Segment3 = Segment<T, 3>;
using Segment3f = Segment3<float>;
using Segment3d = Segment3<double>;
} // namespace Geometry

#endif // GEOMETRY_FWDSEGMENT_HPP
