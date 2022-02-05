#ifndef GEOMETRY_TRIANGLEINDICES_H
#define GEOMETRY_TRIANGLEINDICES_H

namespace Geometry
{
template <typename T, typename U>
CORE_CONSTEXPR Core::TVector<U> calcTriangleIndices(const Core::TVector<Facet<T>>& facets)
{
    Core::TVector<U> result;
    for (const auto& facet: facets)
        for (const Geometry::Halfedge<T>* halfedge: Geometry::calcHalfedges(facet))
            result.push_back(static_cast<U>(halfedge->getVertexIndex()));
    return result;
}
} // namespace Geometry

#endif // GEOMETRY_TRIANGLEINDICES_H
