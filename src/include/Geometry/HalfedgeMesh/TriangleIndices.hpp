#ifndef GEOMETRY_TRIANGLEINDICES_HPP
#define GEOMETRY_TRIANGLEINDICES_HPP

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
} // namespace Geometry

#endif // GEOMETRY_TRIANGLEINDICES_HPP
