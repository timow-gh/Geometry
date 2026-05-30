#ifndef GEOMETRY_TRIANGLEHALFEDGEMESH_HPP
#define GEOMETRY_TRIANGLEHALFEDGEMESH_HPP

#include "Geometry/Handle.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <unordered_map>
#include <unordered_set>
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
    m_vertices.reserve(m_vertices.size() + 1);
    m_vertexHalfedges.reserve(m_vertexHalfedges.size() + 1);
    m_vertices.push_back(Vertex{position, HalfedgeHandle{}});
    m_vertexHalfedges.emplace_back();
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

    FaceKey const faceKey = make_face_key(triangleVertices);
    FaceHandle const face = make_handle<FaceHandle>(m_faces.size());
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

      previousVertexHalfedges[i] = get_vertex(from).halfedge;
      previousVertexHalfedgeCounts[i] = m_vertexHalfedges[handle_index(from)].size();
      m_vertexHalfedges[handle_index(from)].reserve(m_vertexHalfedges[handle_index(from)].size() + 1);

      auto const oppositeIt = m_directedEdges.find(oppositeKey);
      if (oppositeIt != m_directedEdges.end())
      {
        oppositeHalfedges[i] = oppositeIt->second;
      }
      else
      {
        ++newEdgeCount;
      }
    }

    m_faces.reserve(m_faces.size() + 1);
    m_halfedges.reserve(m_halfedges.size() + triangleHalfedges.size());
    m_edges.reserve(m_edges.size() + newEdgeCount);
    m_directedEdges.reserve(m_directedEdges.size() + directedEdgeKeys.size());
    m_faceKeys.reserve(m_faceKeys.size() + 1);

    size_type const faceCount = m_faces.size();
    size_type const halfedgeCount = m_halfedges.size();
    size_type const edgeCount = m_edges.size();

    auto rollback = [&]() noexcept {
      for (DirectedEdgeKey const key : directedEdgeKeys)
      {
        m_directedEdges.erase(key);
      }
      m_faceKeys.erase(faceKey);

      for (size_type i = 0; i < triangleVertices.size(); ++i)
      {
        size_type const vertexIndex = handle_index(triangleVertices[i]);
        m_vertices[vertexIndex].halfedge = previousVertexHalfedges[i];
        m_vertexHalfedges[vertexIndex].resize(previousVertexHalfedgeCounts[i]);

        if (oppositeHalfedges[i].is_valid() && contains(oppositeHalfedges[i]))
        {
          Halfedge& oppositeHalfedge = m_halfedges[handle_index(oppositeHalfedges[i])];
          if (oppositeHalfedge.twin == triangleHalfedges[i])
          {
            oppositeHalfedge.twin = HalfedgeHandle{};
          }
        }
      }

      m_edges.resize(edgeCount);
      m_halfedges.resize(halfedgeCount);
      m_faces.resize(faceCount);
    };

    try
    {
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
        m_vertexHalfedges[handle_index(triangleVertices[i])].push_back(triangleHalfedges[i]);
      }

      for (size_type i = 0; i < triangleHalfedges.size(); ++i)
      {
        Halfedge& halfedge = get_halfedge(triangleHalfedges[i]);

        if (oppositeHalfedges[i].is_valid())
        {
          Halfedge& oppositeHalfedge = get_halfedge(oppositeHalfedges[i]);

          halfedge.twin = oppositeHalfedges[i];
          oppositeHalfedge.twin = triangleHalfedges[i];
          halfedge.edge = oppositeHalfedge.edge;
        }
        else
        {
          EdgeHandle const edge = make_handle<EdgeHandle>(m_edges.size());
          m_edges.push_back(Edge{triangleHalfedges[i]});
          halfedge.edge = edge;
        }

        m_directedEdges.emplace(directedEdgeKeys[i], triangleHalfedges[i]);
      }

      m_faceKeys.insert(faceKey);
      get_face(face).halfedge = triangleHalfedges.front();
    }
    catch (...)
    {
      rollback();
      throw;
    }

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
    std::vector<unsigned char> visited(m_halfedges.size(), 0U);
    append_halfedges_around_vertex_fan(vertex, get_vertex(vertex).halfedge, result, visited);

    for (HalfedgeHandle const halfedge : m_vertexHalfedges[handle_index(vertex)])
    {
      append_halfedges_around_vertex_fan(vertex, halfedge, result, visited);
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
    if (m_vertexHalfedges.size() != m_vertices.size())
    {
      return false;
    }

    for (size_type i = 0; i < m_halfedges.size(); ++i)
    {
      const Halfedge& halfedge = m_halfedges[i];

      if (!contains(halfedge.vertex) || !contains(halfedge.next) || !contains(halfedge.prev) || !contains(halfedge.face)
          || !contains(halfedge.edge))
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_vertices.size(); ++i)
    {
      VertexHandle const vertex = make_handle<VertexHandle>(i);
      HalfedgeHandle const halfedge = m_vertices[i].halfedge;
      if (halfedge.is_valid() && (!contains(halfedge) || unchecked_source_vertex(halfedge) != vertex))
      {
        return false;
      }

      for (HalfedgeHandle const outgoingHalfedge : m_vertexHalfedges[i])
      {
        if (!contains(outgoingHalfedge) || unchecked_source_vertex(outgoingHalfedge) != vertex)
        {
          return false;
        }
      }
    }

    for (size_type i = 0; i < m_faces.size(); ++i)
    {
      FaceHandle const face = make_handle<FaceHandle>(i);
      HalfedgeHandle const firstHalfedge = m_faces[i].halfedge;
      if (!contains(firstHalfedge))
      {
        return false;
      }

      HalfedgeHandle const secondHalfedge = m_halfedges[handle_index(firstHalfedge)].next;
      HalfedgeHandle const thirdHalfedge = m_halfedges[handle_index(secondHalfedge)].next;
      if (firstHalfedge == secondHalfedge || secondHalfedge == thirdHalfedge || thirdHalfedge == firstHalfedge)
      {
        return false;
      }

      if (m_halfedges[handle_index(thirdHalfedge)].next != firstHalfedge)
      {
        return false;
      }

      if (m_halfedges[handle_index(firstHalfedge)].face != face || m_halfedges[handle_index(secondHalfedge)].face != face
          || m_halfedges[handle_index(thirdHalfedge)].face != face)
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_edges.size(); ++i)
    {
      EdgeHandle const edge = make_handle<EdgeHandle>(i);
      HalfedgeHandle const halfedge = m_edges[i].halfedge;
      if (!contains(halfedge) || m_halfedges[handle_index(halfedge)].edge != edge)
      {
        return false;
      }
    }

    if (m_directedEdges.size() != m_halfedges.size() || m_faceKeys.size() != m_faces.size())
    {
      return false;
    }

    for (size_type i = 0; i < m_halfedges.size(); ++i)
    {
      HalfedgeHandle const halfedgeHandle = make_handle<HalfedgeHandle>(i);
      const Halfedge& halfedge = m_halfedges[i];

      if (m_halfedges[handle_index(halfedge.next)].prev != halfedgeHandle
          || m_halfedges[handle_index(halfedge.prev)].next != halfedgeHandle)
      {
        return false;
      }

      if (halfedge.twin.is_valid())
      {
        if (!contains(halfedge.twin))
        {
          return false;
        }

        const Halfedge& twin = m_halfedges[handle_index(halfedge.twin)];
        if (twin.twin != halfedgeHandle || twin.edge != halfedge.edge
            || unchecked_source_vertex(halfedgeHandle) != unchecked_target_vertex(halfedge.twin)
            || unchecked_target_vertex(halfedgeHandle) != unchecked_source_vertex(halfedge.twin))
        {
          return false;
        }
      }

      DirectedEdgeKey const key{unchecked_source_vertex(halfedgeHandle).get_value(), unchecked_target_vertex(halfedgeHandle).get_value()};
      auto const directedEdgeIt = m_directedEdges.find(key);
      if (directedEdgeIt == m_directedEdges.end() || directedEdgeIt->second != halfedgeHandle)
      {
        return false;
      }

      VertexHandle const source = unchecked_source_vertex(halfedgeHandle);
      bool foundInVertexHalfedges = false;
      for (HalfedgeHandle const outgoingHalfedge : m_vertexHalfedges[handle_index(source)])
      {
        if (outgoingHalfedge == halfedgeHandle)
        {
          foundInVertexHalfedges = true;
          break;
        }
      }

      if (!foundInVertexHalfedges)
      {
        return false;
      }
    }

    for (size_type i = 0; i < m_faces.size(); ++i)
    {
      FaceHandle const face = make_handle<FaceHandle>(i);
      if (m_faceKeys.find(make_face_key(vertices_around_face(face))) == m_faceKeys.end())
      {
        return false;
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

  struct FaceKey
  {
    std::array<handle_value_type, 3> vertices{};

    GEO_NODISCARD constexpr bool operator==(const FaceKey& other) const noexcept { return vertices == other.vertices; }
  };

  struct FaceKeyHash
  {
    GEO_NODISCARD size_type operator()(const FaceKey& key) const noexcept
    {
      size_type seed = 0;
      for (handle_value_type vertex : key.vertices)
      {
        size_type const value = static_cast<size_type>(vertex);
        seed ^= value + 0x9e3779b9U + (seed << 6U) + (seed >> 2U);
      }
      return seed;
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

    if (m_faceKeys.find(make_face_key(vertices)) != m_faceKeys.end())
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

  GEO_NODISCARD static FaceKey make_face_key(const std::array<VertexHandle, 3>& vertices) noexcept
  {
    FaceKey key{{vertices[0].get_value(), vertices[1].get_value(), vertices[2].get_value()}};
    std::sort(key.vertices.begin(), key.vertices.end());
    return key;
  }

  void append_halfedges_around_vertex_fan(VertexHandle vertex,
                                          HalfedgeHandle start,
                                          std::vector<HalfedgeHandle>& result,
                                          std::vector<unsigned char>& visited) const
  {
    if (!contains(start) || visited[handle_index(start)] != 0U || source_vertex(start) != vertex)
    {
      return;
    }

    HalfedgeHandle first = start;
    while (true)
    {
      HalfedgeHandle const previous = get_halfedge(first).prev;
      HalfedgeHandle const opposite = get_halfedge(previous).twin;
      if (!opposite.is_valid() || !contains(opposite) || source_vertex(opposite) != vertex || opposite == start
          || visited[handle_index(opposite)] != 0U)
      {
        break;
      }

      first = opposite;
    }

    HalfedgeHandle current = first;
    while (contains(current) && visited[handle_index(current)] == 0U && source_vertex(current) == vertex)
    {
      result.push_back(current);
      visited[handle_index(current)] = 1U;

      HalfedgeHandle const twin = get_halfedge(current).twin;
      if (!twin.is_valid() || !contains(twin))
      {
        break;
      }

      current = get_halfedge(twin).next;
    }
  }

  GEO_NODISCARD VertexHandle unchecked_source_vertex(HalfedgeHandle halfedge) const noexcept
  {
    return m_halfedges[handle_index(m_halfedges[handle_index(halfedge)].prev)].vertex;
  }

  GEO_NODISCARD VertexHandle unchecked_target_vertex(HalfedgeHandle halfedge) const noexcept
  {
    return m_halfedges[handle_index(halfedge)].vertex;
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
  std::vector<std::vector<HalfedgeHandle>> m_vertexHalfedges;
  std::vector<Halfedge> m_halfedges;
  std::vector<Face> m_faces;
  std::vector<Edge> m_edges;
  std::unordered_map<DirectedEdgeKey, HalfedgeHandle, DirectedEdgeKeyHash> m_directedEdges;
  std::unordered_set<FaceKey, FaceKeyHash> m_faceKeys;
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
