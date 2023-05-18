#ifndef GEOMETRY_TRANSFORMVEC_HPP
#define GEOMETRY_TRANSFORMVEC_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <linal/HMat.hpp>
#include <linal/HVec.hpp>
#include <linal/Vec3.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR linal::Vec<T, D> transformation(const linal::Vec<T, D>& vec, const linal::hcoord::hmat<T>& trafo)
{
  linal::hcoord::hvec<T> result = trafo * linal::hcoord::vec_to_hvec<T, D>(vec);
  return linal::hcoord::hvec_to_vec<T, D>(result);
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMVEC_HPP
