#ifndef GEOMETRY_TRIANGLEINDICES_HPP
#define GEOMETRY_TRIANGLEINDICES_HPP

namespace Geometry
{

template <typename TFloat, typename TIndex, typename U>
GEO_CONSTEXPR Core::TVector<U> calcTriangleIndices(const Core::TVector<Facet<TFloat, TIndex>>& facets)
{
  Core::TVector<U> result;
  for (const auto& facet: facets)
    for (const Geometry::Halfedge<TFloat, TIndex> halfedge: Geometry::calcHalfedges(facet))
      result.push_back(static_cast<U>(halfedge.getVertexIndex().getValue()));
  return result;
}
} // namespace Geometry

#endif // GEOMETRY_TRIANGLEINDICES_HPP
