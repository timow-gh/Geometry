#ifndef FILAPP_HALFEDGEINDICES_HPP
#define FILAPP_HALFEDGEINDICES_HPP

#include <Core/Types/TVector.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <cstdint>

namespace Geometry
{

struct SegmentIndices
{
  std::uint32_t source;
  std::uint32_t target;

  SegmentIndices(std::uint32_t source, std::uint32_t target)
      : source(source)
      , target(target)
  {
  }
};

template <typename TFloat, typename TIndex>
Core::TVector<SegmentIndices> calcMeshSegmentIndices(const HalfedgeMesh<TFloat, TIndex>& mesh)
{
  Core::TVector<SegmentIndices> result;
  for (const Halfedge<TFloat, TIndex>& halfedge: mesh.halfedges)
    result.push_back(SegmentIndices(halfedge.getVertexIndex().getValue(), halfedge.getNext().getVertexIndex().getValue()));
  return result;
}

} // namespace Geometry

#endif // FILAPP_HALFEDGEINDICES_HPP
