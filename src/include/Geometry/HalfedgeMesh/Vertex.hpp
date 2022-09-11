#ifndef GLFWTESTAPP_VERTEX_H
#define GLFWTESTAPP_VERTEX_H

#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshTraits.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
class Vertex {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndexType>::VertexIndex_t;
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndexType>::HalfedgeIndex_t;

  using Halfedge_t = typename MeshTraits<TFloatType, TIndexType>::Halfedge_t;
  using HalfedgeMesh_t = HalfedgeMesh<TFloatType, TIndexType>;

  CORE_CONSTEXPR Vertex(VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) CORE_NOEXCEPT
      : m_vIndex(vertexIndex)
      , m_mesh(mesh)
  {
  }

  CORE_CONSTEXPR Vertex(VertexIndex_t meshPointIndex, HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t* mesh) CORE_NOEXCEPT
      : m_vIndex(meshPointIndex)
      , m_heIndex(halfedgeIndex)
      , m_mesh(mesh)
  {
  }

  CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3<TFloatType> getVector() const { return m_mesh->getVector(*this); }

  CORE_NODISCARD CORE_CONSTEXPR VertexIndex_t getIndex() const { return m_vIndex; }

  CORE_NODISCARD CORE_CONSTEXPR HalfedgeIndex_t getHalfedgeIndex() const { return m_heIndex; }
  CORE_CONSTEXPR void setHalfedgeIndex(HalfedgeIndex_t halfedgeIndex) { m_heIndex = halfedgeIndex; }

  CORE_NODISCARD CORE_CONSTEXPR const Halfedge_t& getHalfedge() const { return m_mesh->halfedges[m_heIndex.getValue()]; }
  CORE_CONSTEXPR Halfedge_t& getHalfedge() { return m_mesh->halfedges[m_heIndex.getValue()]; }

  CORE_NODISCARD CORE_CONSTEXPR bool isValid() const { return m_vIndex.isValid() && m_heIndex.isValid() && m_mesh; }

  CORE_CONSTEXPR bool operator==(const Vertex& rhs) const
  {
    return m_vIndex == rhs.m_vIndex && m_heIndex == rhs.m_heIndex && m_mesh == rhs.m_mesh;
  }

  CORE_CONSTEXPR bool operator!=(const Vertex& rhs) const { return !(rhs == *this); }

private:
  VertexIndex_t m_vIndex{};
  HalfedgeIndex_t m_heIndex{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GLFWTESTAPP_VERTEX_H
