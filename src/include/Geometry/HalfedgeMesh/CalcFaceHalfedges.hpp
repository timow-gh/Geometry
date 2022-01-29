#ifndef GLFWTESTAPP_FACETUTILS_H
#define GLFWTESTAPP_FACETUTILS_H

#include <Core/Types/TVector.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>

namespace Geometry
{

template <typename T>
CORE_CONSTEXPR Core::TVector<const Halfedge<T>*> calcHalfedges(const Facet<T>& facet)
{
    Core::TVector<const Halfedge<T>*> result;

    const Halfedge<T>* halfedge = &facet.getHalfedge();
    result.push_back(halfedge);
    halfedge = &halfedge->getNext();
    result.push_back(halfedge);

    while (facet.getHalfedge() != halfedge->getNext())
    {
        halfedge = &halfedge->getNext();
        result.push_back(halfedge);
    }
    return result;
}

} // namespace Geometry

#endif // GLFWTESTAPP_FACETUTILS_H
