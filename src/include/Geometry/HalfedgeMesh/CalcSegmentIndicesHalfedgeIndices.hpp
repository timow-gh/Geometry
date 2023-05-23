#ifndef GEOMETRY_CALCSEGMENTINDICESHALFEDGEINDICES_HPP
#define GEOMETRY_CALCSEGMENTINDICESHALFEDGEINDICES_HPP

#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <cstdint>
#include <vector>

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
std::vector<SegmentIndices> calcMeshSegmentIndices(const HalfedgeMesh<TFloat, TIndex>& mesh)
{
  std::vector<SegmentIndices> result;
  for (const Halfedge<TFloat, TIndex>& halfedge: mesh.halfedges)
    result.push_back(SegmentIndices(halfedge.getVertexIndex().getValue(), halfedge.getNext().getVertexIndex().getValue()));
  return result;
}

} // namespace Geometry

#endif // GEOMETRY_CALCSEGMENTINDICESHALFEDGEINDICES_HPP
