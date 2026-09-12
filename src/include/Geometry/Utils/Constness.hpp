#ifndef GEOMETRY_UTILS_CONSTNESS_HPP
#define GEOMETRY_UTILS_CONSTNESS_HPP

#include <type_traits>

namespace Geometry
{

enum class Constness : bool { Mutable = false, Const = true };

// True when the view/iterator is a read-only projection of the underlying element.
constexpr bool is_const(Constness c) noexcept { return c == Constness::Const; }

// A pointer / reference whose cv-qualification is applied per the Constness parameter.
template <Constness C, typename T>
using qualified_ptr_t = std::conditional_t<is_const(C), const T*, T*>;

template <Constness C, typename T>
using qualified_ref_t = std::conditional_t<is_const(C), const T&, T&>;

} // namespace Geometry

#endif // GEOMETRY_UTILS_CONSTNESS_HPP
