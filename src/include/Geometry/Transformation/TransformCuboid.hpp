#ifndef FILAPP_TRANSFORMCUBOID_HPP
#define FILAPP_TRANSFORMCUBOID_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Cuboid.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
CORE_NODISCARD CORE_CONSTEXPR Cuboid<T> transformation(const Cuboid<T>& cuboid, const LinAl::HMatrix<T>& trafo)
{
    LinAl::HVec<T> origin = trafo * LinAl::vec3ToHVec(cuboid.getOrigin());
    return Cuboid<T>{LinAl::hVecToVec3(origin), cuboid.getSideVectors()};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMCUBOID_HPP
