#ifndef GEOMETRY_TRANSFORMATONAABB_HPP
#define GEOMETRY_TRANSFORMATONAABB_HPP

#include <Geometry/AABB.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/HMat.hpp>
#include <linal/HMatRotation.hpp>
#include <linal/HVec.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR AABB<TFloat, D> transformation(const AABB<TFloat, D>& aabb, const linal::hcoord::hmat<TFloat>& trafo)
{
  linal::hcoord::hvec<TFloat> origin = trafo * linal::hcoord::vec3_to_hvec(aabb.getOrigin());
  return AABB<TFloat, D>{linal::hcoord::hvec_to_vec3(origin), aabb.getExtends()};
}

} // namespace Geometry
#endif // GEOMETRY_TRANSFORMATONAABB_HPP
