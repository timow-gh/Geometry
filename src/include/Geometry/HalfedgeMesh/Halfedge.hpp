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
    Halfedge(std::size_t vertexIndex, HalfedgeMesh<T>* mesh) CORE_NOEXCEPT;

    CORE_NODISCARD const Facet<T>* facet() const;
    CORE_NODISCARD std::size_t getFacetIndex() const;
    void setFacet(std::size_t facet);

    CORE_NODISCARD const Halfedge<T>* next() const;
    CORE_NODISCARD Halfedge<T>* next();
    CORE_NODISCARD std::size_t getNextIndex() const;
    void setNextIndex(std::size_t index);

    CORE_NODISCARD const Halfedge<T>* previous() const;
    CORE_NODISCARD Halfedge<T>* previous();
    CORE_NODISCARD std::size_t getPreviousIndex() const;
    void setPreviousIndex(std::size_t index);

    CORE_NODISCARD const Halfedge<T>* opposite() const;
    CORE_NODISCARD std::size_t getOppositeIndex() const;
    void setOppositeIndex(std::size_t index);

    CORE_NODISCARD const Vertex<T>& vertex() const;
    CORE_NODISCARD Vertex<T>& vertex();
    CORE_NODISCARD std::size_t getVertexIndex() const;

    CORE_NODISCARD const Vertex<T>* nextVertex() const;

    bool operator==(const Halfedge& rhs) const;
    bool operator!=(const Halfedge& rhs) const;

    CORE_NODISCARD bool isValid() const;

  private:
    std::size_t m_facet;
    std::size_t m_vertex;
    std::size_t m_next;
    std::size_t m_previous;
    std::size_t m_opposite;
    HalfedgeMesh<T>* m_mesh;
};

template <typename T>
Halfedge<T>::Halfedge(std::size_t vertexIndex, HalfedgeMesh<T>* mesh) CORE_NOEXCEPT
    : m_facet(INVALID_INDEX)
    , m_vertex(vertexIndex)
    , m_next(INVALID_INDEX)
    , m_previous(INVALID_INDEX)
    , m_opposite(INVALID_INDEX)
    , m_mesh(mesh)
{
}

template <typename T>
const Facet<T>* Halfedge<T>::facet() const
{
    return &m_mesh->m_facets[m_facet];
}

template <typename T>
std::size_t Halfedge<T>::getFacetIndex() const
{
    return m_facet;
}

template <typename T>
void Halfedge<T>::setFacet(std::size_t facet)
{
    m_facet = facet;
}

template <typename T>
const Halfedge<T>* Halfedge<T>::next() const
{
    return &m_mesh->getHalfedges()[m_next];
}

template <typename T>
Halfedge<T>* Halfedge<T>::next()
{
    return &m_mesh->getHalfedges()[m_next];
}

template <typename T>
std::size_t Halfedge<T>::getNextIndex() const
{
    return m_next;
}

template <typename T>
void Halfedge<T>::setNextIndex(std::size_t index)
{
    m_next = index;
}

template <typename T>
const Halfedge<T>* Halfedge<T>::previous() const
{
    return &m_mesh->getHalfedges()[m_previous];
}

template <typename T>
Halfedge<T>* Halfedge<T>::previous()
{
    return &m_mesh->getHalfedges()[m_previous];
}

template <typename T>
std::size_t Halfedge<T>::getPreviousIndex() const
{
    return m_previous;
}

template <typename T>
void Halfedge<T>::setPreviousIndex(std::size_t index)
{
    m_previous = index;
}

template <typename T>
const Halfedge<T>* Halfedge<T>::opposite() const
{
    return &m_mesh->getHalfedges()[m_opposite];
}

template <typename T>
std::size_t Halfedge<T>::getOppositeIndex() const
{
    return m_opposite;
}

template <typename T>
void Halfedge<T>::setOppositeIndex(std::size_t index)
{
    m_opposite = index;
}
template <typename T>
const Vertex<T>& Halfedge<T>::vertex() const
{
    return m_mesh->getVertices()[m_vertex];
}

template <typename T>
Vertex<T>& Halfedge<T>::vertex()
{
    return m_mesh->getVertices()[m_vertex];
}

template <typename T>
std::size_t Halfedge<T>::getVertexIndex() const
{
    return m_vertex;
}

template <typename T>
const Vertex<T>* Halfedge<T>::nextVertex() const
{
    if (!next())
        return nullptr;
    return &next()->vertex();
}

template <typename T>
bool Halfedge<T>::operator==(const Halfedge& rhs) const
{
    return m_facet == rhs.m_facet && m_vertex == rhs.m_vertex && m_next == rhs.m_next &&
           m_previous == rhs.m_previous && m_opposite == rhs.m_opposite && m_mesh == rhs.m_mesh;
}

template <typename T>
bool Halfedge<T>::operator!=(const Halfedge& rhs) const
{
    return !(rhs == *this);
}

template <typename T>
bool Halfedge<T>::isValid() const
{
    if (m_facet != INVALID_INDEX && m_vertex != INVALID_INDEX && m_next != INVALID_INDEX &&
        m_previous != INVALID_INDEX && m_mesh->contains(*this))
        return true;
    return false;
}

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGE_H
