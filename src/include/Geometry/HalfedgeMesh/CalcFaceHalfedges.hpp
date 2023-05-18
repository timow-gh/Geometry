#ifndef GLFWTESTAPP_FACETUTILS_H
#define GLFWTESTAPP_FACETUTILS_H

#include <Core/Types/TVector.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
GEO_CONSTEXPR Core::TVector<Halfedge<TFloat, TIndex>> calcHalfedges(const Facet<TFloat, TIndex>& facet)
{
  Core::TVector<Halfedge<TFloat, TIndex>> result;

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

#endif // GLFWTESTAPP_FACETUTILS_H
