#ifndef GLFWTESTAPP_HALFEDGEMESH_H
#define GLFWTESTAPP_HALFEDGEMESH_H

#include <Core/Types/TArray.hpp>
#include <Core/Types/TVector.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <CrossGuid/Guid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshPoints.hpp>
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloat = double, typename TIndex = std::size_t>
class HalfedgeMesh {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;
  using FacetIndex_t = typename MeshIndexTraits<TIndex>::FacetIndex_t;

  using Vertex_t = Vertex<TFloat, TIndex>;
  using Halfedge_t = Halfedge<TFloat, TIndex>;
  using Facet_t = Facet<TFloat, TIndex>;

  using MeshPoints_t = MeshPoints<TFloat, TIndex>;

  GEO_CONSTEXPR HalfedgeMesh() = default;
  GEO_CONSTEXPR explicit HalfedgeMesh(const HalfedgeMesh& rhs) = delete;
  GEO_CONSTEXPR HalfedgeMesh& operator=(const HalfedgeMesh& rhs) = delete;

  GEO_CONSTEXPR HalfedgeMesh(HalfedgeMesh&& rhs) GEO_NOEXCEPT = default;
  GEO_CONSTEXPR HalfedgeMesh& operator=(HalfedgeMesh&& rhs) GEO_NOEXCEPT = default;

  // clang-format off
  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec3<TFloat> getVector(Vertex_t vertex) const { return meshPoints.getPoint(vertex.getIndex().getValue()); }

  GEO_NODISCARD GEO_CONSTEXPR Vertex_t getVertex(const VertexIndex_t vertexIndex) const { return vertices[vertexIndex.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR Halfedge_t getHalfedge(const HalfedgeIndex_t halfedgeIndex) const { return halfedges[halfedgeIndex.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR Facet_t getFacet(const FacetIndex_t facetIndex) const { return facets[facetIndex.getValue()]; }

  GEO_NODISCARD GEO_CONSTEXPR Vertex_t& getVertex(VertexIndex_t vertexIndex) { return vertices[vertexIndex.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR Halfedge_t& getHalfedge(HalfedgeIndex_t halfedgeIndex) { return halfedges[halfedgeIndex.getValue()]; }
  GEO_NODISCARD GEO_CONSTEXPR Facet_t& getFacet(FacetIndex_t facetIndex) { return facets[facetIndex.getValue()]; }

  GEO_NODISCARD GEO_CONSTEXPR LinAl::Vec3Vector<TFloat>& getPoints() { return meshPoints.getPoints(); }
  // clang-format on

  GEO_NODISCARD const Core::TVector<Vertex_t>& getVertices() const { return vertices; }
  GEO_NODISCARD Core::TVector<Vertex_t>& getVertices() { return vertices; }

  GEO_NODISCARD const Core::TVector<Halfedge_t>& getHalfedges() const { return halfedges; }
  GEO_NODISCARD Core::TVector<Halfedge_t>& getHalfedges() { return halfedges; }

  GEO_NODISCARD const Core::TVector<Facet_t>& getFacets() const { return facets; }
  GEO_NODISCARD Core::TVector<Facet_t>& getFacets() { return facets; }

  GEO_NODISCARD const MeshPoints_t& getMeshPoints() const { return meshPoints; }
  GEO_NODISCARD MeshPoints_t& getMeshPoints() { return meshPoints; }

  GEO_NODISCARD GEO_CONSTEXPR bool contains(const Vertex_t& vertex) const
  {
    for (const Vertex_t& v: vertices)
      if (vertex == v)
        return true;
    return false;
  }

  GEO_NODISCARD GEO_CONSTEXPR bool contains(const Halfedge_t& halfedge) const
  {
    for (const Halfedge_t& he: halfedges)
      if (halfedge == he)
        return true;
    return false;
  }

  GEO_NODISCARD GEO_CONSTEXPR bool contains(const Facet_t& facet) const
  {
    for (const Facet_t& f: facets)
      if (facet == f)
        return true;
    return false;
  }

  GEO_NODISCARD GEO_CONSTEXPR bool contains(const LinAl::Vec3<TFloat>& vector) const { return meshPoints.contains(vector); }

private:
  Core::TVector<Vertex_t> vertices;
  Core::TVector<Halfedge_t> halfedges;
  Core::TVector<Facet_t> facets;
  MeshPoints_t meshPoints;
};

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGEMESH_H
