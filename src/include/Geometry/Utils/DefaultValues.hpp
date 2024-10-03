#ifndef GEOMETRY_DEFAULTVALUES_HPP
#define GEOMETRY_DEFAULTVALUES_HPP

#include <cstdint>
#include <limits>
#include <linal/vec.hpp>

namespace Geometry
{

template <typename TFloat>
constexpr inline auto highest_v = std::numeric_limits<TFloat>::max();

template <typename TFloat>
constexpr inline auto lowest_v = std::numeric_limits<TFloat>::lowest();

template <typename TFloat, std::uint8_t D>
constexpr inline linal::vec<TFloat, D> vec_highest_v = linal::vec<TFloat, D>(highest_v<TFloat>);
template <typename TFloat>
constexpr inline linal::vec3<TFloat> vec3_highest_v = linal::vec3<TFloat>(highest_v<TFloat>);
template <typename TFloat>
constexpr inline linal::vec2<TFloat> vec2_highest_v = linal::vec2<TFloat>(highest_v<TFloat>);

template <typename TFloat, std::uint8_t D>
linal::vec<TFloat, D> vec_lowest_v = linal::vec<TFloat, D>(lowest_v<TFloat>);
template <typename TFloat>
linal::vec3<TFloat> vec3_lowest_v = linal::vec3<TFloat>(lowest_v<TFloat>);
template <typename TFloat>
linal::vec2<TFloat> vec2_lowest_v = linal::vec2<TFloat>(lowest_v<TFloat>);

} // namespace Geometry

#endif // GEOMETRY_DEFAULTVALUES_HPP
