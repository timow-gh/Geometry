#ifndef GEOMETRY_MESHINDEXTRAITS_HPP
#define GEOMETRY_MESHINDEXTRAITS_HPP

#include "Geometry/HalfedgeMesh/MeshIndices.hpp"

namespace Geometry
{

template <typename TIndex>
struct MeshIndexTraits
{
  using VertexIndex_t = VertexIndex<TIndex>;
  using HalfedgeIndex_t = HalfedgeIndex<TIndex>;
  using FacetIndex_t = FacetIndex<TIndex>;
};

} // namespace Geometry

#endif // GEOMETRY_MESHINDEXTRAITS_HPP
