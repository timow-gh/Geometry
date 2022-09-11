#ifndef FILAPP_TRANSFORMSEGMENT_HPP
#define FILAPP_TRANSFORMSEGMENT_HPP

#include <Core/Utils/Compiler.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
CORE_NODISCARD CORE_CONSTEXPR Segment3<T> transformation(const Segment3<T>& segment, const LinAl::HMatrix<T>& trafo)
{
  LinAl::HVec<T> source = trafo * LinAl::vec3ToHVec(segment.getSource());
  LinAl::HVec<T> target = trafo * LinAl::vec3ToHVec(segment.getTarget());
  return Segment3<T>{LinAl::hVecToVec3(source), LinAl::hVecToVec3(target)};
}

} // namespace Geometry

#endif // FILAPP_TRANSFORMSEGMENT_HPP
