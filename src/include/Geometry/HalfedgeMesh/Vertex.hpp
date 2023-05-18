#ifndef GLFWTESTAPP_VERTEX_H
#define GLFWTESTAPP_VERTEX_H

#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshTraits.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/vec3.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class Vertex {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;

  using Halfedge_t = typename MeshTraits<TFloat, TIndex>::Halfedge_t;
  using HalfedgeMesh_t = HalfedgeMesh<TFloat, TIndex>;

  GEO_CONSTEXPR Vertex(VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) GEO_NOEXCEPT
      : m_vIndex(vertexIndex)
      , m_mesh(mesh)
  {
  }

  GEO_CONSTEXPR Vertex(VertexIndex_t meshPointIndex, HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t* mesh) GEO_NOEXCEPT
      : m_vIndex(meshPointIndex)
      , m_heIndex(halfedgeIndex)
      , m_mesh(mesh)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR linal::vec3<TFloat> getVector() const { return m_mesh->getVector(*this); }

  GEO_NODISCARD GEO_CONSTEXPR VertexIndex_t getIndex() const { return m_vIndex; }

  GEO_NODISCARD GEO_CONSTEXPR HalfedgeIndex_t getHalfedgeIndex() const { return m_heIndex; }
  GEO_CONSTEXPR void setHalfedgeIndex(HalfedgeIndex_t halfedgeIndex) { m_heIndex = halfedgeIndex; }

  GEO_NODISCARD GEO_CONSTEXPR const Halfedge_t& getHalfedge() const { return m_mesh->getHalfedges()[m_heIndex.getValue()]; }
  GEO_CONSTEXPR Halfedge_t& getHalfedge() { return m_mesh->getHalfedges()[m_heIndex.getValue()]; }

  GEO_NODISCARD GEO_CONSTEXPR bool isValid() const { return m_vIndex.isValid() && m_heIndex.isValid() && m_mesh; }

  GEO_CONSTEXPR bool operator==(const Vertex& rhs) const
  {
    return m_vIndex == rhs.m_vIndex && m_heIndex == rhs.m_heIndex && m_mesh == rhs.m_mesh;
  }

  GEO_CONSTEXPR bool operator!=(const Vertex& rhs) const { return !(rhs == *this); }

private:
  VertexIndex_t m_vIndex{};
  HalfedgeIndex_t m_heIndex{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GLFWTESTAPP_VERTEX_H
