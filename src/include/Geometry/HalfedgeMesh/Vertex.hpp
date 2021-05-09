#ifndef GLFWTESTAPP_VERTEX_H
#define GLFWTESTAPP_VERTEX_H

#include "Geometry/HalfedgeMesh/Halfedge.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include <cstdio>

namespace Geometry {

template <typename T> class Halfedge;

template <typename T> struct HalfedgeMesh;

template <typename T> class Vertex {
public:
  explicit Vertex(std::size_t meshPointIndex, HalfedgeMesh<T> *mesh) noexcept;
  explicit Vertex(std::size_t meshPointIndex, std::size_t halfedgeIndex,
                  HalfedgeMesh<T> *mesh) noexcept;

  [[nodiscard]] LinAl::Vec3<T> vector3d() const;

  [[nodiscard]] const Halfedge<T> *halfedge() const;
  Halfedge<T> *halfedge();

  [[nodiscard]] std::size_t getHalfedgeIndex() const;
  void setHalfedgeIndex(std::size_t halfedgeIndex);

  bool operator==(const Vertex &rhs) const;
  bool operator!=(const Vertex &rhs) const;

  [[nodiscard]] bool isValid() const;

private:
  std::size_t m_vectorIndex;
  std::size_t m_halfedgeIndex;
  HalfedgeMesh<T> *m_mesh;
};

template <typename T>
Vertex<T>::Vertex(std::size_t meshPointIndex, HalfedgeMesh<T> *mesh) noexcept
    : m_vectorIndex(meshPointIndex), m_halfedgeIndex(Core::INVALID_INDEX),
      m_mesh(mesh) {}

template <typename T>
Vertex<T>::Vertex(std::size_t meshPointIndex, std::size_t halfedgeIndex,
                  HalfedgeMesh<T> *mesh) noexcept
    : m_vectorIndex(meshPointIndex), m_halfedgeIndex(halfedgeIndex),
      m_mesh(mesh) {}

template <typename T> LinAl::Vec3<T> Vertex<T>::vector3d() const {
  return m_mesh->getVertexPoints()[m_vectorIndex];
}

template <typename T> const Halfedge<T> *Vertex<T>::halfedge() const {
  return &m_mesh->getHalfedges()[m_halfedgeIndex];
}

template <typename T> Halfedge<T> *Vertex<T>::halfedge() {
  return &m_mesh->getHalfedges()[m_halfedgeIndex];
}

template <typename T> std::size_t Vertex<T>::getHalfedgeIndex() const {
  return m_halfedgeIndex;
}

template <typename T>
void Vertex<T>::setHalfedgeIndex(std::size_t halfedgeIndex) {
  m_halfedgeIndex = halfedgeIndex;
}
template <typename T> bool Vertex<T>::operator==(const Vertex &rhs) const {
  return m_vectorIndex == rhs.m_vectorIndex &&
         m_halfedgeIndex == rhs.m_halfedgeIndex && m_mesh == rhs.m_mesh;
}
template <typename T> bool Vertex<T>::operator!=(const Vertex &rhs) const {
  return !(rhs == *this);
}

template <typename T> bool Vertex<T>::isValid() const {
  if (m_vectorIndex != Core::INVALID_INDEX &&
      m_halfedgeIndex != Core::INVALID_INDEX && m_mesh->contains(*this))
    return true;
  return false;
}

} // namespace Geometry

#endif // GLFWTESTAPP_VERTEX_H
