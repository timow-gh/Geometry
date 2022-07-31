#ifndef GEOMETRY_FWDSEGMENT_HPP
#define GEOMETRY_FWDSEGMENT_HPP

#include <cstdint>

namespace Geometry
{
template <typename T, std::size_t D>
class Segment;
template <typename T>
using Segment3 = class Segment<T, 3>;
} // namespace Geometry

#endif // GEOMETRY_FWDSEGMENT_HPP
