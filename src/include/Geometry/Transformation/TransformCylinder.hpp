#ifndef FILAPP_TRANSFORMCYLINDER_HPP
#define FILAPP_TRANSFORMCYLINDER_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/Transformation/TransformSegment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
CORE_NODISCARD CORE_CONSTEXPR Cylinder<T> transformation(const Cylinder<T>& cylinder, const LinAl::HMatrix<T>& trafo)
{
    return Cylinder<T>{transformation(cylinder.getSegment(), trafo), cylinder.getRadius()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCYLINDER_HPP
