#ifndef ORIGAMI_CALCFACETNORMAL_HPP
#define ORIGAMI_CALCFACETNORMAL_HPP

#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/Halfedge.hpp"
#include <linal/vec.hpp>

namespace Geometry
{

template <typename TFacet>
linal::vec3<typename TFacet::value_type> calc_facet_normal(const TFacet& facet)
{
  using value_type = typename TFacet::value_type;
  using Halfedge_t = typename TFacet::Halfedge_t;

  const Halfedge_t& he = facet.getHalfedge();
  const Halfedge_t& next = he.getNext();

  using vec_t = linal::vec3<value_type>;
  const vec_t v0 = he.getVertex().getVector();
  const vec_t v1 = next.getVertex().getVector();
  const vec_t v2 = next.getNextVertex().getVector();

  linal::vec3<value_type> normal = linal::cross(v1 - v0, v2 - v1);
  return linal::normalize(normal);
}

} // namespace Geometry

#endif // ORIGAMI_CALCFACETNORMAL_HPP
