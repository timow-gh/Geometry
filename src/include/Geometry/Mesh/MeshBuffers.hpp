#ifndef GEOMETRY_MESH_MESHBUFFERS_HPP
#define GEOMETRY_MESH_MESHBUFFERS_HPP

#include "Geometry/Mesh/MeshNormals.hpp"
#include "Geometry/Mesh/MeshResult.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"

#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace Geometry {

/**
 * \brief Index type emitted by the index buffers.
 *
 * The capacity/range checks key off this alias so the emitted element type is the single
 * source of truth: change it here and the range check (mesh_buffer_index_fits) follows.
 */
using BufferIndex = std::uint32_t;

/** \brief Outcome reported by the mesh buffer builders; Ok on success. */
enum class MeshBufferStatus {
    Ok,
    NonFinitePosition,
    PositionOutOfRange,
    CapacityExceeded
};

/** \brief Buffer values plus a failure reason; empty \c values when \c error is set. */
template <typename TValue>
struct MeshBufferResult {
    std::vector<TValue> values;
    MeshBufferStatus error = MeshBufferStatus::Ok;

    GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
    GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

namespace detail {
/**
 * \internal
 * \brief Whether \p count elements of \p stride fit within \p capacity.
 *
 * Divides capacity by stride before multiplying so the size arithmetic cannot overflow.
 */
constexpr bool mesh_buffer_count_fits(std::size_t count, std::size_t stride, std::size_t capacity) noexcept {
    return stride != 0 && count <= capacity / stride;
}

/** \internal \brief Whether \p index is representable in the target index type. */
template <typename TTarget, typename TIndex>
constexpr bool mesh_buffer_index_fits(TIndex index) noexcept {
    return std::in_range<TTarget>(index);
}
} // namespace detail

/**
 * \brief Flat XYZ vertex positions in vertex-handle order, including isolated vertices.
 *
 * Requires valid mesh connectivity; open meshes and isolated vertices are supported.
 *
 * \return Float triples per vertex, or an empty result whose \c error names the failure.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<float> make_vertex_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<float> result;
    if (!detail::mesh_buffer_count_fits(mesh.vertex_count(), 3, result.values.max_size()))
        return {{}, MeshBufferStatus::CapacityExceeded};
    result.values.reserve(mesh.vertex_count() * 3);

    const T limit = static_cast<T>(std::numeric_limits<float>::max());
    for (const auto vertex: mesh.vertices()) {
        const auto& position = mesh.get_vertex(vertex).position;
        for (typename linal::vec3<T>::size_type coordinate = 0; coordinate < 3; ++coordinate) {
            const T value = position[coordinate];
            if (!std::isfinite(value))
                return {{}, MeshBufferStatus::NonFinitePosition};
            if (value < -limit || value > limit)
                return {{}, MeshBufferStatus::PositionOutOfRange};
            result.values.push_back(static_cast<float>(value));
        }
    }
    return result;
}

/**
 * \brief Three vertex indices per face in face-handle order, preserving face winding.
 *
 * \return Index triples per face, or an empty result whose \c error names the failure.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<BufferIndex>
make_triangle_index_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<BufferIndex> result;
    if (!detail::mesh_buffer_count_fits(mesh.face_count(), 3, result.values.max_size()))
        return {{}, MeshBufferStatus::CapacityExceeded};
    result.values.reserve(mesh.face_count() * 3);

    for (const auto face: mesh.faces()) {
        for (auto vertex = mesh.vertices(face).circulator(); vertex.is_valid(); ++vertex) {
            const auto index = vertex.get_vertexhandle().get_value();
            if (!detail::mesh_buffer_index_fits<BufferIndex>(index))
                return {{}, MeshBufferStatus::CapacityExceeded};
            result.values.push_back(static_cast<BufferIndex>(index));
        }
    }
    return result;
}

/**
 * \brief Two endpoint indices per edge in edge-handle order, including boundary edges and
 * triangulation diagonals.
 *
 * Each undirected edge is emitted exactly once.
 *
 * \return Index pairs per edge, or an empty result whose \c error names the failure.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<BufferIndex> make_edge_index_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<BufferIndex> result;
    if (!detail::mesh_buffer_count_fits(mesh.edge_count(), 2, result.values.max_size()))
        return {{}, MeshBufferStatus::CapacityExceeded};
    result.values.reserve(mesh.edge_count() * 2);
    
    for (const auto edge: mesh.edges()) {
        const auto halfedge = mesh.get_edge(edge).halfedge;
        const auto source = mesh.source_vertex(halfedge).get_value();
        const auto target = mesh.target_vertex(halfedge).get_value();
        if (!detail::mesh_buffer_index_fits<BufferIndex>(source) || !detail::mesh_buffer_index_fits<BufferIndex>(target))
            return {{}, MeshBufferStatus::CapacityExceeded};
        result.values.push_back(static_cast<BufferIndex>(source));
        result.values.push_back(static_cast<BufferIndex>(target));
    }
    return result;
}

/**
 * \brief Render buffers for a lit surface: parallel position and normal arrays plus
 * triangle and wireframe-segment indices.
 *
 * Renderers bind one normal per vertex, so a mesh vertex carrying different corner normals
 * across a crease is duplicated ("split") -- each distinct normal gets its own render
 * vertex and the indices are remapped. With no crease edges this is one render vertex per
 * mesh vertex.
 */
struct MeshRenderBuffers {
    std::vector<float> positions;       ///< 3 floats per render vertex
    std::vector<float> normals;         ///< 3 floats per render vertex, parallel to positions
    std::vector<BufferIndex> triangles; ///< 3 indices per face
    std::vector<BufferIndex> segments;  ///< 2 indices per edge (wireframe overlay)
    MeshBufferStatus error = MeshBufferStatus::Ok;

    GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
    GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
    GEO_NODISCARD std::size_t vertex_count() const noexcept { return positions.size() / 3; }
};

namespace detail {
/**
 * \internal
 * \brief Appends a float triple, validating finiteness and float range.
 *
 * \return MeshBufferStatus::Ok on success, otherwise the reason the triple was rejected.
 */
template <typename T>
GEO_NODISCARD MeshBufferStatus mesh_append_float3(std::vector<float>& out, const linal::vec3<T>& value) {
    const T limit = static_cast<T>(std::numeric_limits<float>::max());
    for (typename linal::vec3<T>::size_type coordinate = 0; coordinate < 3; ++coordinate) {
        const T component = value[coordinate];
        if (!std::isfinite(component))
            return MeshBufferStatus::NonFinitePosition;
        if (component < -limit || component > limit)
            return MeshBufferStatus::PositionOutOfRange;
        out.push_back(static_cast<float>(component));
    }
    return MeshBufferStatus::Ok;
}
} // namespace detail

/**
 * \brief Positions, per-vertex normals, and remapped triangle and wireframe-segment
 * indices for a lit surface.
 *
 * Splits shared vertices along creases so every render vertex has a single normal (see
 * MeshRenderBuffers). Requires valid connectivity; isolated vertices are emitted with a
 * zero normal.
 *
 * \return Populated MeshRenderBuffers, or an empty result whose \c error names the failure
 * (e.g. a degenerate face with no usable normal).
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshRenderBuffers make_render_buffers(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    using HalfedgeHandle = typename TriangleHalfedgeMesh<T, 3, TIndex>::HalfedgeHandle;

    const auto normals = compute_halfedge_normals(mesh);
    if (!normals)
        return {{}, {}, {}, {}, MeshBufferStatus::NonFinitePosition}; // degenerate face -> no usable normal

    MeshRenderBuffers result;

    // Per mesh vertex, the render copies already emitted (their normal and render id). A corner
    // reuses a copy with the identical normal (same smooth sector) or creates a new one.
    struct Copy { linal::vec3<T> normal; BufferIndex id; };
    std::vector<std::vector<Copy>> copies(mesh.vertex_count());
    // One representative render id per mesh vertex, used to remap wireframe segment endpoints
    // (all copies share a position, so any copy yields the same line).
    std::vector<BufferIndex> representative(mesh.vertex_count(), std::numeric_limits<BufferIndex>::max());

    const auto append_vertex = [&](TIndex vertexValue, const linal::vec3<T>& normal, BufferIndex& outId) -> MeshBufferStatus {
        const auto slot = static_cast<std::size_t>(vertexValue);
        for (const auto& copy: copies[slot])
            if (copy.normal == normal) { outId = copy.id; return MeshBufferStatus::Ok; }
        const auto newId = result.positions.size() / 3;
        if (!detail::mesh_buffer_index_fits<BufferIndex>(newId))
            return MeshBufferStatus::CapacityExceeded;
        const auto id = static_cast<BufferIndex>(newId);
        if (const auto error = detail::mesh_append_float3(result.positions, mesh.get_vertex(typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle{vertexValue}).position); error != MeshBufferStatus::Ok)
            return error;
        if (const auto error = detail::mesh_append_float3(result.normals, normal); error != MeshBufferStatus::Ok)
            return error;
        copies[slot].push_back(Copy{normal, id});
        if (representative[slot] == std::numeric_limits<BufferIndex>::max())
            representative[slot] = id;
        outId = id;
        return MeshBufferStatus::Ok;
    };

    result.triangles.reserve(mesh.face_count() * 3);
    for (const auto face: mesh.faces()) {
        for (const HalfedgeHandle corner: mesh.halfedges_around_face(face)) {
            const auto vertexValue = mesh.target_vertex(corner).get_value();
            const auto& normal = normals.values[static_cast<std::size_t>(corner.get_value())];
            BufferIndex id{};
            if (const auto error = append_vertex(vertexValue, normal, id); error != MeshBufferStatus::Ok)
                return {{}, {}, {}, {}, error};
            result.triangles.push_back(id);
        }
    }

    // Isolated vertices (no incident face) never got a representative; emit them as bare positions
    // with a zero normal so vertex-handle numbering stays meaningful for segment remapping.
    for (const auto vertex: mesh.vertices()) {
        const auto slot = static_cast<std::size_t>(vertex.get_value());
        if (representative[slot] == std::numeric_limits<BufferIndex>::max()) {
            BufferIndex id{};
            if (const auto error = append_vertex(vertex.get_value(), linal::vec3<T>{}, id); error != MeshBufferStatus::Ok)
                return {{}, {}, {}, {}, error};
        }
    }

    // Only crease (feature) edges go into the wireframe overlay, so the visualization shows the
    // shape's sharp edges (cone rim + apex, box edges) rather than the full triangulation.
    for (const auto edge: mesh.edges()) {
        if (!mesh.is_crease(edge))
            continue;
        const auto halfedge = mesh.get_edge(edge).halfedge;
        result.segments.push_back(representative[static_cast<std::size_t>(mesh.source_vertex(halfedge).get_value())]);
        result.segments.push_back(representative[static_cast<std::size_t>(mesh.target_vertex(halfedge).get_value())]);
    }

    return result;
}

} // namespace Geometry
#endif // GEOMETRY_MESH_MESHBUFFERS_HPP
