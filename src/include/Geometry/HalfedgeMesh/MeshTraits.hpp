#ifndef GEOMETRY_MESHTRAITS_HPP
#define GEOMETRY_MESHTRAITS_HPP

namespace Geometry
{

template <typename TFloat, typename TIndex>
class Vertex;

template <typename TFloat, typename TIndex>
class Halfedge;

template <typename TFloat, typename TIndex>
class Facet;

template <typename TFloat, typename TIndex>
class HalfedgeMesh;

template <typename TFloat, typename TIndex>
struct MeshTraits
{
  using Vertex_t = Vertex<TFloat, TIndex>;
  using Halfedge_t = Halfedge<TFloat, TIndex>;
  using Facet_t = Facet<TFloat, TIndex>;
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRAITS_HPP
