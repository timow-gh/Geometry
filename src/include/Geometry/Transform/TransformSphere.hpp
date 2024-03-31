#ifndef GEOMETRY_TRANSFORMSPHERE_HPP
#define GEOMETRY_TRANSFORMSPHERE_HPP

#include "Geometry/Sphere.hpp"
#include "Geometry/Transform/TransformVec.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>

namespace Geometry
{

template <typename T>
GEO_NODISCARD constexpr Sphere<T> transform(const Sphere<T>& sphere, const linal::hmat<T>& trafo) noexcept
{
  return Sphere<T>{transform(sphere.get_origin(), trafo), sphere.get_radius()};
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMSPHERE_HPP
