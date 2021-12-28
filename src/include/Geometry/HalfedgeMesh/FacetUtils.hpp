#ifndef GLFWTESTAPP_FACETUTILS_H
#define GLFWTESTAPP_FACETUTILS_H

#include <Core/Types/TVector.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>

namespace Geometry
{

template <typename T>
Core::TVector<const Halfedge<T>*> calcHalfedges(const Facet<T>& facet);

template <typename T>
Core::TVector<const Halfedge<T>*> calcHalfedges(const Facet<T>& facet)
{
    Core::TVector<const Halfedge<T>*> result;

    const Halfedge<T>* halfedge = &facet.halfedge();
    result.push_back(halfedge);
    halfedge = halfedge->next();
    result.push_back(halfedge);

    while (facet.halfedge() != *halfedge->next())
    {
        halfedge = halfedge->next();
        result.push_back(halfedge);
    };
    return result;
}

template <typename T>
Core::TVector<Vertex<T>>
calcVertices(const Core::TVector<const Halfedge<T>*>& halfedges)
{
    Core::TVector<Vertex<T>> result;
    for (const Halfedge<T>* halfedge: halfedges)
        result.push_back(halfedge->vertex());
    return result;
}

template <typename T>
Core::TVector<Vertex<T>> calcVertices(const Facet<T>& facet)
{
    return calcVertices(calcHalfedges(facet));
}

} // namespace Geometry

#endif // GLFWTESTAPP_FACETUTILS_H
