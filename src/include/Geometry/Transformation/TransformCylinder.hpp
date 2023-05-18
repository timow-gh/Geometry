#ifndef FILAPP_TRANSFORMCYLINDER_HPP
#define FILAPP_TRANSFORMCYLINDER_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/Transformation/TransformSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Cylinder<T> transformation(const Cylinder<T>& cylinder, const linal::hcoord::hmat<T>& trafo)
{
  return Cylinder<T>{transformation(cylinder.getSegment(), trafo), cylinder.getRadius()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCYLINDER_HPP
