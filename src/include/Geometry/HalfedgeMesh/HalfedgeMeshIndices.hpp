#ifndef MESHLER_HALFEDGEMESHINDICES_HPP
#define MESHLER_HALFEDGEMESHINDICES_HPP

#include <Geometry/Utils/Compiler.hpp>
#include <cstddef>
#include <limits>

namespace Geometry
{

namespace details
{
template <typename T>
inline GEO_CONSTEXPR T INVALID_INDEX = T(std::numeric_limits<T>::max());
}

template <typename T>
class MeshIndexBase {
  T m_value{details::INVALID_INDEX<T>};

public:
  GEO_CONSTEXPR MeshIndexBase() GEO_NOEXCEPT = default;
  GEO_CONSTEXPR MeshIndexBase(const MeshIndexBase&) GEO_NOEXCEPT = default;
  GEO_CONSTEXPR MeshIndexBase& operator=(const MeshIndexBase&) GEO_NOEXCEPT = default;
  GEO_CONSTEXPR MeshIndexBase(MeshIndexBase&&) GEO_NOEXCEPT = default;
  GEO_CONSTEXPR MeshIndexBase& operator=(MeshIndexBase&&) GEO_NOEXCEPT = default;

  explicit GEO_CONSTEXPR MeshIndexBase(T value)
      : m_value(value)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR bool operator==(const MeshIndexBase& rhs) const { return m_value == rhs.m_value; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator!=(const MeshIndexBase& rhs) const { return !(rhs == *this); }

  GEO_NODISCARD GEO_CONSTEXPR bool operator<(const MeshIndexBase& rhs) const { return m_value < rhs.m_value; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator>(const MeshIndexBase& rhs) const { return rhs < *this; }
  GEO_NODISCARD GEO_CONSTEXPR bool operator<=(const MeshIndexBase& rhs) const { return !(rhs < *this); }
  GEO_NODISCARD GEO_CONSTEXPR bool operator>=(const MeshIndexBase& rhs) const { return !(*this < rhs); }

  GEO_NODISCARD GEO_CONSTEXPR bool isValid() const { return m_value != details::INVALID_INDEX<T>; }

  GEO_NODISCARD GEO_CONSTEXPR T getValue() const { return m_value; }
};

template <typename T>
class VertexIndex : public MeshIndexBase<std::size_t> {
public:
  using MeshIndexBase<T>::MeshIndexBase;
};

template <typename T>
class HalfedgeIndex : public MeshIndexBase<T> {
public:
  using MeshIndexBase<T>::MeshIndexBase;
};

template <typename T>
class FacetIndex : public MeshIndexBase<std::size_t> {
public:
  using MeshIndexBase<T>::MeshIndexBase;
};

} // namespace Geometry

#endif // MESHLER_HALFEDGEMESHINDICES_HPP
