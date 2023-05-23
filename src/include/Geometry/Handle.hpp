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

  static GEO_CONSTEXPR value_type invalidHanldes = std::numeric_limits<T>::max();
  static GEO_CONSTEXPR value_type nullHandle = value_type{};

  GEO_CONSTEXPR Handle() GEO_NOEXCEPT = default;
  explicit GEO_CONSTEXPR Handle(T value) GEO_NOEXCEPT : m_value(value) {}

  GEO_CONSTEXPR T getValue() const GEO_NOEXCEPT { return m_value; }
  GEO_CONSTEXPR bool isValid() const GEO_NOEXCEPT { return m_value != invalidHanldes; }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const Handle& rhs) const GEO_NOEXCEPT { return m_value == rhs.m_value; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const Handle& rhs) const GEO_NOEXCEPT { return !(rhs == *this); }
  GEO_NODISCARD GEO_CONSTEXPR bool operator<(const Handle& rhs) const GEO_NOEXCEPT { return m_value < rhs.m_value; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator>(const Handle& rhs) const GEO_NOEXCEPT { return rhs < *this; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator<=(const Handle& rhs) const GEO_NOEXCEPT { return !(rhs < *this); }
  GEO_NODISCARD GEO_CONSTEXPR bool operator>=(const Handle& rhs) const GEO_NOEXCEPT { return !(*this < rhs); }

private:
  T m_value = nullHandle;
};

} // namespace Geometry

#endif // GEOMETRY_HANDLE_HPP
