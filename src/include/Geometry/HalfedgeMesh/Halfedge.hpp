#ifndef GLFWTESTAPP_HALFEDGE_H
#define GLFWTESTAPP_HALFEDGE_H

#include <Core/Utils/Compiler.hpp>
#include <Geometry/InvalidIndex.hpp>
#include <limits>

namespace Geometry
{

template <typename T>
class Vertex;

template <typename T>
class Facet;

template <typename T>
struct HalfedgeMesh;

template <typename T>
class Halfedge {
  public:
    CORE_CONSTEXPR Halfedge(std::size_t vertexIndex, HalfedgeMesh<T>* mesh) CORE_NOEXCEPT
        : m_facetIdx(INVALID_INDEX)
        , m_vertexIdx(vertexIndex)
        , m_nextIdx(INVALID_INDEX)
        , m_previousIdx(INVALID_INDEX)
        , m_oppositeIdx(INVALID_INDEX)
        , m_mesh(mesh)
    {
    }

    CORE_NODISCARD
    CORE_CONSTEXPR const Facet<T>& getFacet() const
    {
        return m_mesh->getFacets()[m_facetIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getFacetIndex() const
    {
        return m_facetIdx;
    }

    CORE_CONSTEXPR void setFacetIndex(std::size_t facetIndex)
    {
        m_facetIdx = facetIndex;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>& getNext() const
    {
        return m_mesh->getHalfedges()[m_nextIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR Halfedge<T>& getNext()
    {
        return m_mesh->getHalfedges()[m_nextIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getNextIndex() const
    {
        return m_nextIdx;
    }

    CORE_CONSTEXPR void setNextIndex(std::size_t index)
    {
        m_nextIdx = index;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>& getPrevious() const
    {
        return m_mesh->getHalfedges()[m_previousIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR Halfedge<T>& getPrevious()
    {
        return m_mesh->getHalfedges()[m_previousIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getPreviousIndex() const
    {
        return m_previousIdx;
    }

    CORE_CONSTEXPR void setPreviousIndex(std::size_t index)
    {
        m_previousIdx = index;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>& getOpposite() const
    {
        return m_mesh->getHalfedges()[m_oppositeIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getOppositeIndex() const
    {
        return m_oppositeIdx;
    }

    CORE_CONSTEXPR void setOppositeIndex(std::size_t index)
    {
        m_oppositeIdx = index;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Vertex<T>& getVertex() const
    {
        return m_mesh->getVertices()[m_vertexIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR Vertex<T>& getVertex()
    {
        return m_mesh->getVertices()[m_vertexIdx];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getVertexIndex() const
    {
        return m_vertexIdx;
    }

    CORE_CONSTEXPR void setVertexIndex(std::size_t index)
    {
        m_vertexIdx = index;
    }

    CORE_NODISCARD CORE_CONSTEXPR Vertex<T>& getNextVertex()
    {
        return getNext().getVertex();
    }

    CORE_NODISCARD CORE_CONSTEXPR const Vertex<T>& getNextVertex() const
    {
        return getNext().getVertex();
    }

    CORE_CONSTEXPR bool operator==(const Halfedge& rhs) const
    {
        return m_facetIdx == rhs.m_facetIdx && m_vertexIdx == rhs.m_vertexIdx &&
               m_nextIdx == rhs.m_nextIdx && m_previousIdx == rhs.m_previousIdx &&
               m_oppositeIdx == rhs.m_oppositeIdx && m_mesh == rhs.m_mesh;
    }

    CORE_CONSTEXPR bool operator!=(const Halfedge& rhs) const
    {
        return !(rhs == *this);
    }

    CORE_NODISCARD CORE_CONSTEXPR bool isValid() const
    {
        if (m_facetIdx != INVALID_INDEX && m_vertexIdx != INVALID_INDEX &&
            m_nextIdx != INVALID_INDEX && m_previousIdx != INVALID_INDEX && m_mesh->contains(*this))
            return true;
        return false;
    }

  private:
    std::size_t m_facetIdx;
    std::size_t m_vertexIdx;
    std::size_t m_nextIdx;
    std::size_t m_previousIdx;
    std::size_t m_oppositeIdx;
    HalfedgeMesh<T>* m_mesh;
};

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGE_H
