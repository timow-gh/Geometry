#ifndef ORIGAMI_CALCVERTEXFACETS_HPP
#define ORIGAMI_CALCVERTEXFACETS_HPP

#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include <vector>

namespace Geometry
{

template <typename TFloat, typename TIndex>
std::vector<Facet<TFloat, TIndex>> calcVertexFacets(const Vertex<TFloat, TIndex>& vertex)
{
  std::vector<Facet<TFloat, TIndex>> facets;
  Halfedge<TFloat, TIndex> he = vertex.getHalfedge();
  const Halfedge<TFloat, TIndex>& start = he;
  facets.push_back(he.getFacet());
  he = he.getOpposite();
  do
  {
    facets.push_back(he.getFacet());
    he = he.getNext().getOpposite();
  }
  while (he != start);

  return facets;
}

} // namespace Geometry
#endif // ORIGAMI_CALCVERTEXFACETS_HPP
