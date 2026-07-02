#ifndef GEOMETRY_TRANSFORMVEC_HPP
#define GEOMETRY_TRANSFORMVEC_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <linal/hmat.hpp>

#include <linal/vec.hpp>

namespace Geometry
{

namespace detail
{
template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr linal::vec<T, D> hvec_to_vec(const linal::hvec<T>& vec) noexcept
{
  static_assert(D == 2 || D == 3);
  if constexpr (D == 2)
  {
    return linal::vec<T, D>{vec[0] / vec[3], vec[1] / vec[3]};
  }
  else
  {
    return linal::vec<T, D>{vec[0] / vec[3], vec[1] / vec[3], vec[2] / vec[3]};
  }
}
} // namespace detail

template <typename T, std::uint8_t D>
GEO_NODISCARD constexpr linal::vec<T, D> transform(const linal::vec<T, D>& vec, const linal::hmat<T>& trafo) noexcept
{
  linal::hvec<T> result = trafo * linal::to_hvec<T, D>(vec);
  return detail::hvec_to_vec<T, D>(result);
}

} // namespace Geometry

#endif // GEOMETRY_TRANSFORMVEC_HPP
