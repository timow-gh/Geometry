#ifndef GEOMETRY_FACETUTILS_H
#define GEOMETRY_FACETUTILS_H

#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/Halfedge.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <vector>

namespace Geometry
{

/** @brief Calculate the halfedges of the given facet.
 *
 * @attention The halfedges follow the cw order with respect to the normal of the facet.
 */
template <typename TFloat, typename TIndex>
GEO_NODISCARD constexpr std::vector<Halfedge<TFloat, TIndex>> calc_halfedges(const Facet<TFloat, TIndex>& facet)
{
  std::vector<Halfedge<TFloat, TIndex>> result;

  Halfedge<TFloat, TIndex> halfedge = facet.getHalfedge();
  result.push_back(halfedge);
  halfedge = halfedge.getNext();
  result.push_back(halfedge);

  while (facet.getHalfedge() != halfedge.getNext())
  {
    halfedge = halfedge.getNext();
    result.push_back(halfedge);
  }

  return result;
}

} // namespace Geometry

#endif // GEOMETRY_FACETUTILS_H
