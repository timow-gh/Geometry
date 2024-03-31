#ifndef GEOMETRY_TRANSFORMAABB_HPP
#define GEOMETRY_TRANSFORMAABB_HPP

#include "Geometry/AABB.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>



namespace Geometry
{

template <typename TFloat, std::uint8_t D>
GEO_NODISCARD constexpr AABB<TFloat, D> transform(const AABB<TFloat, D>& aabb, const linal::hmat<TFloat>& trafo) noexcept
{
  linal::hvec<TFloat> min = trafo * linal::to_hvec<TFloat, D>(aabb.get_min());
  linal::hvec<TFloat> max = trafo * linal::to_hvec<TFloat, D>(aabb.get_max());
  return AABB<TFloat, D>{linal::to_vec<TFloat, D>(min), linal::to_vec<TFloat, D>(max)};
}

} // namespace Geometry
#endif // GEOMETRY_TRANSFORMAABB_HPP
