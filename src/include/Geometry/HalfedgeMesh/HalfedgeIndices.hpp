#ifndef FILAPP_HALFEDGEINDICES_HPP
#define FILAPP_HALFEDGEINDICES_HPP

#include <Core/Types/TVector.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>

namespace Geometry
{

struct SegmentIndices
{
    uint32_t source;
    uint32_t target;

    SegmentIndices(uint32_t source, uint32_t target)
        : source(source), target(target)
    {
    }
};

template <typename T>
Core::TVector<SegmentIndices>
calcMeshSegmentIndices(const HalfedgeMesh<T>& mesh)
{
    Core::TVector<SegmentIndices> result;
    for (const Halfedge<T>& halfedge: mesh.getHalfedges())
        result.push_back(SegmentIndices(halfedge.getVertexIndex(),
                                        halfedge.next()->getVertexIndex()));
    return result;
}

} // namespace Geometry

#endif // FILAPP_HALFEDGEINDICES_HPP
