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
template <typename U, typename TFacet>
GEO_NODISCARD constexpr std::vector<U> calc_triangle_indices(const std::vector<TFacet>& facets)
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

/** @brief Calculates the point-indices of the lines for the given halfedge mesh.
 *
 * Each hafledge pair will be represented by a line.
 */
template <typename U, typename THalfedgeMesh>
GEO_NODISCARD constexpr std::vector<U> calc_line_indices(const THalfedgeMesh& mesh)
{
  using Halfedge_t = typename THalfedgeMesh::Halfedge_t;
  using index_type = typename THalfedgeMesh::index_type;

  struct Edge
  {
    U v0;
    U v1;

    static Edge create(index_type idx0, index_type idx1) noexcept
    {
      GEO_ASSERT(idx0 <= std::numeric_limits<U>::max());
      GEO_ASSERT(idx1 <= std::numeric_limits<U>::max());

      Edge edge{static_cast<U>(idx0), static_cast<U>(idx1)};
      if (edge.v0 > edge.v1)
      {
        std::swap(edge.v0, edge.v1);
      }

      return edge;
    }

    constexpr bool operator<(const Edge& other) const { return v0 < other.v0 || (v0 == other.v0 && v1 < other.v1); }
    constexpr bool operator==(const Edge& other) const { return (v0 == other.v0 && v1 == other.v1) || (v0 == other.v1 && v1 == other.v0); }
    constexpr bool operator!=(const Edge& other) const { return !(*this == other); }
  };

  std::vector<Edge> edges;
  for (const Halfedge_t& halfedge: mesh.getHalfedges())
  {
    Edge edge = Edge::create(halfedge.getVertexIndex().get_value(), halfedge.getNext().getVertexIndex().get_value());
    auto iter = std::lower_bound(edges.begin(), edges.end(), edge);
    if (iter == edges.end() || *iter != edge)
    {
      edges.insert(iter, edge);
    }
    GEO_ASSERT(std::is_sorted(edges.begin(), edges.end()));
  }

  return {reinterpret_cast<const U*>(edges.data()), reinterpret_cast<const U*>(edges.data() + edges.size())};
}

} // namespace Geometry

#endif // GEOMETRY_CALCTRIANGLEINDICES_HPP
