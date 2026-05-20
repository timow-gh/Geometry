#ifndef GEOMETRY_CALCSEGMENTINDICESHALFEDGEINDICES_HPP
#define GEOMETRY_CALCSEGMENTINDICESHALFEDGEINDICES_HPP

#include "Geometry/HalfedgeMesh/Halfedge.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"
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

template <typename TMeshTraits>
std::vector<SegmentIndices> calc_mesh_segment_indices(const HalfedgeMesh<TMeshTraits>& mesh)
{
  std::vector<SegmentIndices> result;
  for (const Halfedge_t& halfedge: mesh.halfedges)
    result.push_back(SegmentIndices(halfedge.get_vertex_index().get_value(), halfedge.get_next().get_vertex_index().get_value()));
  return result;
}

} // namespace Geometry

#endif // GEOMETRY_CALCSEGMENTINDICESHALFEDGEINDICES_HPP
