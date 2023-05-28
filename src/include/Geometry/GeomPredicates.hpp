#ifndef GEOMETRY_GEOMPREDICATES_HPP
#define GEOMETRY_GEOMPREDICATES_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <cstdint>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

// compare vector components using eps
template <typename T, std::size_t D>
GEO_NODISCARD constexpr bool is_equal(const linal::vec<T, D> lhs, const linal::vec<T, D> rhs) noexcept
{
  for (std::size_t i = 0; i < D; ++i)
  {
    if (!linal::isEq(lhs[i], rhs[i]))
    {
      return false;
    }
  }
  return true;
}

} // namespace Geometry

#endif // GEOMETRY_GEOMPREDICATES_HPP
