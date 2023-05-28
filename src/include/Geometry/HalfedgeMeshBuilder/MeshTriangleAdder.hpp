#ifndef GEOMETRY_MESHTRIANGLEADDER_HPP
#define GEOMETRY_MESHTRIANGLEADDER_HPP

#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/Triangle.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class MeshTriangleAdder {
  HalfedgeMesh<TFloat, TIndex>* m_halfedgeMesh;

public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;
  using FacetIndex_t = typename MeshIndexTraits<TIndex>::FacetIndex_t;

  using Vertex_t = Vertex<TFloat, TIndex>;
  using Halfedge_t = Halfedge<TFloat, TIndex>;
  using Facet_t = Facet<TFloat, TIndex>;

  using MeshPoints_t = MeshPoints<TFloat, TIndex>;

  constexpr explicit MeshTriangleAdder(HalfedgeMesh<TFloat, TIndex>& halfedgeMesh) noexcept : m_halfedgeMesh(&halfedgeMesh) {}

  void operator()(const Triangle<TFloat, 3>& triangle)
  {
    MeshPoints_t& meshPoints = m_halfedgeMesh->getMeshPoints();
    std::vector<Vertex_t>& vertices = m_halfedgeMesh->getVertices();
    std::vector<Halfedge_t>& halfedges = m_halfedgeMesh->getHalfedges();
    std::vector<Facet_t>& facets = m_halfedgeMesh->getFacets();

    HalfedgeIndex_t halfedgeIndex{halfedges.size() == 0 ? 0 : halfedges.size()};

    std::array<VertexIndex_t, 3> vertexIndices;
    create_or_find_vertex(triangle, meshPoints, vertices, vertexIndices);
    create_halfedge_and_set_vertex(vertexIndices, vertices, halfedges, m_halfedgeMesh);
    facets.emplace_back(halfedgeIndex, *m_halfedgeMesh);
    flil_halfedges_of_facet(facets, halfedgeIndex, halfedges);
    set_opposite_halfedges(halfedgeIndex, halfedges);
  }

private:
  void create_or_find_vertex(const Triangle<TFloat, 3>& triangle,
                          MeshPoints_t& meshPoints,
                          std::vector<Vertex_t>& vertices,
                          std::array<VertexIndex_t, 3>& vertexIndices) const
  {
    // Create or find the Vertex of the linal::vec3
    const linal::vecArray<TFloat, 3, 3> trianglePoints = triangle.get_triangle_points();
    for (std::size_t i = 0; i < 3; ++i)
    {
      VertexIndex_t vertexIndex;
      if (!meshPoints.contains(trianglePoints[i], vertexIndex))
      {
        vertexIndex = meshPoints.add(trianglePoints[i]);
        vertices.emplace_back(vertexIndex, m_halfedgeMesh);
      }
      vertexIndices[i] = vertexIndex;
    }
  }

  void create_halfedge_and_set_vertex(const std::array<VertexIndex_t, 3>& vertexIndices,
                                  std::vector<Vertex_t>& vertices,
                                  std::vector<Halfedge_t>& halfedges,
                                  HalfedgeMesh<TFloat, TIndex>* halfedgeMesh) const
  {
    // Create the Halfedges and set the Halfedges for Vertices
    for (const VertexIndex_t vIndex: vertexIndices)
    {
      halfedges.emplace_back(vIndex, halfedgeMesh);
      if (!vertices[vIndex.get_value()].getHalfedgeIndex().is_valid())
        vertices[vIndex.get_value()].setHalfedgeIndex(HalfedgeIndex_t{halfedges.size() - 1});
    }
  }

  void flil_halfedges_of_facet(std::vector<Facet_t>& facets, HalfedgeIndex_t halfedgeIndex, std::vector<Halfedge_t>& halfedges) const
  {
    // Fill the facet, the next and the previous pointer for each Halfedge
    // of the Facet
    const FacetIndex_t facetIdx = FacetIndex_t(facets.size() - 1);
    for (TIndex i = halfedgeIndex.get_value(); i < halfedges.size(); ++i)
    {
      halfedges[i].setFacetIndex(facetIdx);
      TIndex nextHeIndex = i == halfedges.size() - 1 ? halfedgeIndex.get_value() : i + 1;
      halfedges[i].setNextIndex(HalfedgeIndex_t{nextHeIndex});
      TIndex previousHeIndex = i == halfedgeIndex.get_value() ? halfedges.size() - 1 : i - 1;
      halfedges[i].setPreviousIndex(HalfedgeIndex_t{previousHeIndex});
    }
  }

  void set_opposite_halfedges(HalfedgeIndex_t halfedgeIndex, std::vector<Halfedge_t>& halfedges) const
  {
    // Find and set the opposite Halfedges for each Halfedge of the Facet
    for (std::size_t i = halfedgeIndex.get_value(); i < halfedges.size(); ++i)
    {
      Halfedge_t& halfedge = halfedges[i];

      Vertex_t& heVertex = halfedge.getVertex();
      Halfedge_t& he = heVertex.getHalfedge();

      Halfedge_t& oppHeCandidate = he.getPrevious();
      HalfedgeIndex_t oppHeCandidateIndex = heVertex.getHalfedge().getPreviousIndex();

      Vertex_t& nextVertex = halfedge.getNextVertex();
      Vertex_t& oppNextVertex = oppHeCandidate.getNextVertex();

      if (heVertex == oppNextVertex && nextVertex == oppHeCandidate.getVertex())
      {
        halfedge.setOppositeIndex(oppHeCandidateIndex);
        oppHeCandidate.setOppositeIndex(HalfedgeIndex_t{i});
      }
    }
  }
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRIANGLEADDER_HPP
