#ifndef GEOMETRY_MESH_MESHCOREFINESTATUS_HPP
#define GEOMETRY_MESH_MESHCOREFINESTATUS_HPP

namespace Geometry
{

/** \brief Reason \c corefine could not refine two meshes; Ok on success. */
enum class CorefineStatus
{
  Ok,
  // A vertex position is infinite or NaN.
  NonFiniteCoordinates,
  // A face has zero area, so it has no plane to intersect with.
  DegenerateFace,
  // Predicate results contradict each other, or two distinct intersection points coincide, e.g. two
  // points on one edge that round to the same position. Exact predicates on explicit points rule out
  // the first; the second needs predicates on the intersection points' definitions (see
  // detail::ImplicitPoint).
  DegenerateIntersection
};

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHCOREFINESTATUS_HPP
