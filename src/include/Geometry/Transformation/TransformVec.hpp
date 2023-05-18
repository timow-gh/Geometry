#ifndef FILAPP_TRANSFORMVEC_HPP
#define FILAPP_TRANSFORMVEC_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec3<T> transformation(const LinAl::Vec3<T>& vec, const LinAl::HMatrix<T>& trafo)
{
  LinAl::HVec<T> result = trafo * LinAl::vec3ToHVec(vec);
  return LinAl::hVecToVec3(result);
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMVEC_HPP
