#ifndef FILAPP_TRANSFORMSPHERE_HPP
#define FILAPP_TRANSFORMSPHERE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Sphere.hpp>
#include <Geometry/Transformation/TransformVec.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Sphere<T> transformation(const Sphere<T>& sphere, const linal::hcoord::hmat<T>& trafo)
{
  return Sphere<T>{transformation(sphere.getOrigin(), trafo), sphere.getRadius()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMSPHERE_HPP
