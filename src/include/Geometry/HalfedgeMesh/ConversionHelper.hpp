#ifndef ORIGAMI_CONVERSIONHELPER_HPP
#define ORIGAMI_CONVERSIONHELPER_HPP

#include <concepts>

namespace Geometry
{

template <std::integral TIndex, std::integral T>
[[nodiscard]] constexpr TIndex to_idx(T value) noexcept
{
  return TIndex(value);
}

} // namespace Geometry

#endif // ORIGAMI_CONVERSIONHELPER_HPP
