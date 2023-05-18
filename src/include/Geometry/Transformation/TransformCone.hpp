#ifndef FILAPP_TRANSFORMCONE_HPP
#define FILAPP_TRANSFORMCONE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Cone.hpp>
#include <Geometry/Transformation/TransformSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Cone<T> transformation(const Cone<T>& cone, const linal::hcoord::hmatd& trafo)
{
  return Cone<T>{Geometry::transformation(cone.get_segment(), trafo), cone.getRadius()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCONE_HPP
