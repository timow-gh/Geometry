#ifndef FILAPP_TRANSFORMSEGMENT_HPP
#define FILAPP_TRANSFORMSEGMENT_HPP

#include <Geometry/Segment.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Segment3<T> transformation(const Segment3<T>& segment, const linal::hcoord::hmat<T>& trafo)
{
  linal::hcoord::hvec<T> source = trafo * linal::vec3ToHVec(segment.getSource());
  linal::hcoord::hvec<T> target = trafo * linal::vec3ToHVec(segment.getTarget());
  return Segment3<T>{linal::hcoord::hvecToVec3(source), linal::hcoord::hvecToVec3(target)};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMSEGMENT_HPP
