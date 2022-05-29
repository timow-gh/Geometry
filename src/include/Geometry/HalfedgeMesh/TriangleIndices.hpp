#ifndef GEOMETRY_TRIANGLEINDICES_HPP
#define GEOMETRY_TRIANGLEINDICES_HPP

namespace Geometry
{

template <typename TFloatType, typename TIndexType, typename U>
CORE_CONSTEXPR Core::TVector<U> calcTriangleIndices(const Core::TVector<Facet<TFloatType, TIndexType>>& facets)
{
    Core::TVector<U> result;
    for (const auto& facet: facets)
        for (const Geometry::Halfedge<TFloatType, TIndexType> halfedge: Geometry::calcHalfedges(facet))
            result.push_back(static_cast<U>(halfedge.getVertexIndex().getValue()));
    return result;
}
} // namespace Geometry

#endif // GEOMETRY_TRIANGLEINDICES_HPP
