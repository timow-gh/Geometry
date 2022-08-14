#ifndef GEOMETRY_FWDAABB_HPP
#define GEOMETRY_FWDAABB_HPP

#include <cmath>
#include <cstddef>

namespace Geometry
{

template <typename TFloat, std::size_t D>
class AABB;

template <typename TFloat>
using AABB2 = AABB<TFloat, 2>;
using AABB2f = AABB2<float_t>;
using AABB2d = AABB2<double_t>;

template <typename TFloat>
using AABB3 = AABB<TFloat, 3>;
using AABB3f = AABB3<float_t>;
using AABB3d = AABB3<double_t>;

} // namespace Geometry

#endif // GEOMETRY_FWDAABB_HPP
