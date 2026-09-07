#ifndef GEOMETRY_MESH_ADDTRIANGLE_HPP
#define GEOMETRY_MESH_ADDTRIANGLE_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstdint>

namespace Geometry
{

namespace detail
{

// Precondition check for add_triangle. Uses only the read-only (const) connectivity view.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool can_add_triangle(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                    const std::array<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle, 3>& vertices)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;

  auto const connectivity = mesh.connectivity();
  using DirectedEdgeKey = typename decltype(connectivity)::DirectedEdgeKey;

  if (!mesh.contains(vertices[0]) || !mesh.contains(vertices[1]) || !mesh.contains(vertices[2]))
  {
    return false;
  }

  if (vertices[0] == vertices[1] || vertices[1] == vertices[2] || vertices[2] == vertices[0])
  {
    return false;
  }

  if (connectivity.contains_face_key(connectivity.make_face_key(vertices)))
  {
    return false;
  }

  for (typename Mesh::size_type i = 0; i < vertices.size(); ++i)
  {
    typename Mesh::VertexHandle const from = vertices[i];
    typename Mesh::VertexHandle const to = vertices[(i + 1) % vertices.size()];
    DirectedEdgeKey const key{from.get_value(), to.get_value()};
    DirectedEdgeKey const oppositeKey{to.get_value(), from.get_value()};

    if (connectivity.find_directed_edge(key).is_valid())
    {
      return false;
    }

    typename Mesh::HalfedgeHandle const opposite = connectivity.find_directed_edge(oppositeKey);
    if (opposite.is_valid() && connectivity.halfedge(opposite).twin.is_valid())
    {
      return false;
    }
  }

  return true;
}

} // namespace detail

// Adds a triangle face spanned by the three vertices (in order) to the mesh, reusing existing
// halfedges/edges where the neighbouring triangle already created them. Returns an invalid
// FaceHandle if the triangle cannot be added (duplicate face, non-manifold edge, degenerate
// vertices, ...). Strongly exception-safe: on failure the mesh is rolled back to its prior state.
//
// This is a write algorithm built on the mesh's low-level connectivity kernel
// (mesh.connectivity()); it maintains all mesh invariants itself.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle
add_triangle(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
             const std::array<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle, 3>& triangleVertices)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using Halfedge = typename Mesh::Halfedge;
  using size_type = typename Mesh::size_type;

  if (!detail::can_add_triangle(mesh, triangleVertices))
  {
    return FaceHandle{};
  }

  auto connectivity = mesh.connectivity();
  using DirectedEdgeKey = typename decltype(connectivity)::DirectedEdgeKey;
  using FaceKey = typename decltype(connectivity)::FaceKey;

  FaceKey const faceKey = connectivity.make_face_key(triangleVertices);
  std::array<HalfedgeHandle, 3> triangleHalfedges{};
  std::array<HalfedgeHandle, 3> previousVertexHalfedges{};
  std::array<size_type, 3> previousVertexHalfedgeCounts{};
  std::array<HalfedgeHandle, 3> oppositeHalfedges{};
  std::array<DirectedEdgeKey, 3> directedEdgeKeys{};
  size_type newEdgeCount = 0;

  for (size_type i = 0; i < triangleVertices.size(); ++i)
  {
    VertexHandle const from = triangleVertices[i];
    VertexHandle const to = triangleVertices[(i + 1) % triangleVertices.size()];
    directedEdgeKeys[i] = DirectedEdgeKey{from.get_value(), to.get_value()};
    DirectedEdgeKey const oppositeKey{to.get_value(), from.get_value()};

    previousVertexHalfedges[i] = connectivity.vertex(from).halfedge;
    previousVertexHalfedgeCounts[i] = connectivity.vertex_halfedges(from).size();
    connectivity.reserve_vertex_halfedges(from, 1);

    HalfedgeHandle const opposite = connectivity.find_directed_edge(oppositeKey);
    if (opposite.is_valid())
    {
      oppositeHalfedges[i] = opposite;
    }
    else
    {
      ++newEdgeCount;
    }
  }

  connectivity.reserve_faces(1);
  connectivity.reserve_halfedges(triangleHalfedges.size());
  connectivity.reserve_edges(newEdgeCount);
  connectivity.reserve_directed_edges(directedEdgeKeys.size());
  connectivity.reserve_face_keys(1);

  size_type const faceCount = connectivity.face_count();
  size_type const halfedgeCount = connectivity.halfedge_count();
  size_type const edgeCount = connectivity.edge_count();

  FaceHandle const face = connectivity.new_face();

  auto rollback = [&]() noexcept {
    for (DirectedEdgeKey const key : directedEdgeKeys)
    {
      connectivity.erase_directed_edge(key);
    }
    connectivity.erase_face_key(faceKey);

    for (size_type i = 0; i < triangleVertices.size(); ++i)
    {
      connectivity.vertex(triangleVertices[i]).halfedge = previousVertexHalfedges[i];
      connectivity.resize_vertex_halfedges(triangleVertices[i], previousVertexHalfedgeCounts[i]);

      if (oppositeHalfedges[i].is_valid() && connectivity.contains(oppositeHalfedges[i]))
      {
        Halfedge& oppositeHalfedge = connectivity.halfedge(oppositeHalfedges[i]);
        if (oppositeHalfedge.twin == triangleHalfedges[i])
        {
          oppositeHalfedge.twin = HalfedgeHandle{};
        }
      }
    }

    connectivity.resize_edges(edgeCount);
    connectivity.resize_halfedges(halfedgeCount);
    connectivity.resize_faces(faceCount);
  };

  try
  {
    for (size_type i = 0; i < triangleHalfedges.size(); ++i)
    {
      triangleHalfedges[i] = connectivity.new_halfedge();
    }

    for (size_type i = 0; i < triangleHalfedges.size(); ++i)
    {
      size_type const nextIndex = (i + 1) % triangleHalfedges.size();
      size_type const prevIndex = (i + triangleHalfedges.size() - 1) % triangleHalfedges.size();

      Halfedge& halfedge = connectivity.halfedge(triangleHalfedges[i]);
      halfedge.vertex = triangleVertices[nextIndex];
      halfedge.next = triangleHalfedges[nextIndex];
      halfedge.prev = triangleHalfedges[prevIndex];
      halfedge.face = face;

      if (!connectivity.vertex(triangleVertices[i]).halfedge.is_valid())
      {
        connectivity.vertex(triangleVertices[i]).halfedge = triangleHalfedges[i];
      }
      connectivity.vertex_halfedges(triangleVertices[i]).push_back(triangleHalfedges[i]);
    }

    for (size_type i = 0; i < triangleHalfedges.size(); ++i)
    {
      Halfedge& halfedge = connectivity.halfedge(triangleHalfedges[i]);

      if (oppositeHalfedges[i].is_valid())
      {
        Halfedge& oppositeHalfedge = connectivity.halfedge(oppositeHalfedges[i]);

        halfedge.twin = oppositeHalfedges[i];
        oppositeHalfedge.twin = triangleHalfedges[i];
        halfedge.edge = oppositeHalfedge.edge;
      }
      else
      {
        EdgeHandle const edge = connectivity.new_edge(triangleHalfedges[i]);
        halfedge.edge = edge;
      }

      connectivity.insert_directed_edge(directedEdgeKeys[i], triangleHalfedges[i]);
    }

    connectivity.insert_face_key(faceKey);
    connectivity.face(face).set_halfedgehandle(triangleHalfedges.front());
  }
  catch (...)
  {
    rollback();
    throw;
  }

  return face;
}

template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle
add_triangle(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
             typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle first,
             typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle second,
             typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle third)
{
  return add_triangle(mesh, {first, second, third});
}

} // namespace Geometry

#endif // GEOMETRY_MESH_ADDTRIANGLE_HPP
