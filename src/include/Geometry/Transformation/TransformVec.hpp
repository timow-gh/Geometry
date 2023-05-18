#ifndef FILAPP_TRANSFORMVEC_HPP
#define FILAPP_TRANSFORMVEC_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR linal::Vec3<T> transformation(const linal::Vec3<T>& vec, const linal::hcoord::hmat<T>& trafo)
{
  linal::hcoord::hvec<T> result = trafo * linal::vec3ToHVec(vec);
  return linal::hcoord::hvecToVec3(result);
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMVEC_HPP
