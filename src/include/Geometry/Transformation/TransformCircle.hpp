#ifndef FILAPP_TRANSFORMCIRCLE_HPP
#define FILAPP_TRANSFORMCIRCLE_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Circle.hpp>
#include <Geometry/Transformation/TransformVec.hpp>

namespace Geometry
{

template <typename T>
CORE_NODISCARD CORE_CONSTEXPR Circle3<T> transformation(const Circle3<T>& circle,
                                                        const LinAl::HMatrix<T>& transformation)
{
    return Circle3<T>{transformation(circle.getOrigin(), transformation),
                      circle.getRadius(),
                      circle.getNormal()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCIRCLE_HPP
