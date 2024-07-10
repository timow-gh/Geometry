#ifndef GEOMETRY_CALCTRIANGLEINDICES_HPP
#define GEOMETRY_CALCTRIANGLEINDICES_HPP

#include "Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"

namespace Geometry
{

template <typename TFloat, typename TIndex, typename U>
constexpr std::vector<U> calcTriangleIndices(const std::vector<Facet<TFloat, TIndex>>& facets)
{
  std::vector<U> result;
  for (const auto& facet: facets)
    for (const Geometry::Halfedge<TFloat, TIndex> halfedge: Geometry::calcHalfedges(facet))
      result.push_back(static_cast<U>(halfedge.getVertexIndex().get_value()));
  return result;
}

template <typename TFloat, typename TIndex, typename U>
constexpr std::vector<U> calcLineIndices(const HalfedgeMesh<float, std::uint32_t>& mesh)
{
  struct Edge
  {
    U v0;
    U v1;

    static Edge create(U v0, U v1) { return v0 < v1 ? Edge{v0, v1} : Edge{v1, v0}; }

    constexpr bool operator<(const Edge& other) const { return v0 < other.v0 || (v0 == other.v0 && v1 < other.v1); }
    constexpr bool operator==(const Edge& other) const { return (v0 == other.v0 && v1 == other.v1) || (v0 == other.v1 && v1 == other.v0); }
  };

  std::vector<Edge> edges;
  for (const Halfedge<TFloat, TIndex>& halfedge: mesh.getHalfedges())
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
