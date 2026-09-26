#ifndef GEOMETRY_MESH_MESHBUFFERS_HPP
#define GEOMETRY_MESH_MESHBUFFERS_HPP

#include "Geometry/Mesh/MeshNormals.hpp"
#include "Geometry/Mesh/MeshResult.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"

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
    CapacityExceeded,
    DegenerateGeometry
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

/**
 * \internal
 * \brief Whether every component of \p value converts to a finite float.
 *
 * \return MeshBufferStatus::Ok when the triple can be emitted, otherwise the reason it cannot.
 */
template <typename T>
GEO_NODISCARD MeshBufferStatus float3_status(const linal::vec3<T>& value) noexcept {
    const T limit = static_cast<T>(std::numeric_limits<float>::max());
    for (typename linal::vec3<T>::size_type coordinate = 0; coordinate < 3; ++coordinate) {
        const T component = value[coordinate];
        if (!std::isfinite(component))
            return MeshBufferStatus::NonFinitePosition;
        if (component < -limit || component > limit)
            return MeshBufferStatus::PositionOutOfRange;
    }
    return MeshBufferStatus::Ok;
}

/**
 * \internal
 * \brief Buffer index of every live vertex, by vertex-handle value: live vertices are numbered
 * consecutively in handle order, the order \c make_vertex_buffer emits them in.
 *
 * Handle values stop matching buffer slots once tombstones exist, so index buffers translate
 * through this map rather than emitting raw handle values. Deleted slots hold an unused value. O(V).
 */
template <typename T, typename TIndex>
GEO_NODISCARD MeshBufferResult<BufferIndex> make_live_vertex_indices(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<BufferIndex> result;
    result.values.assign(mesh.vertex_storage_size(), std::numeric_limits<BufferIndex>::max());
    std::size_t nextIndex = 0;
    for (const auto vertex: mesh.vertices()) {
        if (!mesh_buffer_index_fits<BufferIndex>(nextIndex))
            return {{}, MeshBufferStatus::CapacityExceeded};
        result.values[static_cast<std::size_t>(vertex.get_value())] = static_cast<BufferIndex>(nextIndex++);
    }
    return result;
}
} // namespace detail

/**
 * \brief Flat XYZ vertex positions of the live vertices in vertex-handle order, including isolated
 * vertices.
 *
 * Requires valid mesh connectivity; open meshes, isolated vertices and tombstoned elements are
 * supported. The index buffers address this compacted order.
 *
 * \return Float triples per vertex, or an empty result whose \c error names the failure.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<float> make_vertex_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<float> result;
    if (!detail::mesh_buffer_count_fits(mesh.vertex_count(), 3, result.values.max_size()))
        return {{}, MeshBufferStatus::CapacityExceeded};
    result.values.reserve(mesh.vertex_count() * 3);

    for (const auto vertex: mesh.vertices()) {
        const auto& position = mesh.get_vertex(vertex).position;
        if (const auto error = detail::float3_status(position); error != MeshBufferStatus::Ok)
            return {{}, error};
        for (typename linal::vec3<T>::size_type coordinate = 0; coordinate < 3; ++coordinate)
            result.values.push_back(static_cast<float>(position[coordinate]));
    }
    return result;
}

/**
 * \brief Three vertex indices per face in face-handle order, preserving face winding.
 *
 * Indices address \c make_vertex_buffer, so they stay correct while tombstoned elements await
 * \c garbage_collection().
 *
 * \return Index triples per face, or an empty result whose \c error names the failure.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<BufferIndex>
make_triangle_index_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<BufferIndex> result;
    if (!detail::mesh_buffer_count_fits(mesh.face_count(), 3, result.values.max_size()))
        return {{}, MeshBufferStatus::CapacityExceeded};
    const auto liveIndices = detail::make_live_vertex_indices(mesh);
    if (!liveIndices)
        return {{}, liveIndices.error};
    result.values.reserve(mesh.face_count() * 3);

    for (const auto face: mesh.faces())
        for (auto vertex = mesh.vertices(face).circulator(); vertex.is_valid(); ++vertex)
            result.values.push_back(liveIndices.values[static_cast<std::size_t>(vertex.get_vertexhandle().get_value())]);
    return result;
}

/**
 * \brief Two endpoint indices per edge in edge-handle order, including boundary edges and
 * triangulation diagonals.
 *
 * Each undirected edge is emitted exactly once. Indices address \c make_vertex_buffer, as for
 * \c make_triangle_index_buffer.
 *
 * \return Index pairs per edge, or an empty result whose \c error names the failure.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshBufferResult<BufferIndex> make_edge_index_buffer(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    MeshBufferResult<BufferIndex> result;
    if (!detail::mesh_buffer_count_fits(mesh.edge_count(), 2, result.values.max_size()))
        return {{}, MeshBufferStatus::CapacityExceeded};
    const auto liveIndices = detail::make_live_vertex_indices(mesh);
    if (!liveIndices)
        return {{}, liveIndices.error};
    result.values.reserve(mesh.edge_count() * 2);

    for (const auto edge: mesh.edges()) {
        const auto halfedge = mesh.get_edge(edge).halfedge;
        result.values.push_back(liveIndices.values[static_cast<std::size_t>(mesh.source_vertex(halfedge).get_value())]);
        result.values.push_back(liveIndices.values[static_cast<std::size_t>(mesh.target_vertex(halfedge).get_value())]);
    }
    return result;
}

/**
 * \brief Render buffers for a lit surface: parallel position and normal arrays plus
 * triangle and wireframe-segment indices.
 *
 * Renderers bind one normal per vertex, so a mesh vertex is emitted once per crease-bounded
 * smooth sector around it ("split" along creases) and the indices are remapped. With no crease
 * edges this is one render vertex per mesh vertex.
 */
struct MeshRenderBuffers {
    // 3 floats per render vertex.
    std::vector<float> positions;
    // 3 floats per render vertex, parallel to positions.
    std::vector<float> normals;
    // 3 indices per face.
    std::vector<BufferIndex> triangles;
    // 2 indices per crease edge (wireframe overlay).
    std::vector<BufferIndex> segments;
    MeshBufferStatus error = MeshBufferStatus::Ok;

    GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
    GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
    GEO_NODISCARD std::size_t vertex_count() const noexcept { return positions.size() / 3; }
};

/**
 * \brief Positions, per-vertex normals, and remapped triangle and wireframe-segment
 * indices for a lit surface.
 *
 * Emits one render vertex per smooth sector (see MeshRenderBuffers), sharing the sector
 * walk of \c compute_halfedge_normals so render vertices and normals agree. Render vertices
 * come out in vertex-handle order. Requires valid connectivity; isolated vertices are
 * emitted with a zero normal, tombstoned elements are skipped. O(H).
 *
 * \return Populated MeshRenderBuffers, or an empty result whose \c error names the failure
 * (e.g. \c DegenerateGeometry for a zero-area face with no usable normal).
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD MeshRenderBuffers make_render_buffers(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) {
    using HalfedgeHandle = typename TriangleHalfedgeMesh<T, 3, TIndex>::HalfedgeHandle;
    using VertexHandle = typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle;

    const auto normals = compute_halfedge_normals(mesh);
    if (!normals)
        return {{}, {}, {}, {}, MeshBufferStatus::DegenerateGeometry};

    MeshRenderBuffers result;
    MeshBufferStatus status = MeshBufferStatus::Ok;
    constexpr BufferIndex unassigned = std::numeric_limits<BufferIndex>::max();
    // Render id of each corner, indexed by halfedge value like the normals.
    std::vector<BufferIndex> cornerIds(mesh.halfedge_storage_size(), unassigned);
    // One render id per mesh vertex for wireframe segment endpoints: every copy of a vertex
    // shares its position, so any copy yields the same line.
    std::vector<BufferIndex> representative(mesh.vertex_storage_size(), unassigned);

    const auto emit_vertex = [&](VertexHandle vertex, const linal::vec3<T>& normal) -> MeshBufferStatus {
        if (!detail::mesh_buffer_index_fits<BufferIndex>(result.vertex_count()))
            return MeshBufferStatus::CapacityExceeded;
        const auto& position = mesh.get_vertex(vertex).position;
        if (const auto error = detail::float3_status(position); error != MeshBufferStatus::Ok)
            return error;
        if (const auto error = detail::float3_status(normal); error != MeshBufferStatus::Ok)
            return error;
        for (typename linal::vec3<T>::size_type coordinate = 0; coordinate < 3; ++coordinate) {
            result.positions.push_back(static_cast<float>(position[coordinate]));
            result.normals.push_back(static_cast<float>(normal[coordinate]));
        }
        auto& vertexRepresentative = representative[static_cast<std::size_t>(vertex.get_value())];
        if (vertexRepresentative == unassigned)
            vertexRepresentative = static_cast<BufferIndex>(result.vertex_count() - 1);
        return MeshBufferStatus::Ok;
    };

    for (const VertexHandle vertex: mesh.vertices()) {
        if (!mesh.get_vertex(vertex).halfedge.is_valid()) {
            status = emit_vertex(vertex, linal::vec3<T>{});
        } else {
            detail::for_each_smooth_sector(mesh, vertex, [&](HalfedgeHandle sectorBegin, HalfedgeHandle sectorEnd) {
                if (status != MeshBufferStatus::Ok)
                    return;
                // Every corner of the sector carries the same normal, so the first one speaks for all.
                const HalfedgeHandle firstCorner = mesh.get_halfedge(sectorBegin).prev;
                status = emit_vertex(vertex, normals.values[static_cast<std::size_t>(firstCorner.get_value())]);
                if (status != MeshBufferStatus::Ok)
                    return;
                const auto id = static_cast<BufferIndex>(result.vertex_count() - 1);
                detail::for_each_sector_corner(mesh, sectorBegin, sectorEnd, [&](HalfedgeHandle corner) {
                    cornerIds[static_cast<std::size_t>(corner.get_value())] = id;
                });
            });
        }
        if (status != MeshBufferStatus::Ok)
            return {{}, {}, {}, {}, status};
    }

    result.triangles.reserve(mesh.face_count() * 3);
    for (const auto face: mesh.faces())
        for (const HalfedgeHandle corner: mesh.halfedges_around_face(face)) {
            const BufferIndex id = cornerIds[static_cast<std::size_t>(corner.get_value())];
            GEO_ASSERT(id != unassigned);
            result.triangles.push_back(id);
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
