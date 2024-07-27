#ifndef ORIGAMI_CALCFACETNORMAL_HPP
#define ORIGAMI_CALCFACETNORMAL_HPP

#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/Halfedge.hpp"
#include <linal/vec.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
linal::vec3<TFloat> calc_facet_normal(const Facet<TFloat, TIndex>& facet)
{
  using halfedge_t = Halfedge<TFloat, TIndex>;

  const halfedge_t& he = facet.getHalfedge();
  const halfedge_t& next = he.getNext();

  using vec_t = linal::vec3<TFloat>;
  const vec_t v0 = he.getVertex().getVector();
  const vec_t v1 = next.getVertex().getVector();
  const vec_t v2 = next.getNextVertex().getVector();

  linal::vec3<TFloat> normal = linal::cross(v1 - v0, v2 - v1);
  return linal::normalize(normal);
}

} // namespace Geometry

#endif // ORIGAMI_CALCFACETNORMAL_HPP
