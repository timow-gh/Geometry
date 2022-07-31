#ifndef GEOMETRY_MESHTRAITS_HPP
#define GEOMETRY_MESHTRAITS_HPP

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
class Vertex;

template <typename TFloatType, typename TIndexType>
class Halfedge;

template <typename TFloatType, typename TIndexType>
class Facet;

template <typename TFloatType, typename TIndexType>
class HalfedgeMesh;

template <typename TFloatType, typename TIndexType>
struct MeshTraits
{
    using Vertex_t = Vertex<TFloatType, TIndexType>;
    using Halfedge_t = Halfedge<TFloatType, TIndexType>;
    using Facet_t = Facet<TFloatType, TIndexType>;
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRAITS_HPP
