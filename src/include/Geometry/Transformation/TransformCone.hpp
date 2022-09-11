#ifndef FILAPP_TRANSFORMCONE_HPP
#define FILAPP_TRANSFORMCONE_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Cone.hpp>
#include <Geometry/Transformation/TransformSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
CORE_NODISCARD CORE_CONSTEXPR Cone<T> transformation(const Cone<T>& cone, const LinAl::HMatrixd& trafo)
{
  return Cone<T>{Geometry::transformation(cone.getSegment(), trafo), cone.getRadius()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCONE_HPP
