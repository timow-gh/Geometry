#ifndef GEOMETRY_MESHINDEXTRAITS_HPP
#define GEOMETRY_MESHINDEXTRAITS_HPP

#include <Geometry/HalfedgeMesh/HalfedgeMeshIndices.hpp>

namespace Geometry
{

template <typename TIndexType>
struct MeshIndexTraits
{
  using VertexIndex_t = VertexIndex<TIndexType>;
  using HalfedgeIndex_t = HalfedgeIndex<TIndexType>;
  using FacetIndex_t = FacetIndex<TIndexType>;
};

} // namespace Geometry

#endif // GEOMETRY_MESHINDEXTRAITS_HPP
