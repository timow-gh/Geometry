#ifndef GLFWTESTAPP_HALFEDGE_H
#define GLFWTESTAPP_HALFEDGE_H

#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshTraits.hpp>

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
class Halfedge {
  public:
    using VertexIndex_t = typename MeshIndexTraits<TIndexType>::VertexIndex_t;
    using HalfedgeIndex_t = typename MeshIndexTraits<TIndexType>::HalfedgeIndex_t;
    using FacetIndex_t = typename MeshIndexTraits<TIndexType>::FacetIndex_t;

    using MeshTraits = MeshTraits<TFloatType, TIndexType>;
    using Vertex_t = typename MeshTraits::Vertex_t;
    using Halfedge_t = typename MeshTraits::Halfedge_t;
    using Facet_t = typename MeshTraits::Facet_t;

    using HalfedgeMesh_t = HalfedgeMesh<TFloatType, TIndexType>;

    CORE_CONSTEXPR Halfedge(VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) CORE_NOEXCEPT
        : m_vertexIdx(vertexIndex)
        , m_mesh(mesh)
    {
    }

    CORE_NODISCARD
    CORE_CONSTEXPR const Facet_t& getFacet() const { return m_mesh->facets[m_facetIdx.getValue()]; }

    CORE_NODISCARD CORE_CONSTEXPR const Vertex_t& getVertex() const { return m_mesh->vertices[m_vertexIdx.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR const Vertex_t& getNextVertex() const { return getNext().getVertex(); }

    CORE_NODISCARD CORE_CONSTEXPR Vertex_t& getNextVertex() { return getNext().getVertex(); }
    CORE_NODISCARD CORE_CONSTEXPR Vertex_t& getVertex() { return m_mesh->vertices[m_vertexIdx.getValue()]; }

    CORE_NODISCARD CORE_CONSTEXPR VertexIndex_t getVertexIndex() const { return m_vertexIdx; }
    CORE_CONSTEXPR void setVertexIndex(VertexIndex_t index) { m_vertexIdx = index; }

    CORE_NODISCARD CORE_CONSTEXPR HalfedgeIndex_t getNextIndex() const { return m_nextIdx; }
    CORE_NODISCARD CORE_CONSTEXPR HalfedgeIndex_t getPreviousIndex() const { return m_previousIdx; }
    CORE_NODISCARD CORE_CONSTEXPR HalfedgeIndex_t getOppositeIndex() const { return m_oppositeIdx; }
    CORE_NODISCARD CORE_CONSTEXPR FacetIndex_t getFacetIndex() const { return m_facetIdx; }

    CORE_CONSTEXPR void setNextIndex(HalfedgeIndex_t index) { m_nextIdx = index; }
    CORE_CONSTEXPR void setPreviousIndex(HalfedgeIndex_t index) { m_previousIdx = index; }
    CORE_CONSTEXPR void setOppositeIndex(HalfedgeIndex_t index) { m_oppositeIdx = index; }
    CORE_CONSTEXPR void setFacetIndex(FacetIndex_t facetIndex) { m_facetIdx = facetIndex; }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge_t& getNext() const { return m_mesh->halfedges[m_nextIdx.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR const Halfedge_t& getPrevious() const { return m_mesh->halfedges[m_previousIdx.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR const Halfedge_t& getOpposite() const { return m_mesh->halfedges[m_oppositeIdx.getValue()]; }

    CORE_NODISCARD CORE_CONSTEXPR Halfedge_t& getNext() { return m_mesh->halfedges[m_nextIdx.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR Halfedge_t& getPrevious() { return m_mesh->halfedges[m_previousIdx.getValue()]; }

    CORE_CONSTEXPR bool operator==(const Halfedge& rhs) const
    {
        return m_facetIdx == rhs.m_facetIdx && m_vertexIdx == rhs.m_vertexIdx && m_nextIdx == rhs.m_nextIdx &&
               m_previousIdx == rhs.m_previousIdx && m_oppositeIdx == rhs.m_oppositeIdx && m_mesh == rhs.m_mesh;
    }

    CORE_CONSTEXPR bool operator!=(const Halfedge& rhs) const { return !(rhs == *this); }

    CORE_NODISCARD CORE_CONSTEXPR bool isValid() const { return m_mesh->contains(*this); }

  private:
    FacetIndex_t m_facetIdx{};
    VertexIndex_t m_vertexIdx{};
    HalfedgeIndex_t m_nextIdx{};
    HalfedgeIndex_t m_previousIdx{};
    HalfedgeIndex_t m_oppositeIdx{};
    HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGE_H
