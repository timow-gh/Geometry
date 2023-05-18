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
  linal::hcoord::hvec<T> origin = trafo * linal::hcoord::vec_to_hvec<T,3>(cuboid.getOrigin());
  return Cuboid<T>{linal::hcoord::hvec_to_vec<T,D>(origin), cuboid.getSideVectors()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCUBOID_HPP
