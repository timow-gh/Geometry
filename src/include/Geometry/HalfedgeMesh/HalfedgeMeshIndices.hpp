#ifndef MESHLER_HALFEDGEMESHINDICES_HPP
#define MESHLER_HALFEDGEMESHINDICES_HPP

#include <Core/Utils/Compiler.hpp>
#include <cstddef>
#include <limits>

namespace Geometry
{

namespace details
{
template <typename T>
inline CORE_CONSTEXPR T INVALID_INDEX = T(std::numeric_limits<T>::max());
}

template <typename T>
class MeshIndexBase {
    T m_value{details::INVALID_INDEX<T>};

  public:
    CORE_CONSTEXPR MeshIndexBase() = default;
    explicit CORE_CONSTEXPR MeshIndexBase(T value) : m_value(value) {}

    CORE_NODISCARD CORE_CONSTEXPR bool operator==(const MeshIndexBase& rhs) const { return m_value == rhs.m_value; }
    CORE_NODISCARD CORE_CONSTEXPR bool operator!=(const MeshIndexBase& rhs) const { return !(rhs == *this); }

    CORE_NODISCARD CORE_CONSTEXPR bool operator<(const MeshIndexBase& rhs) const { return m_value < rhs.m_value; }
    CORE_NODISCARD CORE_CONSTEXPR bool operator>(const MeshIndexBase& rhs) const { return rhs < *this; }
    CORE_NODISCARD CORE_CONSTEXPR bool operator<=(const MeshIndexBase& rhs) const { return !(rhs < *this); }
    CORE_NODISCARD CORE_CONSTEXPR bool operator>=(const MeshIndexBase& rhs) const { return !(*this < rhs); }

    CORE_NODISCARD CORE_CONSTEXPR bool isValid() const { return m_value == details::INVALID_INDEX<T>; }

    CORE_NODISCARD CORE_CONSTEXPR T getValue() const { return  m_value; }
};

template <typename T = std::size_t>
class HalfedgeIndex : public MeshIndexBase<T> {
    using MeshIndexBase<T>::MeshIndexBase;
};

template <typename T = std::size_t>
class VertexIndex : public MeshIndexBase<std::size_t> {
    using MeshIndexBase<T>::MeshIndexBase;
};

template <typename T = std::size_t>
class FacetIndex : public MeshIndexBase<std::size_t> {
    using MeshIndexBase<T>::MeshIndexBase;
};

} // namespace Geometry

#endif // MESHLER_HALFEDGEMESHINDICES_HPP
