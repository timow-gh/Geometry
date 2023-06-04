#ifndef GEOMETRY_VERTEX_H
#define GEOMETRY_VERTEX_H

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

  constexpr Vertex(VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) noexcept
      : m_vIndex(vertexIndex)
      , m_mesh(mesh)
  {
  }

  constexpr Vertex(VertexIndex_t meshPointIndex, HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t* mesh) noexcept
      : m_vIndex(meshPointIndex)
      , m_heIndex(halfedgeIndex)
      , m_mesh(mesh)
  {
  }

  GEO_NODISCARD constexpr linal::vec3<TFloat> getVector() const { return m_mesh->getVector(*this); }

  GEO_NODISCARD constexpr VertexIndex_t getIndex() const { return m_vIndex; }

  GEO_NODISCARD constexpr HalfedgeIndex_t getHalfedgeIndex() const { return m_heIndex; }
  constexpr void setHalfedgeIndex(HalfedgeIndex_t halfedgeIndex) { m_heIndex = halfedgeIndex; }

  GEO_NODISCARD constexpr const Halfedge_t& getHalfedge() const { return m_mesh->getHalfedges()[m_heIndex.get_value()]; }
  constexpr Halfedge_t& getHalfedge() { return m_mesh->getHalfedges()[m_heIndex.get_value()]; }

  GEO_NODISCARD constexpr bool is_valid() const { return m_vIndex.is_valid() && m_heIndex.is_valid() && m_mesh; }

  constexpr bool operator==(const Vertex& rhs) const
  {
    return m_vIndex == rhs.m_vIndex && m_heIndex == rhs.m_heIndex && m_mesh == rhs.m_mesh;
  }

  constexpr bool operator!=(const Vertex& rhs) const { return !(rhs == *this); }

private:
  VertexIndex_t m_vIndex{};
  HalfedgeIndex_t m_heIndex{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GEOMETRY_VERTEX_H
