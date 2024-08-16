#ifndef ORIGAMI_CALCVERTEXFACETS_HPP
#define ORIGAMI_CALCVERTEXFACETS_HPP

#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include "Geometry/Utils/Assert.hpp"
#include <iostream>
#include <vector>

namespace Geometry
{

template <typename TVertex>
std::vector<typename TVertex::Facet_t> calc_vertex_facets(const TVertex& vertex)
{
  using Halfedge_t = typename TVertex::Halfedge_t;
  using Facet_t = typename TVertex::Facet_t;

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
