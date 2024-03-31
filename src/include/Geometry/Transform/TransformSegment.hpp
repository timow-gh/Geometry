#ifndef GEOMETRY_TRANSFORMSEGMENT_HPP
#define GEOMETRY_TRANSFORMSEGMENT_HPP

#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>

#include <linal/vec.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Segment3<T> transform(const Segment3<T>& segment, const linal::hmat<T>& trafo) noexcept
{
  linal::hvec<T> source = trafo * linal::to_hvec<T, 3>(segment.get_source());
  linal::hvec<T> target = trafo * linal::to_hvec<T, 3>(segment.get_target());
  return Segment3<T>{linal::to_vec<T, 3>(source), linal::to_vec<T, 3>(target)};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMSEGMENT_HPP
