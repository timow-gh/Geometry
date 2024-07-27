#ifndef ORIGAMI_CALCVERTEXFACETS_HPP
#define ORIGAMI_CALCVERTEXFACETS_HPP

#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include "Geometry/Utils/Assert.hpp"
#include <iostream>
#include <vector>

namespace Geometry
{

template <typename TFloat, typename TIndex>
std::vector<Facet<TFloat, TIndex>> calc_vertex_facets(const Vertex<TFloat, TIndex>& vertex)
{
  using Halfedge_t = Halfedge<TFloat, TIndex>;
  using Facet_t = Facet<TFloat, TIndex>;

  std::vector<Facet_t> facets;
  for (const Halfedge_t& halfedge: vertex.calcHalfedges())
  {
    const Facet_t facet = halfedge.getFacet();
    facets.push_back(facet);
  }

  return facets;
}

} // namespace Geometry
#endif // ORIGAMI_CALCVERTEXFACETS_HPP
