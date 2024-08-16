#ifndef GEOMETRY_CALCTRIANGLEINDICES_HPP
#define GEOMETRY_CALCTRIANGLEINDICES_HPP

#include "Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"

namespace Geometry
{

/** @brief Calculate the indices of triangles for the given facets.
 *
 * @attention The triangle indices will be ccw with respect to the normal of the facet.
 */
template <typename TFacet, typename U>
GEO_NODISCARD constexpr std::vector<U> calcTriangleIndices(const std::vector<TFacet>& facets)
{
  using Halfedge_t = typename TFacet::Halfedge_t;

  std::vector<U> result;

  for (const auto& facet: facets)
  {
    std::vector<Halfedge_t> halfedges;

    Halfedge_t halfedge = facet.getHalfedge();
    halfedges.push_back(halfedge);
    halfedge = halfedge.getNext();
    halfedges.push_back(halfedge);

    while (facet.getHalfedge() != halfedge.getNext())
    {
      halfedge = halfedge.getNext();
      halfedges.push_back(halfedge);
    }

    for (const Halfedge_t& he: halfedges)
    {
      result.push_back(static_cast<U>(he.getVertexIndex().get_value()));
    }
  }

  return result;
}

template <typename THalfedgeMesh, typename U>
GEO_NODISCARD constexpr std::vector<U> calcLineIndices(const THalfedgeMesh& mesh)
{
  using Halfedge_t = typename THalfedgeMesh::Halfedge_t;

  struct Edge
  {
    U v0;
    U v1;

    static Edge create(U v0, U v1) { return v0 < v1 ? Edge{v0, v1} : Edge{v1, v0}; }

    constexpr bool operator<(const Edge& other) const { return v0 < other.v0 || (v0 == other.v0 && v1 < other.v1); }
    constexpr bool operator==(const Edge& other) const { return (v0 == other.v0 && v1 == other.v1) || (v0 == other.v1 && v1 == other.v0); }
  };

  std::vector<Edge> edges;
  for (const Halfedge_t& halfedge: mesh.getHalfedges())
  {
    const U v0 = halfedge.getVertexIndex().get_value();
    const U v1 = halfedge.getNext().getVertexIndex().get_value();
    Edge edge = Edge::create(v0, v1);
    auto iter = std::lower_bound(edges.begin(), edges.end(), edge);
    if (iter == edges.end() || *iter != edge)
    {
      edges.insert(iter, edge);
    }
    assert(std::is_sorted(edges.begin(), edges.end()));
  }

  return {reinterpret_cast<const U*>(edges.data()), reinterpret_cast<const U*>(edges.data() + edges.size())};
}

} // namespace Geometry

#endif // GEOMETRY_CALCTRIANGLEINDICES_HPP
