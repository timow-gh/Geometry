#ifndef GLFWTESTAPP_VERTEX_H
#define GLFWTESTAPP_VERTEX_H

#include "HalfedgeMesh.hpp"
#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <cstdio>

namespace Geometry
{

template <typename T>
class Halfedge;

template <typename T>
struct HalfedgeMesh;

template <typename T>
class Vertex {
  public:
    Vertex(std::size_t meshPointIndex, HalfedgeMesh<T>* mesh) noexcept;
    Vertex(std::size_t meshPointIndex,
           std::size_t halfedgeIndex,
           HalfedgeMesh<T>* mesh) noexcept;

    CORE_NODISCARD LinAl::Vec3<T> getPoint() const;
    CORE_NODISCARD std::size_t getIndex() const;

    CORE_NODISCARD const Halfedge<T>* halfedge() const;
    Halfedge<T>* halfedge();

    CORE_NODISCARD std::size_t getHalfedgeIndex() const;
    void setHalfedgeIndex(std::size_t halfedgeIndex);

    bool operator==(const Vertex& rhs) const;
    bool operator!=(const Vertex& rhs) const;

    CORE_NODISCARD bool isValid() const;

  private:
    std::size_t m_index;
    std::size_t m_halfedgeIndex;
    HalfedgeMesh<T>* halfedgeMesh;
};

template <typename T>
Vertex<T>::Vertex(std::size_t meshPointIndex, HalfedgeMesh<T>* mesh) noexcept
    : m_index(meshPointIndex)
    , m_halfedgeIndex(INVALID_INDEX)
    , halfedgeMesh(mesh)
{
}

template <typename T>
Vertex<T>::Vertex(std::size_t meshPointIndex,
                  std::size_t halfedgeIndex,
                  HalfedgeMesh<T>* mesh) noexcept
    : m_index(meshPointIndex)
    , m_halfedgeIndex(halfedgeIndex)
    , halfedgeMesh(mesh)
{
}

template <typename T>
LinAl::Vec3<T> Vertex<T>::getPoint() const
{
    return halfedgeMesh->getVertexPoints()[m_index];
}

template <typename T>
size_t Vertex<T>::getIndex() const
{
    return m_index;
}

template <typename T>
const Halfedge<T>* Vertex<T>::halfedge() const
{
    return &halfedgeMesh->getHalfedges()[m_halfedgeIndex];
}

template <typename T>
Halfedge<T>* Vertex<T>::halfedge()
{
    return &halfedgeMesh->getHalfedges()[m_halfedgeIndex];
}

template <typename T>
std::size_t Vertex<T>::getHalfedgeIndex() const
{
    return m_halfedgeIndex;
}

template <typename T>
void Vertex<T>::setHalfedgeIndex(std::size_t halfedgeIndex)
{
    m_halfedgeIndex = halfedgeIndex;
}
template <typename T>
bool Vertex<T>::operator==(const Vertex& rhs) const
{
    return m_index == rhs.m_index && m_halfedgeIndex == rhs.m_halfedgeIndex &&
           halfedgeMesh == rhs.halfedgeMesh;
}
template <typename T>
bool Vertex<T>::operator!=(const Vertex& rhs) const
{
    return !(rhs == *this);
}

template <typename T>
bool Vertex<T>::isValid() const
{
    if (m_index != INVALID_INDEX && m_halfedgeIndex != INVALID_INDEX &&
        halfedgeMesh->contains(*this))
        return true;
    return false;
}

} // namespace Geometry

#endif // GLFWTESTAPP_VERTEX_H
