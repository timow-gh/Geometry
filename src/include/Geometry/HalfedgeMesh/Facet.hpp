#ifndef MESHFACE_HEADER
#define MESHFACE_HEADER

#include <Core/Utils/Compiler.hpp>
#include <Geometry/InvalidIndex.hpp>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <utility>

namespace Geometry
{

template <typename T>
class Halfedge;
template <typename T>
class HalfedgeMesh;

template <typename T>
class Facet {
  public:
    CORE_CONSTEXPR Facet(std::size_t halfedgeIndex, HalfedgeMesh<T>& mesh) CORE_NOEXCEPT
        : m_halfedgeIndex(halfedgeIndex)
        , m_mesh(&mesh)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>& getHalfedge() const
    {
        return m_mesh->getHalfedges()[m_halfedgeIndex];
    }
    CORE_CONSTEXPR Halfedge<T>& getHalfedge()
    {
        return m_mesh->getHalfedges()[m_halfedgeIndex];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getHalfedgeIndex() const
    {
        return m_halfedgeIndex;
    }

    CORE_CONSTEXPR bool operator==(const Facet& rhs) const
    {
        return m_halfedgeIndex == rhs.m_halfedgeIndex && m_mesh == rhs.m_mesh;
    }
    CORE_CONSTEXPR bool operator!=(const Facet& rhs) const
    {
        return !(rhs == *this);
    }

    CORE_NODISCARD CORE_CONSTEXPR bool isValid() const
    {
        return m_halfedgeIndex != Geometry::INVALID_INDEX && m_mesh->contains(*this);
    }

  private:
    std::size_t m_halfedgeIndex;
    HalfedgeMesh<T>* m_mesh;
};

} // namespace Geometry

#endif