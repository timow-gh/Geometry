#ifndef FILAPP_HALFEDGEINDICES_HPP
#define FILAPP_HALFEDGEINDICES_HPP

#include <Core/Types/TVector.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <cstdint>

namespace Geometry
{

struct SegmentIndices
{
    std::uint32_t source;
    std::uint32_t target;

    SegmentIndices(std::uint32_t source, std::uint32_t target) : source(source), target(target) {}
};

template <typename TFloatType, typename TIndexType>
Core::TVector<SegmentIndices> calcMeshSegmentIndices(const HalfedgeMesh<TFloatType, TIndexType>& mesh)
{
    Core::TVector<SegmentIndices> result;
    for (const Halfedge<TFloatType, TIndexType>& halfedge: mesh.halfedges)
        result.push_back(SegmentIndices(halfedge.getVertexIndex().getValue(), halfedge.getNext().getVertexIndex().getValue()));
    return result;
}

} // namespace Geometry

#endif // FILAPP_HALFEDGEINDICES_HPP
