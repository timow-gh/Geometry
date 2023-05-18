#ifndef GEOMETRY_TRANSFORMCUBOID_HPP
#define GEOMETRY_TRANSFORMCUBOID_HPP

#include <Geometry/Cuboid.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Cuboid<T> transformation(const Cuboid<T>& cuboid, const linal::hcoord::hmat<T>& trafo)
{
  linal::hcoord::hvec<T> origin = trafo * linal::vec3ToHVec(cuboid.getOrigin());
  return Cuboid<T>{linal::hcoord::hvecToVec3(origin), cuboid.getSideVectors()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCUBOID_HPP
