#ifndef GEOMETRY_HANDLE_HPP
#define GEOMETRY_HANDLE_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <limits>

namespace Geometry
{

template <typename T>
class Handle {
public:
  using value_type = T;

  static constexpr value_type invalidHandles = std::numeric_limits<T>::max();
  static constexpr value_type nullHandle = value_type{};

  constexpr Handle() noexcept = default;
  explicit constexpr Handle(T value) noexcept : m_value(value) {}

  constexpr T get_value() const noexcept { return m_value; }
  constexpr bool is_valid() const noexcept { return m_value != invalidHandles; }

  GEO_NODISCARD constexpr bool operator==(const Handle& rhs) const noexcept { return m_value == rhs.m_value; }
  GEO_NODISCARD constexpr bool operator!=(const Handle& rhs) const noexcept { return !(rhs == *this); }
  GEO_NODISCARD constexpr bool operator<(const Handle& rhs) const noexcept { return m_value < rhs.m_value; }
  GEO_NODISCARD constexpr bool operator>(const Handle& rhs) const noexcept { return rhs < *this; }
  GEO_NODISCARD constexpr bool operator<=(const Handle& rhs) const noexcept { return !(rhs < *this); }
  GEO_NODISCARD constexpr bool operator>=(const Handle& rhs) const noexcept { return !(*this < rhs); }

private:
  T m_value = nullHandle;
};

} // namespace Geometry

#endif // GEOMETRY_HANDLE_HPP
