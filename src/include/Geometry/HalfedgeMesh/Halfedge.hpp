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
        : m_facet(INVALID_INDEX)
        , m_vertex(vertexIndex)
        , m_next(INVALID_INDEX)
        , m_previous(INVALID_INDEX)
        , m_opposite(INVALID_INDEX)
        , m_mesh(mesh)
    {
    }

    CORE_NODISCARD
    CORE_CONSTEXPR const Facet<T>* facet() const
    {
        return &m_mesh->m_facets[m_facet];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getFacetIndex() const
    {
        return m_facet;
    }

    CORE_CONSTEXPR void setFacet(std::size_t facet)
    {
        m_facet = facet;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>* next() const
    {
        return &m_mesh->getHalfedges()[m_next];
    }

    CORE_NODISCARD CORE_CONSTEXPR Halfedge<T>* next()
    {
        return &m_mesh->getHalfedges()[m_next];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getNextIndex() const
    {
        return m_next;
    }

    CORE_CONSTEXPR void setNextIndex(std::size_t index)
    {
        m_next = index;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>* previous() const
    {
        return &m_mesh->getHalfedges()[m_previous];
    }

    CORE_NODISCARD CORE_CONSTEXPR Halfedge<T>* previous()
    {
        return &m_mesh->getHalfedges()[m_previous];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getPreviousIndex() const
    {
        return m_previous;
    }

    CORE_CONSTEXPR void setPreviousIndex(std::size_t index)
    {
        m_previous = index;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>* opposite() const
    {
        return &m_mesh->getHalfedges()[m_opposite];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getOppositeIndex() const
    {
        return m_opposite;
    }

    CORE_CONSTEXPR void setOppositeIndex(std::size_t index)
    {
        m_opposite = index;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Vertex<T>& vertex() const
    {
        return m_mesh->getVertices()[m_vertex];
    }

    CORE_NODISCARD CORE_CONSTEXPR Vertex<T>& vertex()
    {
        return m_mesh->getVertices()[m_vertex];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getVertexIndex() const
    {
        return m_vertex;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Vertex<T>* nextVertex() const
    {
        if (!next())
            return nullptr;
        return &next()->vertex();
    }

    CORE_CONSTEXPR bool operator==(const Halfedge& rhs) const
    {
        return m_facet == rhs.m_facet && m_vertex == rhs.m_vertex && m_next == rhs.m_next &&
               m_previous == rhs.m_previous && m_opposite == rhs.m_opposite && m_mesh == rhs.m_mesh;
    }

    CORE_CONSTEXPR bool operator!=(const Halfedge& rhs) const
    {
        return !(rhs == *this);
    }

    CORE_NODISCARD CORE_CONSTEXPR bool isValid() const
    {
        if (m_facet != INVALID_INDEX && m_vertex != INVALID_INDEX && m_next != INVALID_INDEX &&
            m_previous != INVALID_INDEX && m_mesh->contains(*this))
            return true;
        return false;
    }

  private:
    std::size_t m_facet;
    std::size_t m_vertex;
    std::size_t m_next;
    std::size_t m_previous;
    std::size_t m_opposite;
    HalfedgeMesh<T>* m_mesh;
};

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGE_H
