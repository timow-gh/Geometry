#ifndef FILAPP_TRANSFORMCONE_HPP
#define FILAPP_TRANSFORMCONE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Cone.hpp>
#include <Geometry/Transformation/TransformSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR Cone<T> transformation(const Cone<T>& cone, const LinAl::HMatrixd& trafo)
{
  return Cone<T>{Geometry::transformation(cone.get_segment(), trafo), cone.get_radius()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCONE_HPP
