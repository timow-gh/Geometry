#ifndef GEOMETRY_TRANSFORMATONAABB_HPP
#define GEOMETRY_TRANSFORMATONAABB_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/AABB.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR AABB<TFloat, D> transformation(const AABB<TFloat, D>& aabb, const LinAl::HMatrix<TFloat>& trafo)
{
  LinAl::HVec<TFloat> origin = trafo * LinAl::vec3ToHVec(aabb.getOrigin());
  return AABB<TFloat, D>{LinAl::hVecToVec3(origin), aabb.getExtends()};
}

} // namespace Geometry
#endif // GEOMETRY_TRANSFORMATONAABB_HPP
