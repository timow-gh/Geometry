#ifndef GEOMETRY_DETAIL_FIXEDVECTOR_HPP
#define GEOMETRY_DETAIL_FIXEDVECTOR_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <array>
#include <cstddef>
#include <span>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Sequence with a fixed capacity, for small results whose size has a proven bound, so that
 * computing them never allocates.
 *
 * Exceeding the capacity is a logic error caught by an assertion; callers that can meet more
 * elements than the bound (e.g. because inexact predicates contradict each other) check \c full()
 * first. \p TValue must be default-constructible, since unused slots hold default values.
 */
template <typename TValue, std::size_t Capacity>
class FixedVector {
  std::array<TValue, Capacity> m_values{};
  std::size_t m_size{0};

public:
  static constexpr std::size_t capacity = Capacity;

  GEO_NODISCARD constexpr std::size_t size() const noexcept { return m_size; }
  GEO_NODISCARD constexpr bool empty() const noexcept { return m_size == 0; }
  GEO_NODISCARD constexpr bool full() const noexcept { return m_size == Capacity; }

  constexpr void push_back(const TValue& value) noexcept
  {
    GEO_ASSERT(!full());
    m_values[m_size++] = value;
  }

  GEO_NODISCARD constexpr const TValue& operator[](const std::size_t index) const noexcept
  {
    GEO_ASSERT(index < m_size);
    return m_values[index];
  }

  GEO_NODISCARD constexpr const TValue* begin() const noexcept { return m_values.data(); }
  GEO_NODISCARD constexpr const TValue* end() const noexcept { return m_values.data() + m_size; }

  GEO_NODISCARD constexpr std::span<const TValue> span() const noexcept { return {m_values.data(), m_size}; }
};

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_DETAIL_FIXEDVECTOR_HPP
