#ifndef GEOMETRY_MESH_MESHBUFFERS_HPP
#define GEOMETRY_MESH_MESHBUFFERS_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"

#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace Geometry {

enum class MeshBufferError {
    None,
    NonFinitePosition,
    PositionOutOfRange,
    CapacityExceeded
};

template <typename TValue>
struct MeshBufferResult {
    std::vector<TValue> values;
    MeshBufferError error = MeshBufferError::None;

    GEO_NODISCARD bool has_value() const noexcept { return error == MeshBufferError::None; }
    GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

namespace detail {
// Division checks both vector capacity and size arithmetic before multiplication.
constexpr bool mesh_buffer_count_fits(std::size_t count, std::size_t stride, std::size_t capacity) noexcept {
    return stride != 0 && count <= capacity / stride;
}

template <typename TIndex>
constexpr bool mesh_buffer_index_fits(TIndex index) noexcept {
    return std::in_range<std::uint32_t>(index);
}
} // namespace detail

// These conversions require valid mesh connectivity. Open meshes and isolated
// vertices are supported. All buffers share vertex-handle numbering; reported
// failures contain no values. Container allocation failures propagate normally.

/** Flat XYZ values in vertex-handle order, including isolated vertices. */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<float> make_vertex_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<float> result;
    if (!detail::mesh_buffer_count_fits(mesh.vertex_count(), 3, result.values.max_size()))
        return {{}, MeshBufferError::CapacityExceeded};
    result.values.reserve(mesh.vertex_count() * 3);
    const T limit = static_cast<T>(std::numeric_limits<float>::max());
    for (const auto vertex: mesh.vertices()) {
        const auto& position = mesh.get_vertex(vertex).position;
        for (typename linal::vec3<T>::size_type coordinate = 0; coordinate < 3; ++coordinate) {
            const T value = position[coordinate];
            if (!std::isfinite(value))
                return {{}, MeshBufferError::NonFinitePosition};
            if (value < -limit || value > limit)
                return {{}, MeshBufferError::PositionOutOfRange};
            result.values.push_back(static_cast<float>(value));
        }
    }
    return result;
}

/** Three indices per face in face-handle order, preserving the face winding. */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<std::uint32_t>
make_triangle_index_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<std::uint32_t> result;
    if (!detail::mesh_buffer_count_fits(mesh.face_count(), 3, result.values.max_size()))
        return {{}, MeshBufferError::CapacityExceeded};
    result.values.reserve(mesh.face_count() * 3);
    for (const auto face: mesh.faces()) {
        for (auto vertex = mesh.vertices(face).circulator(); vertex.is_valid(); ++vertex) {
            const auto index = vertex.get_vertexhandle().get_value();
            if (!detail::mesh_buffer_index_fits(index))
                return {{}, MeshBufferError::CapacityExceeded};
            result.values.push_back(static_cast<std::uint32_t>(index));
        }
    }
    return result;
}

/** Two endpoint indices per edge in edge-handle order, including boundary edges
 * and triangulation diagonals. Each undirected edge is emitted exactly once. */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<std::uint32_t> make_edge_index_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<std::uint32_t> result;
    if (!detail::mesh_buffer_count_fits(mesh.edge_count(), 2, result.values.max_size()))
        return {{}, MeshBufferError::CapacityExceeded};
    result.values.reserve(mesh.edge_count() * 2);
    for (const auto edge: mesh.edges()) {
        const auto halfedge = mesh.get_edge(edge).halfedge;
        const auto source = mesh.source_vertex(halfedge).get_value();
        const auto target = mesh.target_vertex(halfedge).get_value();
        if (!detail::mesh_buffer_index_fits(source) || !detail::mesh_buffer_index_fits(target))
            return {{}, MeshBufferError::CapacityExceeded};
        result.values.push_back(static_cast<std::uint32_t>(source));
        result.values.push_back(static_cast<std::uint32_t>(target));
    }
    return result;
}

} // namespace Geometry
#endif // GEOMETRY_MESH_MESHBUFFERS_HPP
