#ifndef GEOMETRY_MESH_DETAIL_COREFINE_HPP
#define GEOMETRY_MESH_DETAIL_COREFINE_HPP

#include "Geometry/Mesh/MeshCorefineStatus.hpp"
#include "Geometry/Mesh/MeshFlip.hpp"
#include "Geometry/Mesh/MeshSplit.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/BooleanIntersection.hpp"
#include "Geometry/Predicates.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/detail/ImplicitPoint.hpp"
#include "Geometry/detail/TriangleOrientation.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

namespace Geometry
{
namespace detail
{

/** \internal \brief Entry of \c OperandRefinement::pointOfVertex for a vertex that is no intersection point. */
inline constexpr std::size_t no_intersection_point = std::numeric_limits<std::size_t>::max();

/** \internal \brief One of the two operands of a mesh Boolean. */
enum class Operand : std::uint8_t
{
  A,
  B
};

/** \internal \brief The simplex of \p key on \p operand. O(1). */
template <typename TIndex>
GEO_NODISCARD constexpr MeshSimplex<TIndex> simplex_on(const IntersectionKey<TIndex>& key, const Operand operand) noexcept
{
  return operand == Operand::A ? key.simplexA : key.simplexB;
}

/** \internal \brief The lookups of \p graph for \p operand. O(1). */
template <typename T, typename TIndex>
GEO_NODISCARD const OperandIntersections<TIndex>& intersections_on(const IntersectionGraph<T, TIndex>& graph, const Operand operand) noexcept
{
  return operand == Operand::A ? graph.onMeshA : graph.onMeshB;
}

/**
 * \internal
 * \brief How one operand was refined: the vertex of every intersection point and the edge of every
 * intersection segment.
 *
 * Indices refer to the \c IntersectionGraph the operand was refined with; point i is the point of
 * key i, so \c vertexOfPoint is also the key-to-vertex map.
 */
template <typename T, typename TIndex>
struct OperandRefinement
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;

  std::vector<VertexHandle> vertexOfPoint;
  // By vertex storage index; no_intersection_point for a vertex that is no intersection point. The
  // way back from a vertex to its definition, so later predicates never see a bare rounded position.
  std::vector<std::size_t> pointOfVertex;
  std::vector<EdgeHandle> edgeOfSegment;
  // By edge storage index. Separate from Edge::crease, which marks shading features, not the curve.
  std::vector<bool> isIntersectionEdge;
};

/**
 * \internal
 * \brief The \c ImplicitPoint at \p vertex of a refined operand: the intersection point's definition,
 * or the explicit input vertex. O(1).
 */
template <typename T, typename TIndex>
GEO_NODISCARD ImplicitPoint<T> implicit_point_of(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                                 const OperandRefinement<T, TIndex>& refinement,
                                                 const IntersectionGraph<T, TIndex>& graph,
                                                 const typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle vertex) noexcept
{
  GEO_ASSERT(vertex.get_value() < refinement.pointOfVertex.size());
  const std::size_t point = refinement.pointOfVertex[vertex.get_value()];
  if (point == no_intersection_point)
  {
    return ImplicitPoint<T>::create_explicit(mesh.get_position(vertex));
  }
  return graph.points[point];
}

/**
 * \internal
 * \brief Refines one operand along an \c IntersectionGraph: inserts every intersection point as a
 * vertex, then makes every intersection segment an edge.
 *
 * Points on original edges are inserted first, in order along each edge, then points inside original
 * faces. Segments that are no edge after the insertions are recovered by flipping the edges they cross
 * inside their original face (Sloan's constrained-edge recovery), never an edge along an original edge
 * or an already recovered segment, so the refinement stays inside each original face and the
 * intersection curve, once an edge, stays one.
 *
 * Splits only append elements and flips keep handles, so every original handle stays valid and side
 * arrays indexed by storage just grow; the original face of every face is tracked that way.
 */
template <typename T, typename TIndex>
class OperandRefiner {
public:
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using Graph = IntersectionGraph<T, TIndex>;
  using Point = ImplicitPoint<T>;
  using Refinement = OperandRefinement<T, TIndex>;

  /** \pre \p graph was computed with \p mesh as \p operand, and \p mesh has not changed since. */
  OperandRefiner(Mesh& mesh, const Graph& graph, const Operand operand) noexcept
      : m_mesh(&mesh)
      , m_graph(&graph)
      , m_operand(operand)
  {
  }

  /**
   * \brief Runs the refinement once.
   *
   * On failure the mesh keeps a valid connectivity but is partly refined. O(k log k + sum over faces
   * of (points in the face)^2 + flips) for k intersection points; the quadratic term is the point
   * location, which scans the parts of the original face.
   *
   * \return \c Ok, or \c DegenerateIntersection when predicate results contradict each other.
   */
  GEO_NODISCARD CorefineStatus refine()
  {
    initialize();
    if (!register_vertex_points() || !order_edge_points())
    {
      return CorefineStatus::DegenerateIntersection;
    }
    insert_edge_points();
    if (!insert_face_points() || !recover_segments())
    {
      return CorefineStatus::DegenerateIntersection;
    }
    GEO_ASSERT(std::ranges::all_of(m_refinement.vertexOfPoint, [](const VertexHandle vertex) { return vertex.is_valid(); }));
    GEO_ASSERT(std::ranges::all_of(m_refinement.edgeOfSegment, [](const EdgeHandle edge) { return edge.is_valid(); }));
    return CorefineStatus::Ok;
  }

  /** \brief Hands over the refinement. \pre \c refine returned \c Ok. */
  GEO_NODISCARD Refinement release() && noexcept { return std::move(m_refinement); }

private:
  // Coplanar tests in an original face run in the projection that drops this axis; orientation is
  // the sign of the projected face, so side tests can be normalized to "Positive = left".
  struct FaceFrame
  {
    std::uint8_t axis{0};
    Orientation orientation{Orientation::Zero};
  };

  // The points of one original edge, in order from its stored halfedge's source to end.
  struct EdgeRun
  {
    EdgeHandle edge;
    VertexHandle end;
    std::size_t begin{0};
    std::size_t count{0};
  };

  void initialize()
  {
    Mesh& mesh = *m_mesh;
    const OperandIntersections<TIndex>& intersections = detail::intersections_on(*m_graph, m_operand);
    const std::size_t insertedCount = intersections.pointsByEdge.size() + intersections.pointsByFace.size();
    // Each insertion adds at most 1 vertex, 3 edges and 2 faces.
    mesh.reserve({.vertices = mesh.vertex_storage_size() + insertedCount,
                  .edges = mesh.edge_storage_size() + 3 * insertedCount,
                  .faces = mesh.face_storage_size() + 2 * insertedCount});

    m_refinement.vertexOfPoint.assign(m_graph->points.size(), VertexHandle{});
    m_refinement.pointOfVertex.assign(mesh.vertex_storage_size(), no_intersection_point);
    m_refinement.edgeOfSegment.assign(m_graph->segments.size(), EdgeHandle{});
    m_refinement.isIntersectionEdge.assign(mesh.edge_storage_size(), false);
    m_followsOriginalEdge.assign(mesh.edge_storage_size(), true);
    m_originalFace.resize(mesh.face_storage_size());
    std::iota(m_originalFace.begin(), m_originalFace.end(), TIndex{0});
    m_inSubFaces.assign(mesh.face_storage_size(), false);

    m_frames.assign(mesh.face_storage_size(), FaceFrame{});
    for (const FaceHandle face : mesh.faces())
    {
      const std::array<VertexHandle, 3> corners = mesh.vertices_around_face(face);
      const linal::vec3<T>& first = mesh.get_position(corners[0]);
      const linal::vec3<T>& second = mesh.get_position(corners[1]);
      const linal::vec3<T>& third = mesh.get_position(corners[2]);
      const std::uint8_t axis = detail::dominant_axis(detail::triangle_orientation(first, second, third));
      const Orientation orientation = orient2d(detail::project_dropping_axis(first, axis),
                                               detail::project_dropping_axis(second, axis),
                                               detail::project_dropping_axis(third, axis));
      m_frames[face.get_value()] = FaceFrame{axis, orientation};
    }
  }

  GEO_NODISCARD Point implicit_point(const VertexHandle vertex) const noexcept
  {
    return detail::implicit_point_of(*m_mesh, m_refinement, *m_graph, vertex);
  }

  void register_point(const std::size_t point, const VertexHandle vertex) noexcept
  {
    GEO_ASSERT(!m_refinement.vertexOfPoint[point].is_valid());
    m_refinement.vertexOfPoint[point] = vertex;
    m_refinement.pointOfVertex[vertex.get_value()] = point;
  }

  /**
   * \brief Maps the points that are input vertices of this operand to those vertices.
   * \return False if two points name the same vertex, which only contradicting predicates produce.
   */
  GEO_NODISCARD bool register_vertex_points() noexcept
  {
    for (std::size_t i = 0; i < m_graph->keys.size(); ++i)
    {
      const MeshSimplex<TIndex> simplex = detail::simplex_on(m_graph->keys[i], m_operand);
      if (simplex.kind != SimplexKind::Vertex)
      {
        continue;
      }
      const VertexHandle vertex{simplex.index};
      if (m_refinement.pointOfVertex[vertex.get_value()] != no_intersection_point)
      {
        return false;
      }
      register_point(i, vertex);
    }
    return true;
  }

  /**
   * \brief Sorts the points of every original edge along it, before any split changes the faces
   * around the edges.
   * \return False if two points on an edge, or a point and an endpoint, cannot be told apart.
   */
  GEO_NODISCARD bool order_edge_points()
  {
    const Mesh& mesh = *m_mesh;
    bool ordered = true;
    detail::intersections_on(*m_graph, m_operand).pointsByEdge.for_each_group([&](const TIndex edgeValue, const std::span<const std::size_t> points) {
      if (!ordered)
      {
        return;
      }
      const EdgeHandle edge{edgeValue};
      const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
      // Any face of the edge supplies a reference point off the edge's line: its apex.
      const HalfedgeHandle faceSide = mesh.is_boundary(forward) ? mesh.get_halfedge(forward).twin : forward;
      const VertexHandle apex = mesh.target_vertex(mesh.get_halfedge(faceSide).next);
      const std::uint8_t axis = m_frames[mesh.get_halfedge(faceSide).face.get_value()].axis;

      const std::size_t begin = m_orderedEdgePoints.size();
      m_orderedEdgePoints.insert(m_orderedEdgePoints.end(), points.begin(), points.end());
      const std::span<std::size_t> run = std::span<std::size_t>{m_orderedEdgePoints}.subspan(begin, points.size());
      ordered = sort_along_edge(run, mesh.source_vertex(forward), mesh.target_vertex(forward), apex, axis);
      m_edgeRuns.push_back(EdgeRun{edge, mesh.target_vertex(forward), begin, points.size()});
    });
    return ordered;
  }

  /**
   * \brief Sorts \p points, which lie on the edge (\p start, \p end), from \p start to \p end.
   *
   * Seen from \p apex, a point off the edge's line, the points sweep in one rotational direction, so
   * "first precedes second" is \c orient2d(apex, first, second) agreeing with
   * \c orient2d(apex, start, end). A presort by the rounded parameter only makes the insertion sort
   * that follows linear in the usual case; the predicate decides.
   *
   * \return False if two consecutive points, or a point and an endpoint, are not strictly ordered.
   */
  GEO_NODISCARD bool sort_along_edge(const std::span<std::size_t> points,
                                     const VertexHandle start,
                                     const VertexHandle end,
                                     const VertexHandle apex,
                                     const std::uint8_t axis) const
  {
    using Vec3 = linal::vec3<T>;

    const Point startPoint = implicit_point(start);
    const Point endPoint = implicit_point(end);
    const Point apexPoint = implicit_point(apex);
    const Orientation forward = orient2d(apexPoint, startPoint, endPoint, axis);
    if (forward == Orientation::Zero)
    {
      return false;
    }
    // Edge ordering runs on rounded intersection points, so it is only as exact as the ImplicitPoint
    // predicates (see detail::ImplicitPoint).
    const auto precedes = [&](const Point& first, const Point& second) {
      return orient2d(apexPoint, first, second, axis) == forward;
    };

    const Vec3 direction{endPoint.position() - startPoint.position()};
    std::ranges::sort(points, {}, [&](const std::size_t point) {
      return linal::dot(direction, Vec3{m_graph->points[point].position() - startPoint.position()});
    });
    for (std::size_t i = 1; i < points.size(); ++i)
    {
      for (std::size_t j = i; j > 0 && precedes(m_graph->points[points[j]], m_graph->points[points[j - 1]]); --j)
      {
        std::swap(points[j], points[j - 1]);
      }
    }

    const Point* previous = &startPoint;
    for (const std::size_t point : points)
    {
      if (!precedes(*previous, m_graph->points[point]))
      {
        return false;
      }
      previous = &m_graph->points[point];
    }
    return precedes(*previous, endPoint);
  }

  /** \brief Splits every original edge at its points, in order. */
  void insert_edge_points()
  {
    const Mesh& mesh = *m_mesh;
    for (const EdgeRun& run : m_edgeRuns)
    {
      // Each split keeps the edge's handle for the half at its stored source, so the rest of the
      // original edge is always the piece reaching run.end.
      EdgeHandle remaining = run.edge;
      for (std::size_t i = run.begin; i < run.begin + run.count; ++i)
      {
        const std::size_t point = m_orderedEdgePoints[i];
        const VertexHandle vertex = split_edge_tracked(remaining, m_graph->points[point].position());
        register_point(point, vertex);
        remaining = mesh.get_halfedge(mesh.find_halfedge(vertex, run.end)).edge;
      }
    }
  }

  /**
   * \brief Inserts the points strictly inside each original face into the part of the face that
   * contains them.
   * \return False if a point lies outside every part, on the face's boundary, or on a vertex.
   */
  GEO_NODISCARD bool insert_face_points()
  {
    bool inserted = true;
    detail::intersections_on(*m_graph, m_operand).pointsByFace.for_each_group([&](const TIndex faceValue, const std::span<const std::size_t> points) {
      if (!inserted)
      {
        return;
      }
      const FaceFrame frame = m_frames[faceValue];
      if (frame.orientation == Orientation::Zero)
      {
        inserted = false;
        return;
      }
      collect_sub_faces(FaceHandle{faceValue});
      inserted = std::ranges::all_of(points, [&](const std::size_t point) { return insert_face_point(point, frame); });
      release_sub_faces();
    });
    return inserted;
  }

  /**
   * \brief Locates \p point among the collected parts of its original face and splits the part, or
   * the inner edge it lies on.
   *
   * O(parts of the face).
   */
  GEO_NODISCARD bool insert_face_point(const std::size_t point, const FaceFrame frame)
  {
    const Mesh& mesh = *m_mesh;
    const Point& intersection = m_graph->points[point];
    for (std::size_t i = 0; i < m_subFaces.size(); ++i)
    {
      const FaceHandle subFace = m_subFaces[i];
      const std::array<HalfedgeHandle, 3> sides = mesh.halfedges_around_face(subFace);
      std::array<Orientation, 3> sideOfPoint{};
      for (std::size_t j = 0; j < 3; ++j)
      {
        // Point location runs on rounded intersection points, so it is only as exact as the
        // ImplicitPoint predicates (see detail::ImplicitPoint).
        const Orientation side = orient2d(implicit_point(mesh.source_vertex(sides[j])), implicit_point(mesh.target_vertex(sides[j])), intersection, frame.axis);
        sideOfPoint[j] = detail::multiply_signs(side, frame.orientation);
      }
      if (std::ranges::find(sideOfPoint, Orientation::Negative) != sideOfPoint.end())
      {
        continue;
      }
      const auto onSide = std::ranges::find(sideOfPoint, Orientation::Zero);
      if (std::ranges::count(sideOfPoint, Orientation::Zero) > 1)
      {
        // On a vertex: two distinct points at one position.
        return false;
      }

      const std::size_t oldFaceStorage = mesh.face_storage_size();
      VertexHandle vertex;
      if (onSide == sideOfPoint.end())
      {
        vertex = split_face_tracked(subFace, intersection.position());
      }
      else
      {
        const auto sideIndex = static_cast<std::size_t>(std::distance(sideOfPoint.begin(), onSide));
        const EdgeHandle edge = mesh.get_halfedge(sides[sideIndex]).edge;
        // Strictly inside the original face, so never on its boundary.
        if (m_followsOriginalEdge[edge.get_value()])
        {
          return false;
        }
        vertex = split_edge_tracked(edge, intersection.position());
      }
      for (std::size_t face = oldFaceStorage; face < mesh.face_storage_size(); ++face)
      {
        m_subFaces.push_back(FaceHandle{static_cast<TIndex>(face)});
      }
      register_point(point, vertex);
      return true;
    }
    return false;
  }

  /** \brief Collects the faces that refine \p original into \c m_subFaces. O(parts of the face). */
  void collect_sub_faces(const FaceHandle original)
  {
    const Mesh& mesh = *m_mesh;
    const TIndex originalValue = original.get_value();
    GEO_ASSERT(m_originalFace[originalValue] == originalValue);
    m_subFaces.clear();
    m_subFaces.push_back(original);
    m_inSubFaces[originalValue] = true;
    for (std::size_t i = 0; i < m_subFaces.size(); ++i)
    {
      for (const HalfedgeHandle side : mesh.halfedges_around_face(m_subFaces[i]))
      {
        const FaceHandle neighbor = mesh.get_halfedge(mesh.get_halfedge(side).twin).face;
        if (!neighbor.is_valid() || m_originalFace[neighbor.get_value()] != originalValue || m_inSubFaces[neighbor.get_value()])
        {
          continue;
        }
        m_inSubFaces[neighbor.get_value()] = true;
        m_subFaces.push_back(neighbor);
      }
    }
  }

  void release_sub_faces() noexcept
  {
    for (const FaceHandle face : m_subFaces)
    {
      m_inSubFaces[face.get_value()] = false;
    }
    m_subFaces.clear();
  }

  /**
   * \brief Makes every segment an edge, recovering the missing ones inside their original face.
   * \return False if a segment cannot be recovered: it would cross a fixed edge or pass through a vertex.
   */
  GEO_NODISCARD bool recover_segments()
  {
    bool recovered = true;
    detail::intersections_on(*m_graph, m_operand).segmentsByFace.for_each_group([&](const TIndex faceValue, const std::span<const std::size_t> segments) {
      if (recovered)
      {
        recovered = std::ranges::all_of(segments, [&](const std::size_t segment) { return recover_segment(faceValue, segment); });
      }
    });
    return recovered;
  }

  /** \brief Makes \p segment an edge inside original face \p faceValue and marks it. */
  GEO_NODISCARD bool recover_segment(const TIndex faceValue, const std::size_t segment)
  {
    const Mesh& mesh = *m_mesh;
    // A segment along an original edge is listed under both of its faces.
    if (m_refinement.edgeOfSegment[segment].is_valid())
    {
      return true;
    }
    const VertexHandle first = m_refinement.vertexOfPoint[m_graph->segments[segment][0]];
    const VertexHandle second = m_refinement.vertexOfPoint[m_graph->segments[segment][1]];
    HalfedgeHandle halfedge = mesh.find_halfedge(first, second);
    if (!halfedge.is_valid())
    {
      const FaceFrame frame = m_frames[faceValue];
      if (frame.orientation == Orientation::Zero || !flip_until_edge(faceValue, first, second, frame))
      {
        return false;
      }
      halfedge = mesh.find_halfedge(first, second);
      if (!halfedge.is_valid())
      {
        return false;
      }
    }
    const EdgeHandle edge = mesh.get_halfedge(halfedge).edge;
    // Distinct segments join distinct vertex pairs, since no two points share a vertex.
    GEO_ASSERT(!m_refinement.isIntersectionEdge[edge.get_value()]);
    m_refinement.edgeOfSegment[segment] = edge;
    m_refinement.isIntersectionEdge[edge.get_value()] = true;
    return true;
  }

  /**
   * \brief Side of the directed line through \p first and \p second for \p vertex, normalized so that
   * \c Positive is left in the original face.
   */
  GEO_NODISCARD Orientation side_of_segment(const Point& first, const Point& second, const VertexHandle vertex, const FaceFrame frame) const noexcept
  {
    return detail::multiply_signs(orient2d(first, second, implicit_point(vertex), frame.axis), frame.orientation);
  }

  /**
   * \brief Collects, in order, the edges that the segment from \p first to \p second crosses inside
   * original face \p faceValue.
   *
   * Walks from \p first: finds the part of the face around it that the segment leaves through, then
   * crosses one edge at a time until reaching \p second.
   *
   * \return False if the segment would cross an edge that must stay (along an original edge, or a
   * recovered segment), leave the face, or pass through a vertex.
   */
  GEO_NODISCARD bool collect_crossing_edges(const TIndex faceValue, const VertexHandle first, const VertexHandle second, const FaceFrame frame)
  {
    const Mesh& mesh = *m_mesh;
    const Point firstPoint = implicit_point(first);
    const Point secondPoint = implicit_point(second);
    m_crossingEdges.clear();

    // The part (first, right, left) the segment leaves through has right strictly right of the segment
    // and left strictly left; its halfedge right -> left is the first crossing.
    HalfedgeHandle crossing{};
    const HalfedgeHandle startOutgoing = mesh.get_vertex(first).halfedge;
    HalfedgeHandle outgoing = startOutgoing;
    do
    {
      if (!mesh.is_boundary(outgoing) && m_originalFace[mesh.get_halfedge(outgoing).face.get_value()] == faceValue)
      {
        const HalfedgeHandle opposite = mesh.get_halfedge(outgoing).next;
        if (side_of_segment(firstPoint, secondPoint, mesh.target_vertex(outgoing), frame) == Orientation::Negative
            && side_of_segment(firstPoint, secondPoint, mesh.target_vertex(opposite), frame) == Orientation::Positive)
        {
          crossing = opposite;
          break;
        }
      }
      outgoing = mesh.next_in_outgoing_fan(outgoing);
    } while (outgoing != startOutgoing);
    if (!crossing.is_valid())
    {
      return false;
    }

    // Each step enters a new part, so more crossings than parts means the predicates contradict.
    const std::size_t limit = mesh.face_storage_size();
    while (m_crossingEdges.size() < limit)
    {
      const EdgeHandle edge = mesh.get_halfedge(crossing).edge;
      if (m_followsOriginalEdge[edge.get_value()] || m_refinement.isIntersectionEdge[edge.get_value()])
      {
        return false;
      }
      m_crossingEdges.push_back(edge);
      const HalfedgeHandle entered = mesh.get_halfedge(crossing).twin;
      if (mesh.is_boundary(entered) || m_originalFace[mesh.get_halfedge(entered).face.get_value()] != faceValue)
      {
        return false;
      }
      const VertexHandle apex = mesh.target_vertex(mesh.get_halfedge(entered).next);
      if (apex == second)
      {
        return true;
      }
      // The entered part is (left, right, apex): leave through apex -> left if apex is right of the
      // segment, through right -> apex otherwise.
      const Orientation apexSide = side_of_segment(firstPoint, secondPoint, apex, frame);
      if (apexSide == Orientation::Zero)
      {
        return false;
      }
      crossing = apexSide == Orientation::Negative ? mesh.get_halfedge(entered).prev : mesh.get_halfedge(entered).next;
    }
    return false;
  }

  /**
   * \brief Whether the two faces of \p edge form a strictly convex quad in \p frame's projection, so
   * that flipping the edge keeps both faces' orientation.
   */
  GEO_NODISCARD bool is_strictly_convex_quad(const EdgeHandle edge, const FaceFrame frame) const noexcept
  {
    const Mesh& mesh = *m_mesh;
    const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
    const HalfedgeHandle backward = mesh.get_halfedge(forward).twin;
    const Point start = implicit_point(mesh.source_vertex(forward));
    const Point end = implicit_point(mesh.target_vertex(forward));
    const Point left = implicit_point(mesh.target_vertex(mesh.get_halfedge(forward).next));
    const Point right = implicit_point(mesh.target_vertex(mesh.get_halfedge(backward).next));
    // Flip convexity runs on rounded intersection points, so it is only as exact as the ImplicitPoint
    // predicates (see detail::ImplicitPoint).
    return detail::multiply_signs(orient2d(start, end, left, frame.axis), orient2d(start, end, right, frame.axis)) == Orientation::Negative
           && detail::multiply_signs(orient2d(left, right, start, frame.axis), orient2d(left, right, end, frame.axis)) == Orientation::Negative;
  }

  /**
   * \brief Flips the edges crossing the segment from \p first to \p second until it is an edge.
   *
   * Sloan's recovery: flip any crossing edge whose quad is strictly convex, keep the new diagonal if it
   * still crosses, and retry the non-convex ones later. With exact predicates every pass flips at least
   * one edge and the loop ends; with contradicting ones a pass without a flip, or a flip budget of
   * n^2 + n for n crossings, ends it with a failure instead of looping. O(n^2) flips.
   */
  GEO_NODISCARD bool flip_until_edge(const TIndex faceValue, const VertexHandle first, const VertexHandle second, const FaceFrame frame)
  {
    Mesh& mesh = *m_mesh;
    if (!collect_crossing_edges(faceValue, first, second, frame))
    {
      return false;
    }
    const Point firstPoint = implicit_point(first);
    const Point secondPoint = implicit_point(second);
    const std::size_t crossingCount = m_crossingEdges.size();
    std::size_t flipBudget = crossingCount * crossingCount + crossingCount;

    while (!m_crossingEdges.empty())
    {
      bool flipped = false;
      std::size_t kept = 0;
      for (std::size_t i = 0; i < m_crossingEdges.size(); ++i)
      {
        const EdgeHandle edge = m_crossingEdges[i];
        if (flipBudget == 0 || !is_strictly_convex_quad(edge, frame) || is_flip_ok(mesh, edge) != FlipStatus::Ok)
        {
          m_crossingEdges[kept++] = edge;
          continue;
        }
        detail::flip_edge_unchecked(mesh, edge);
        --flipBudget;
        flipped = true;
        const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
        const VertexHandle start = mesh.source_vertex(forward);
        const VertexHandle end = mesh.target_vertex(forward);
        const bool touchesSegment = start == first || start == second || end == first || end == second;
        if (!touchesSegment
            && detail::multiply_signs(side_of_segment(firstPoint, secondPoint, start, frame), side_of_segment(firstPoint, secondPoint, end, frame))
                   == Orientation::Negative)
        {
          m_crossingEdges[kept++] = edge;
        }
      }
      m_crossingEdges.resize(kept);
      if (!flipped)
      {
        return false;
      }
    }
    return true;
  }

  /** \brief Grows the side arrays to the mesh's storage; new entries are filled in by the caller. */
  void grow_side_arrays()
  {
    const Mesh& mesh = *m_mesh;
    m_refinement.pointOfVertex.resize(mesh.vertex_storage_size(), no_intersection_point);
    m_refinement.isIntersectionEdge.resize(mesh.edge_storage_size(), false);
    m_followsOriginalEdge.resize(mesh.edge_storage_size(), false);
    m_originalFace.resize(mesh.face_storage_size(), std::numeric_limits<TIndex>::max());
    m_inSubFaces.resize(mesh.face_storage_size(), false);
  }

  /**
   * \brief \c split_edge that keeps the side arrays current: the new half of the edge follows an
   * original edge iff the edge did, and each new face belongs to the original face it was split from.
   */
  GEO_NODISCARD VertexHandle split_edge_tracked(const EdgeHandle edge, const linal::vec3<T>& position)
  {
    Mesh& mesh = *m_mesh;
    const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
    const FaceHandle leftFace = mesh.get_halfedge(forward).face;
    const FaceHandle rightFace = mesh.get_halfedge(mesh.get_halfedge(forward).twin).face;
    const VertexHandle end = mesh.target_vertex(forward);

    const VertexHandle middle = Geometry::split_edge(mesh, edge, position);
    GEO_ASSERT(middle.is_valid());
    grow_side_arrays();
    // The new half runs from middle to end; the new face on each side holds it.
    const HalfedgeHandle middleToEnd = mesh.find_halfedge(middle, end);
    m_followsOriginalEdge[mesh.get_halfedge(middleToEnd).edge.get_value()] = m_followsOriginalEdge[edge.get_value()];
    if (leftFace.is_valid())
    {
      m_originalFace[mesh.get_halfedge(middleToEnd).face.get_value()] = m_originalFace[leftFace.get_value()];
    }
    if (rightFace.is_valid())
    {
      m_originalFace[mesh.get_halfedge(mesh.get_halfedge(middleToEnd).twin).face.get_value()] = m_originalFace[rightFace.get_value()];
    }
    return middle;
  }

  /** \brief \c split_face that keeps the side arrays current: the new faces belong to \p face's original face. */
  GEO_NODISCARD VertexHandle split_face_tracked(const FaceHandle face, const linal::vec3<T>& position)
  {
    Mesh& mesh = *m_mesh;
    const std::size_t oldFaceStorage = mesh.face_storage_size();
    const VertexHandle center = Geometry::split_face(mesh, face, position);
    GEO_ASSERT(center.is_valid());
    grow_side_arrays();
    for (std::size_t newFace = oldFaceStorage; newFace < mesh.face_storage_size(); ++newFace)
    {
      m_originalFace[newFace] = m_originalFace[face.get_value()];
    }
    return center;
  }

  Mesh* m_mesh;
  const Graph* m_graph;
  Operand m_operand;
  Refinement m_refinement;

  // By original face storage index.
  std::vector<FaceFrame> m_frames;
  // By face storage index: the original face each face lies in.
  std::vector<TIndex> m_originalFace;
  // By edge storage index: whether the edge is (a piece of) an original edge. Those edges are never
  // flipped, and a point strictly inside a face never lands on one.
  std::vector<bool> m_followsOriginalEdge;
  std::vector<EdgeRun> m_edgeRuns;
  std::vector<std::size_t> m_orderedEdgePoints;

  // Buffers reused per original face and per segment.
  std::vector<FaceHandle> m_subFaces;
  std::vector<bool> m_inSubFaces;
  std::vector<EdgeHandle> m_crossingEdges;
};

/** \internal \brief Both operands refined along their shared intersection graph. */
template <typename T, typename TIndex>
struct Corefinement
{
  IntersectionGraph<T, TIndex> graph;
  OperandRefinement<T, TIndex> onMeshA;
  OperandRefinement<T, TIndex> onMeshB;
};

/**
 * \internal
 * \brief Corefinement built by \c corefine_in_place, or the reason it failed.
 *
 * A reported failure always carries an empty corefinement.
 */
template <typename T, typename TIndex>
struct CorefinementResult
{
  Corefinement<T, TIndex> corefinement;
  CorefineStatus error = CorefineStatus::Ok;

  GEO_NODISCARD bool has_value() const noexcept { return error == CorefineStatus::Ok; }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

/**
 * \internal
 * \brief Whether a live face has a zero area vector, which leaves it without a plane and without a
 * projection axis. O(F).
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool has_degenerate_face(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  return std::ranges::any_of(mesh.faces(), [&mesh](const FaceHandle face) {
    const std::array<VertexHandle, 3> corners = mesh.vertices_around_face(face);
    const linal::vec3<T> normal =
        detail::triangle_orientation(mesh.get_position(corners[0]), mesh.get_position(corners[1]), mesh.get_position(corners[2]));
    // Exact comparison: the precondition is on this very vector (see dominant_axis), not on a tolerance.
    return normal[0] == T{0} && normal[1] == T{0} && normal[2] == T{0};
  });
}

/**
 * \internal
 * \brief Whether two distinct points of \p graph have the same rounded position.
 *
 * Both would become distinct vertices at one position, joined by zero-length edges or folded faces.
 * O(k log k).
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool has_coincident_points(const IntersectionGraph<T, TIndex>& graph)
{
  using Vec3 = linal::vec3<T>;

  // Exact, lexicographic: linal's comparisons have a tolerance.
  const auto coordinates = [&graph](const std::size_t point) {
    const Vec3& position = graph.points[point].position();
    return std::array<T, 3>{position[0], position[1], position[2]};
  };
  std::vector<std::size_t> order(graph.points.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::ranges::sort(order, {}, coordinates);
  return std::ranges::adjacent_find(order, {}, coordinates) != order.end();
}

/**
 * \internal
 * \brief Refines \p meshA and \p meshB in place so that their intersection curve consists of edges
 * of both, and returns the shared intersection graph with each operand's refinement.
 *
 * Every intersection point becomes one vertex in each mesh at the same rounded position, so the
 * curves of both meshes match edge for edge. Inputs need not be closed. On failure the meshes keep a
 * valid connectivity but may be partly refined; \c corefine offers the strong guarantee on top.
 * Allocation failures propagate as exceptions. O(F log F + k log k + location + flips), see
 * \c OperandRefiner::refine.
 *
 * \pre \p meshA and \p meshB are distinct objects.
 */
template <typename T, typename TIndex>
GEO_NODISCARD CorefinementResult<T, TIndex> corefine_in_place(TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                              TriangleHalfedgeMesh<T, 3, TIndex>& meshB)
{
  using Result = CorefinementResult<T, TIndex>;
  using Refiner = OperandRefiner<T, TIndex>;

  GEO_ASSERT(&meshA != &meshB);
  if (!detail::has_finite_positions(meshA) || !detail::has_finite_positions(meshB))
  {
    return Result{{}, CorefineStatus::NonFiniteCoordinates};
  }
  if (detail::has_degenerate_face(meshA) || detail::has_degenerate_face(meshB))
  {
    return Result{{}, CorefineStatus::DegenerateFace};
  }
  IntersectionGraphResult<T, TIndex> graphResult = detail::compute_intersection_graph(meshA, meshB);
  if (!graphResult.has_value() || detail::has_coincident_points(graphResult.graph))
  {
    // Finite positions were checked above, so only contradicting predicates remain.
    GEO_ASSERT(graphResult.error != IntersectionGraphStatus::NonFiniteCoordinates);
    return Result{{}, CorefineStatus::DegenerateIntersection};
  }

  Refiner refinerA(meshA, graphResult.graph, Operand::A);
  Refiner refinerB(meshB, graphResult.graph, Operand::B);
  if (refinerA.refine() != CorefineStatus::Ok || refinerB.refine() != CorefineStatus::Ok)
  {
    return Result{{}, CorefineStatus::DegenerateIntersection};
  }
  OperandRefinement<T, TIndex> refinementA = std::move(refinerA).release();
  OperandRefinement<T, TIndex> refinementB = std::move(refinerB).release();
  return Result{Corefinement<T, TIndex>{std::move(graphResult.graph), std::move(refinementA), std::move(refinementB)}, CorefineStatus::Ok};
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_COREFINE_HPP
