#ifndef GEOMETRY_MESHTRAITS_HPP
#define GEOMETRY_MESHTRAITS_HPP

#include "Geometry/HalfedgeMesh/MeshIndices.hpp"

namespace Geometry
{

template <typename TMeshTraits>
class Vertex;

template <typename TMeshTraits>
class Halfedge;

template <typename TMeshTraits>
class Facet;

template <typename TMeshTraits>
class HalfedgeMesh;

template <typename TMeshTraits>
class MeshPoints;

template <typename TFloat, typename TIndex>
struct MeshTraits
{
  using value_type = TFloat;
  using index_type = TIndex;
  using size_type = TIndex;

  using VertexIndex_t = VertexIndex<TIndex>;
  using HalfedgeIndex_t = HalfedgeIndex<TIndex>;
  using FacetIndex_t = FacetIndex<TIndex>;

  using Vertex_t = Vertex<MeshTraits>;
  using Halfedge_t = Halfedge<MeshTraits>;
  using Facet_t = Facet<MeshTraits>;

  using HalfedgeMesh_t = HalfedgeMesh<MeshTraits>;

  using MeshPoints_t = MeshPoints<MeshTraits>;
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRAITS_HPP
