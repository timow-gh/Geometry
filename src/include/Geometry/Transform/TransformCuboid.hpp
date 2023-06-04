#ifndef GEOMETRY_TRANSFORMCUBOID_HPP
#define GEOMETRY_TRANSFORMCUBOID_HPP

#include <Geometry/Cuboid.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Cuboid<T> transform(const Cuboid<T>& cuboid, const linal::hcoord::hmat<T>& trafo) noexcept
{
  linal::hcoord::hvec<T> origin = trafo * linal::hcoord::vec_to_hvec<T,3>(cuboid.get_origin());
  return Cuboid<T>{linal::hcoord::hvec_to_vec<T,D>(origin), cuboid.get_side_vectors()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMCUBOID_HPP
