#ifndef GEOMETRY_HALFEDGEMESH_H
#define GEOMETRY_HALFEDGEMESH_H

#include <CrossGuid/Guid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshPoints.hpp>
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <array>
#include <linal/vec3.hpp>
#include <vector>

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

  constexpr HalfedgeMesh() = default;
  constexpr explicit HalfedgeMesh(const HalfedgeMesh& rhs) = delete;
  constexpr HalfedgeMesh& operator=(const HalfedgeMesh& rhs) = delete;

  constexpr HalfedgeMesh(HalfedgeMesh&& rhs) noexcept = default;
  constexpr HalfedgeMesh& operator=(HalfedgeMesh&& rhs) noexcept = default;

  // clang-format off
  GEO_NODISCARD constexpr linal::vec3<TFloat> getVector(Vertex_t vertex) const { return meshPoints.getPoint(vertex.getIndex().get_value()); }

  GEO_NODISCARD constexpr Vertex_t getVertex(const VertexIndex_t vertexIndex) const { return vertices[vertexIndex.get_value()]; }
  GEO_NODISCARD constexpr Halfedge_t getHalfedge(const HalfedgeIndex_t halfedgeIndex) const { return halfedges[halfedgeIndex.get_value()]; }
  GEO_NODISCARD constexpr Facet_t getFacet(const FacetIndex_t facetIndex) const { return facets[facetIndex.get_value()]; }

  GEO_NODISCARD constexpr Vertex_t& getVertex(VertexIndex_t vertexIndex) { return vertices[vertexIndex.get_value()]; }
  GEO_NODISCARD constexpr Halfedge_t& getHalfedge(HalfedgeIndex_t halfedgeIndex) { return halfedges[halfedgeIndex.get_value()]; }
  GEO_NODISCARD constexpr Facet_t& getFacet(FacetIndex_t facetIndex) { return facets[facetIndex.get_value()]; }

  GEO_NODISCARD constexpr linal::vec3vector<TFloat>& getPoints() { return meshPoints.getPoints(); }
  // clang-format on

  GEO_NODISCARD const std::vector<Vertex_t>& getVertices() const { return vertices; }
  GEO_NODISCARD std::vector<Vertex_t>& getVertices() { return vertices; }

  GEO_NODISCARD const std::vector<Halfedge_t>& getHalfedges() const { return halfedges; }
  GEO_NODISCARD std::vector<Halfedge_t>& getHalfedges() { return halfedges; }

  GEO_NODISCARD const std::vector<Facet_t>& getFacets() const { return facets; }
  GEO_NODISCARD std::vector<Facet_t>& getFacets() { return facets; }

  GEO_NODISCARD const MeshPoints_t& getMeshPoints() const { return meshPoints; }
  GEO_NODISCARD MeshPoints_t& getMeshPoints() { return meshPoints; }

  GEO_NODISCARD constexpr bool contains(const Vertex_t& vertex) const
  {
    for (const Vertex_t& v: vertices)
      if (vertex == v)
        return true;
    return false;
  }

  GEO_NODISCARD constexpr bool contains(const Halfedge_t& halfedge) const
  {
    for (const Halfedge_t& he: halfedges)
      if (halfedge == he)
        return true;
    return false;
  }

  GEO_NODISCARD constexpr bool contains(const Facet_t& facet) const
  {
    for (const Facet_t& f: facets)
      if (facet == f)
        return true;
    return false;
  }

  GEO_NODISCARD constexpr bool contains(const linal::vec3<TFloat>& vector) const { return meshPoints.contains(vector); }

private:
  std::vector<Vertex_t> vertices;
  std::vector<Halfedge_t> halfedges;
  std::vector<Facet_t> facets;
  MeshPoints_t meshPoints;
};

} // namespace Geometry

#endif // GEOMETRY_HALFEDGEMESH_H
