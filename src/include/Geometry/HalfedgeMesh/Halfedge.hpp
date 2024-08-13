#ifndef GEOMETRY_HALFEDGE_H
#define GEOMETRY_HALFEDGE_H

#include "Geometry/HalfedgeMesh/MeshIndexTraits.hpp"
#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"

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

  /** @brief Creates a halfedge that must be completed later.
   *
   * Intended for use during the building of the mesh, where the complete of a halfedge is not yet known.
   *
   * @param halfedgeIndex
   * @param vertexIndex
   * @param mesh
   */
  constexpr Halfedge(HalfedgeIndex_t halfedgeIndex, VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) noexcept
      : m_halfedgeIdx(halfedgeIndex)
      , m_vertexIdx(vertexIndex)
      , m_mesh(mesh)
  {
  }

  GEO_NODISCARD
  constexpr const Facet_t& getFacet() const { return m_mesh->getFacets()[m_facetIdx.get_value()]; }

  GEO_NODISCARD constexpr const Vertex_t& getVertex() const { return m_mesh->getVertices()[m_vertexIdx.get_value()]; }
  GEO_NODISCARD constexpr Vertex_t& getVertex() { return m_mesh->getVertices()[m_vertexIdx.get_value()]; }

  GEO_NODISCARD constexpr const Vertex_t& getNextVertex() const { return getNext().getVertex(); }
  GEO_NODISCARD constexpr Vertex_t& getNextVertex() { return getNext().getVertex(); }

  GEO_NODISCARD constexpr VertexIndex_t getVertexIndex() const { return m_vertexIdx; }
  constexpr void setVertexIndex(VertexIndex_t index) { m_vertexIdx = index; }

  GEO_NODISCARD constexpr HalfedgeIndex_t getHalfedgeIndex() const { return m_halfedgeIdx; }
  GEO_NODISCARD constexpr HalfedgeIndex_t getNextIndex() const { return m_nextIdx; }
  GEO_NODISCARD constexpr HalfedgeIndex_t getPreviousIndex() const { return m_previousIdx; }
  GEO_NODISCARD constexpr HalfedgeIndex_t getOppositeIndex() const { return m_oppositeIdx; }
  GEO_NODISCARD constexpr FacetIndex_t getFacetIndex() const { return m_facetIdx; }

  constexpr void setNextIndex(HalfedgeIndex_t index) { m_nextIdx = index; }
  constexpr void setPreviousIndex(HalfedgeIndex_t index) { m_previousIdx = index; }
  constexpr void setOppositeIndex(HalfedgeIndex_t index) { m_oppositeIdx = index; }
  constexpr void setFacetIndex(FacetIndex_t facetIndex) { m_facetIdx = facetIndex; }

  GEO_NODISCARD constexpr const Halfedge_t& getNext() const { return m_mesh->getHalfedges()[m_nextIdx.get_value()]; }
  GEO_NODISCARD constexpr const Halfedge_t& getPrevious() const { return m_mesh->getHalfedges()[m_previousIdx.get_value()]; }
  GEO_NODISCARD constexpr const Halfedge_t& getOpposite() const { return m_mesh->getHalfedges()[m_oppositeIdx.get_value()]; }

  GEO_NODISCARD constexpr Halfedge_t& getNext() { return m_mesh->getHalfedges()[m_nextIdx.get_value()]; }
  GEO_NODISCARD constexpr Halfedge_t& getPrevious() { return m_mesh->getHalfedges()[m_previousIdx.get_value()]; }

  constexpr bool operator==(const Halfedge& rhs) const { return m_halfedgeIdx == rhs.m_halfedgeIdx; }
  constexpr bool operator!=(const Halfedge& rhs) const { return !(rhs == *this); }

  GEO_NODISCARD constexpr bool is_valid() const
  {
    bool isValid = is_valid_halfedge_impl();
    GEO_ASSERT(isValid);
    return isValid;
  }

private:
  GEO_NODISCARD constexpr bool is_valid_halfedge_impl() const
  {
    if (!(m_halfedgeIdx.is_valid() && m_vertexIdx.is_valid() && m_facetIdx.is_valid() && m_nextIdx.is_valid() && m_previousIdx.is_valid() &&
          m_oppositeIdx.is_valid() && m_mesh != nullptr))
    {
      return false;
    }

    const Vertex_t& vertex = getVertex();
    const auto& heIndices = vertex.getHalfedgeIndices();
    if (std::find(heIndices.begin(), heIndices.end(), m_halfedgeIdx) == heIndices.end())
    {
      return false;
    }

    Halfedge_t oppHe = getOpposite();
    return oppHe.m_oppositeIdx == m_halfedgeIdx;
  }

  HalfedgeIndex_t m_halfedgeIdx{};
  FacetIndex_t m_facetIdx{};
  VertexIndex_t m_vertexIdx{};
  HalfedgeIndex_t m_nextIdx{};
  HalfedgeIndex_t m_previousIdx{};
  HalfedgeIndex_t m_oppositeIdx{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GEOMETRY_HALFEDGE_H
