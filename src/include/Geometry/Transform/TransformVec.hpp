#ifndef GEOMETRY_TRANSFORMVEC_HPP
#define GEOMETRY_TRANSFORMVEC_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <linal/hmat.hpp>
#include <linal/hvec.hpp>
#include <linal/vec3.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD constexpr linal::vec<T, D> transform(const linal::vec<T, D>& vec, const linal::hcoord::hmat<T>& trafo) noexcept
{
  linal::hcoord::hvec<T> result = trafo * linal::hcoord::vec_to_hvec<T, D>(vec);
  return linal::hcoord::hvec_to_vec<T, D>(result);
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMVEC_HPP
