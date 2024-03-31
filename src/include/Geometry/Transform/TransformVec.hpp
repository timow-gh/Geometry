#ifndef GEOMETRY_TRANSFORMVEC_HPP
#define GEOMETRY_TRANSFORMVEC_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>

#include <linal/vec.hpp>

namespace Geometry
{

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr linal::vec<T, D> transform(const linal::vec<T, D>& vec, const linal::hmat<T>& trafo) noexcept
{
  linal::hvec<T> result = trafo * linal::to_hvec<T, D>(vec);
  return linal::to_vec<T, D>(result);
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMVEC_HPP
