#ifndef GEOMETRY_TRANSFORMAABB_HPP
#define GEOMETRY_TRANSFORMAABB_HPP

#include <Geometry/AABB.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/hmat.hpp>
#include <linal/hmat_rotation.hpp>
#include <linal/hvec.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
GEO_NODISCARD constexpr AABB<TFloat, D> transform(const AABB<TFloat, D>& aabb, const linal::hcoord::hmat<TFloat>& trafo) noexcept
{
  linal::hcoord::hvec<TFloat> min = trafo * linal::hcoord::vec_to_hvec<TFloat, D>(aabb.get_min());
  linal::hcoord::hvec<TFloat> max = trafo * linal::hcoord::vec_to_hvec<TFloat, D>(aabb.get_max());
  return AABB<TFloat, D>{linal::hcoord::hvec_to_vec<TFloat, D>(min), linal::hcoord::hvec_to_vec<TFloat, D>(max)};
}

} // namespace Geometry
#endif // GEOMETRY_TRANSFORMAABB_HPP
