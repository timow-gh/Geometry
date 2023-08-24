#ifndef GEOMETRY_TRANSFORMSEGMENT_HPP
#define GEOMETRY_TRANSFORMSEGMENT_HPP

#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>
#include <linal/hvec.hpp>
#include <linal/vec3.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Segment3<T> transform(const Segment3<T>& segment, const linal::hcoord::hmat<T>& trafo) noexcept
{
  linal::hcoord::hvec<T> source = trafo * linal::hcoord::vec_to_hvec<T, 3>(segment.get_source());
  linal::hcoord::hvec<T> target = trafo * linal::hcoord::vec_to_hvec<T, 3>(segment.get_target());
  return Segment3<T>{linal::hcoord::hvec_to_vec<T, 3>(source), linal::hcoord::hvec_to_vec<T, 3>(target)};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMSEGMENT_HPP
