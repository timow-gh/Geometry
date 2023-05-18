#ifndef GEOMETRY_HALFEDGE_H
#define GEOMETRY_HALFEDGE_H

#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshTraits.hpp>
#include <Geometry/Utils/Compiler.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class Halfedge {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;
  using FacetIndex_t = typename MeshIndexTraits<TIndex>::FacetIndex_t;

  using HeMeshTraits = MeshTraits<TFloat, TIndex>;
  using Vertex_t = typename HeMeshTraits::Vertex_t;
  using Halfedge_t = typename HeMeshTraits::Halfedge_t;
  using Facet_t = typename HeMeshTraits::Facet_t;

  using HalfedgeMesh_t = HalfedgeMesh<TFloat, TIndex>;

  GEO_CONSTEXPR Halfedge(VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) GEO_NOEXCEPT
      : m_vertexIdx(vertexIndex)
      , m_mesh(mesh)
  {
  }

  GEO_NODISCARD
  GEO_CONSTEXPR const Facet_t& getFacet() const { return m_mesh->getFacets()[m_facetIdx.getValue()]; }

  GEO_NODISCARD GEO_CONSTEXPR const Vertex_t& getVertex() const { return m_mesh->getVertices()[m_vertexIdx.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR Vertex_t& getVertex() { return m_mesh->getVertices()[m_vertexIdx.getValue()]; }

  GEO_NODISCARD GEO_CONSTEXPR const Vertex_t& getNextVertex() const { return getNext().getVertex(); }
  GEO_NODISCARD GEO_CONSTEXPR Vertex_t& getNextVertex() { return getNext().getVertex(); }

  GEO_NODISCARD GEO_CONSTEXPR VertexIndex_t getVertexIndex() const { return m_vertexIdx; }
  GEO_CONSTEXPR void setVertexIndex(VertexIndex_t index) { m_vertexIdx = index; }

  GEO_NODISCARD GEO_CONSTEXPR HalfedgeIndex_t getNextIndex() const { return m_nextIdx; }
  GEO_NODISCARD GEO_CONSTEXPR HalfedgeIndex_t getPreviousIndex() const { return m_previousIdx; }
  GEO_NODISCARD GEO_CONSTEXPR HalfedgeIndex_t getOppositeIndex() const { return m_oppositeIdx; }
  GEO_NODISCARD GEO_CONSTEXPR FacetIndex_t getFacetIndex() const { return m_facetIdx; }

  GEO_CONSTEXPR void setNextIndex(HalfedgeIndex_t index) { m_nextIdx = index; }
  GEO_CONSTEXPR void setPreviousIndex(HalfedgeIndex_t index) { m_previousIdx = index; }
  GEO_CONSTEXPR void setOppositeIndex(HalfedgeIndex_t index) { m_oppositeIdx = index; }
  GEO_CONSTEXPR void setFacetIndex(FacetIndex_t facetIndex) { m_facetIdx = facetIndex; }

  GEO_NODISCARD GEO_CONSTEXPR const Halfedge_t& getNext() const { return m_mesh->getHalfedges()[m_nextIdx.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR const Halfedge_t& getPrevious() const { return m_mesh->getHalfedges()[m_previousIdx.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR const Halfedge_t& getOpposite() const { return m_mesh->getHalfedges()[m_oppositeIdx.getValue()]; }

  GEO_NODISCARD GEO_CONSTEXPR Halfedge_t& getNext() { return m_mesh->getHalfedges()[m_nextIdx.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR Halfedge_t& getPrevious() { return m_mesh->getHalfedges()[m_previousIdx.getValue()]; }

  GEO_CONSTEXPR bool operator==(const Halfedge& rhs) const
  {
    return m_facetIdx == rhs.m_facetIdx && m_vertexIdx == rhs.m_vertexIdx && m_nextIdx == rhs.m_nextIdx &&
           m_previousIdx == rhs.m_previousIdx && m_oppositeIdx == rhs.m_oppositeIdx && m_mesh == rhs.m_mesh;
  }

  GEO_CONSTEXPR bool operator!=(const Halfedge& rhs) const { return !(rhs == *this); }

  GEO_NODISCARD GEO_CONSTEXPR bool isValid() const { return m_mesh->contains(*this); }

private:
  FacetIndex_t m_facetIdx{};
  VertexIndex_t m_vertexIdx{};
  HalfedgeIndex_t m_nextIdx{};
  HalfedgeIndex_t m_previousIdx{};
  HalfedgeIndex_t m_oppositeIdx{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GEOMETRY_HALFEDGE_H
