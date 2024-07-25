#ifndef ORIGAMI_CALCVERTEXNORMALS_HPP
#define ORIGAMI_CALCVERTEXNORMALS_HPP

#include "Geometry/HalfedgeMesh/CalcFacetNormal.hpp"
#include "Geometry/HalfedgeMesh/CalcVertexFacets.hpp"
#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include <linal/vec.hpp>
#include <vector>

namespace Geometry
{

template <typename TFloat, typename TIndex>
std::vector<linal::vec3<TFloat>> calcVertexNormals(const HalfedgeMesh<TFloat, TIndex>& mesh)
{
  using vec_t = linal::vec3<TFloat>;
  std::vector<vec_t> normals(mesh.getMeshPoints().size(), vec_t{0.0F, 0.0F, 0.0F});

  using vertex_t = Vertex<TFloat, TIndex>;

  for (const vertex_t& vertex: mesh.getVertices())
  {
    const std::vector<Facet<TFloat, TIndex>> facets = calcVertexFacets(vertex);
    for (const Facet<TFloat, TIndex>& facet: facets)
    {
      const vec_t normal = calcFaceNormal(facet);
      normals[vertex.getIndex().get_value()] += normal;
    }
  }

  return normals;
}

} // namespace Geometry

#endif // ORIGAMI_CALCVERTEXNORMALS_HPP
