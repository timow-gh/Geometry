#ifndef GEOMETRY_TRIANGLEHALFEDGEMESH_HPP
#define GEOMETRY_TRIANGLEHALFEDGEMESH_HPP

#include "Geometry/Handle.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <unordered_map>
#include <vector>

namespace Geometry
{

template <typename T, std::uint8_t D, typename TIndex = std::uint32_t>
class TriangleHalfedgeMesh {
public:
  struct VertexHandleTag;
  struct HalfedgeHandleTag;
  struct FaceHandleTag;
  struct EdgeHandleTag;

  using value_type = T;
  using handle_value_type = TIndex;
  using size_type = std::size_t;
  using vec_t = linal::vec<T, D>;

  using VertexHandle = Handle<handle_value_type, VertexHandleTag>;
  using HalfedgeHandle = Handle<handle_value_type, HalfedgeHandleTag>;
  using FaceHandle = Handle<handle_value_type, FaceHandleTag>;
  using EdgeHandle = Handle<handle_value_type, EdgeHandleTag>;

  struct Vertex
  {
    vec_t position{};
    HalfedgeHandle halfedge{};
  };

  struct Halfedge
  {
    // The stored vertex is the target vertex. The face is on the left when walking source -> target.
    VertexHandle vertex{};
    HalfedgeHandle twin{};
    HalfedgeHandle next{};
    HalfedgeHandle prev{};
    FaceHandle face{};
    EdgeHandle edge{};
  };

  struct Face
  {
    HalfedgeHandle halfedge{};
  };

  struct Edge
  {
    HalfedgeHandle halfedge{};
  };

  GEO_NODISCARD VertexHandle add_vertex(const vec_t& position)
  {
    VertexHandle const handle = make_handle<VertexHandle>(m_vertices.size());
    m_vertices.push_back(Vertex{position, HalfedgeHandle{}});
    return handle;
  }

  GEO_NODISCARD FaceHandle add_triangle(VertexHandle first, VertexHandle second, VertexHandle third)
  {
    return add_triangle({first, second, third});
  }

  GEO_NODISCARD FaceHandle add_triangle(const std::array<VertexHandle, 3>& triangleVertices)
  {
    if (!can_add_triangle(triangleVertices))
    {
      return FaceHandle{};
    }

    FaceHandle const face = make_handle<FaceHandle>(m_faces.size());
    std::array<HalfedgeHandle, 3> triangleHalfedges{};

    m_faces.push_back(Face{});

    for (size_type i = 0; i < triangleHalfedges.size(); ++i)
    {
      triangleHalfedges[i] = make_handle<HalfedgeHandle>(m_halfedges.size());
      m_halfedges.push_back(Halfedge{});
    }

    for (size_type i = 0; i < triangleHalfedges.size(); ++i)
    {
      size_type const nextIndex = (i + 1) % triangleHalfedges.size();
      size_type const prevIndex = (i + triangleHalfedges.size() - 1) % triangleHalfedges.size();

      Halfedge& halfedge = get_halfedge(triangleHalfedges[i]);
      halfedge.vertex = triangleVertices[nextIndex];
      halfedge.next = triangleHalfedges[nextIndex];
      halfedge.prev = triangleHalfedges[prevIndex];
      halfedge.face = face;

      if (!get_vertex(triangleVertices[i]).halfedge.is_valid())
      {
        get_vertex(triangleVertices[i]).halfedge = triangleHalfedges[i];
      }
    }

    for (size_type i = 0; i < triangleHalfedges.size(); ++i)
    {
      VertexHandle const from = triangleVertices[i];
      VertexHandle const to = triangleVertices[(i + 1) % triangleVertices.size()];
      DirectedEdgeKey const key{from.get_value(), to.get_value()};
      DirectedEdgeKey const oppositeKey{to.get_value(), from.get_value()};

      auto const oppositeIt = m_directedEdges.find(oppositeKey);
      Halfedge& halfedge = get_halfedge(triangleHalfedges[i]);

      if (oppositeIt != m_directedEdges.end())
      {
        HalfedgeHandle const opposite = oppositeIt->second;
        Halfedge& oppositeHalfedge = get_halfedge(opposite);

        halfedge.twin = opposite;
        oppositeHalfedge.twin = triangleHalfedges[i];
        halfedge.edge = oppositeHalfedge.edge;
      }
      else
      {
        EdgeHandle const edge = make_handle<EdgeHandle>(m_edges.size());
        m_edges.push_back(Edge{triangleHalfedges[i]});
        halfedge.edge = edge;
      }

      m_directedEdges.emplace(key, triangleHalfedges[i]);
    }

    get_face(face).halfedge = triangleHalfedges.front();
    return face;
  }

  GEO_NODISCARD constexpr size_type vertex_count() const noexcept { return m_vertices.size(); }
  GEO_NODISCARD constexpr size_type halfedge_count() const noexcept { return m_halfedges.size(); }
  GEO_NODISCARD constexpr size_type face_count() const noexcept { return m_faces.size(); }
  GEO_NODISCARD constexpr size_type edge_count() const noexcept { return m_edges.size(); }
  GEO_NODISCARD constexpr bool empty() const noexcept { return m_vertices.empty() && m_faces.empty(); }

  GEO_NODISCARD bool contains(VertexHandle handle) const noexcept { return handle_in_range(handle, m_vertices.size()); }
  GEO_NODISCARD bool contains(HalfedgeHandle handle) const noexcept { return handle_in_range(handle, m_halfedges.size()); }
  GEO_NODISCARD bool contains(FaceHandle handle) const noexcept { return handle_in_range(handle, m_faces.size()); }
  GEO_NODISCARD bool contains(EdgeHandle handle) const noexcept { return handle_in_range(handle, m_edges.size()); }

  GEO_NODISCARD Vertex& get_vertex(VertexHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_vertices[handle_index(handle)];
  }

  GEO_NODISCARD const Vertex& get_vertex(VertexHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_vertices[handle_index(handle)];
  }

  GEO_NODISCARD Halfedge& get_halfedge(HalfedgeHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_halfedges[handle_index(handle)];
  }

  GEO_NODISCARD const Halfedge& get_halfedge(HalfedgeHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_halfedges[handle_index(handle)];
  }

  GEO_NODISCARD Face& get_face(FaceHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_faces[handle_index(handle)];
  }

  GEO_NODISCARD const Face& get_face(FaceHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_faces[handle_index(handle)];
  }

  GEO_NODISCARD Edge& get_edge(EdgeHandle handle) noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_edges[handle_index(handle)];
  }

  GEO_NODISCARD const Edge& get_edge(EdgeHandle handle) const noexcept
  {
    GEO_ASSERT(contains(handle));
    return m_edges[handle_index(handle)];
  }

  GEO_NODISCARD const vec_t& get_position(VertexHandle handle) const noexcept { return get_vertex(handle).position; }

  void set_position(VertexHandle handle, const vec_t& position) noexcept { get_vertex(handle).position = position; }

  GEO_NODISCARD std::array<HalfedgeHandle, 3> halfedges_around_face(FaceHandle face) const noexcept
  {
    GEO_ASSERT(contains(face));

    std::array<HalfedgeHandle, 3> halfedges{};
    halfedges[0] = get_face(face).halfedge;
    halfedges[1] = get_halfedge(halfedges[0]).next;
    halfedges[2] = get_halfedge(halfedges[1]).next;
    GEO_ASSERT(get_halfedge(halfedges[2]).next == halfedges[0]);
    return halfedges;
  }

  GEO_NODISCARD std::array<VertexHandle, 3> vertices_around_face(FaceHandle face) const noexcept
  {
    std::array<HalfedgeHandle, 3> const halfedges = halfedges_around_face(face);
    return {source_vertex(halfedges[0]), target_vertex(halfedges[0]), target_vertex(halfedges[1])};
  }

  GEO_NODISCARD std::vector<HalfedgeHandle> halfedges_around_vertex(VertexHandle vertex) const
  {
    GEO_ASSERT(contains(vertex));

    std::vector<HalfedgeHandle> result;
    for (size_type i = 0; i < m_halfedges.size(); ++i)
    {
      HalfedgeHandle const halfedge = make_handle<HalfedgeHandle>(i);
      if (source_vertex(halfedge) == vertex)
      {
        result.push_back(halfedge);
      }
    }
    return result;
  }

  GEO_NODISCARD std::vector<FaceHandle> faces_around_vertex(VertexHandle vertex) const
  {
    std::vector<HalfedgeHandle> const halfedges = halfedges_around_vertex(vertex);
    std::vector<FaceHandle> result;
    result.reserve(halfedges.size());

    for (HalfedgeHandle halfedge : halfedges)
    {
      result.push_back(get_halfedge(halfedge).face);
    }

    return result;
  }

  GEO_NODISCARD VertexHandle source_vertex(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    HalfedgeHandle const prev = get_halfedge(halfedge).prev;
    GEO_ASSERT(contains(prev));
    return get_halfedge(prev).vertex;
  }

  GEO_NODISCARD VertexHandle target_vertex(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    return get_halfedge(halfedge).vertex;
  }

  GEO_NODISCARD bool is_boundary(HalfedgeHandle halfedge) const noexcept
  {
    GEO_ASSERT(contains(halfedge));
    return !get_halfedge(halfedge).twin.is_valid();
  }

  GEO_NODISCARD bool is_boundary(EdgeHandle edge) const noexcept
  {
    GEO_ASSERT(contains(edge));
    return is_boundary(get_edge(edge).halfedge);
  }

  GEO_NODISCARD bool is_valid() const noexcept
  {
    for (size_type i = 0; i < m_vertices.size(); ++i)
    {
      VertexHandle const vertex = make_handle<VertexHandle>(i);
      HalfedgeHandle const halfedge = get_vertex(vertex).halfedge;
      if (halfedge.is_valid() && (!contains(halfedge) || source_vertex(halfedge) != vertex))
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_faces.size(); ++i)
    {
      FaceHandle const face = make_handle<FaceHandle>(i);
      HalfedgeHandle const halfedge = get_face(face).halfedge;
      if (!contains(halfedge) || get_halfedge(halfedge).face != face)
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_edges.size(); ++i)
    {
      EdgeHandle const edge = make_handle<EdgeHandle>(i);
      HalfedgeHandle const halfedge = get_edge(edge).halfedge;
      if (!contains(halfedge) || get_halfedge(halfedge).edge != edge)
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_halfedges.size(); ++i)
    {
      HalfedgeHandle const halfedgeHandle = make_handle<HalfedgeHandle>(i);
      const Halfedge& halfedge = get_halfedge(halfedgeHandle);

      if (!contains(halfedge.vertex) || !contains(halfedge.next) || !contains(halfedge.prev) || !contains(halfedge.face)
          || !contains(halfedge.edge))
      {
        return false;
      }

      if (get_halfedge(halfedge.next).prev != halfedgeHandle || get_halfedge(halfedge.prev).next != halfedgeHandle)
      {
        return false;
      }

      if (halfedge.twin.is_valid())
      {
        if (!contains(halfedge.twin))
        {
          return false;
        }

        const Halfedge& twin = get_halfedge(halfedge.twin);
        if (twin.twin != halfedgeHandle || twin.edge != halfedge.edge || source_vertex(halfedgeHandle) != target_vertex(halfedge.twin)
            || target_vertex(halfedgeHandle) != source_vertex(halfedge.twin))
        {
          return false;
        }
      }
    }

    return true;
  }

private:
  struct DirectedEdgeKey
  {
    handle_value_type from{};
    handle_value_type to{};

    GEO_NODISCARD constexpr bool operator==(const DirectedEdgeKey& other) const noexcept
    {
      return from == other.from && to == other.to;
    }
  };

  struct DirectedEdgeKeyHash
  {
    GEO_NODISCARD size_type operator()(const DirectedEdgeKey& key) const noexcept
    {
      size_type const from = static_cast<size_type>(key.from);
      size_type const to = static_cast<size_type>(key.to);
      return from ^ (to + 0x9e3779b9U + (from << 6U) + (from >> 2U));
    }
  };

  GEO_NODISCARD bool can_add_triangle(const std::array<VertexHandle, 3>& vertices) const
  {
    if (!contains(vertices[0]) || !contains(vertices[1]) || !contains(vertices[2]))
    {
      return false;
    }

    if (vertices[0] == vertices[1] || vertices[1] == vertices[2] || vertices[2] == vertices[0])
    {
      return false;
    }

    for (size_type i = 0; i < vertices.size(); ++i)
    {
      VertexHandle const from = vertices[i];
      VertexHandle const to = vertices[(i + 1) % vertices.size()];
      DirectedEdgeKey const key{from.get_value(), to.get_value()};
      DirectedEdgeKey const oppositeKey{to.get_value(), from.get_value()};

      if (m_directedEdges.find(key) != m_directedEdges.end())
      {
        return false;
      }

      auto const oppositeIt = m_directedEdges.find(oppositeKey);
      if (oppositeIt != m_directedEdges.end() && get_halfedge(oppositeIt->second).twin.is_valid())
      {
        return false;
      }
    }

    return true;
  }

  template <typename THandle>
  GEO_NODISCARD static THandle make_handle(size_type index) noexcept
  {
    GEO_ASSERT(index < static_cast<size_type>(std::numeric_limits<handle_value_type>::max()));
    return THandle{static_cast<handle_value_type>(index)};
  }

  template <typename THandle>
  GEO_NODISCARD static size_type handle_index(THandle handle) noexcept
  {
    return static_cast<size_type>(handle.get_value());
  }

  template <typename THandle>
  GEO_NODISCARD static bool handle_in_range(THandle handle, size_type count) noexcept
  {
    return handle.is_valid() && handle_index(handle) < count;
  }

  std::vector<Vertex> m_vertices;
  std::vector<Halfedge> m_halfedges;
  std::vector<Face> m_faces;
  std::vector<Edge> m_edges;
  std::unordered_map<DirectedEdgeKey, HalfedgeHandle, DirectedEdgeKeyHash> m_directedEdges;
};

template <typename T>
using TriangleHalfedgeMesh2 = TriangleHalfedgeMesh<T, 2>;
template <typename T>
using TriangleHalfedgeMesh3 = TriangleHalfedgeMesh<T, 3>;

using TriangleHalfedgeMesh2f = TriangleHalfedgeMesh2<float>;
using TriangleHalfedgeMesh3f = TriangleHalfedgeMesh3<float>;

using TriangleHalfedgeMesh2d = TriangleHalfedgeMesh2<double>;
using TriangleHalfedgeMesh3d = TriangleHalfedgeMesh3<double>;

} // namespace Geometry

#endif // GEOMETRY_TRIANGLEHALFEDGEMESH_HPP
