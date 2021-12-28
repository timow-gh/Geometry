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
struct HalfedgeMesh;

template <typename T>
class Facet {
  public:
    Facet(std::size_t halfedgeIndex, HalfedgeMesh<T>* mesh) CORE_NOEXCEPT;

    CORE_NODISCARD const Halfedge<T>& halfedge() const;
    Halfedge<T>& halfedge();

    CORE_NODISCARD std::size_t getHalfedgeIndex() const;

    bool operator==(const Facet& rhs) const;
    bool operator!=(const Facet& rhs) const;

    CORE_NODISCARD bool isValid() const;

  private:
    std::size_t m_halfedgeIndex;
    HalfedgeMesh<T>* m_mesh;
};

template <typename T>
Facet<T>::Facet(std::size_t halfedgeIndex, HalfedgeMesh<T>* mesh) CORE_NOEXCEPT
    : m_halfedgeIndex(halfedgeIndex)
    , m_mesh(mesh)
{
}
template <typename T>
const Halfedge<T>& Facet<T>::halfedge() const
{
    return m_mesh->getHalfedges()[m_halfedgeIndex];
}
template <typename T>
Halfedge<T>& Facet<T>::halfedge()
{
    return m_mesh->m_halfedges[m_halfedgeIndex];
}

template <typename T>
bool Facet<T>::operator==(const Facet& rhs) const
{
    return m_halfedgeIndex == rhs.m_halfedgeIndex && m_mesh == rhs.m_mesh;
}
template <typename T>
bool Facet<T>::operator!=(const Facet& rhs) const
{
    return !(rhs == *this);
}
template <typename T>
std::size_t Facet<T>::getHalfedgeIndex() const
{
    return m_halfedgeIndex;
}
template <typename T>
bool Facet<T>::isValid() const
{
    return m_halfedgeIndex != Geometry::INVALID_INDEX && m_mesh->contains(*this);
}

} // namespace Geometry

#endif