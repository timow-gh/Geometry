#ifndef GEOMETRY_MESH_MAKETRIANGLEMESH_HPP
#define GEOMETRY_MESH_MAKETRIANGLEMESH_HPP

#include "Geometry/AABB.hpp"
#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Mesh/AddTriangle.hpp"
#include "Geometry/Mesh/MeshNormals.hpp"
#include "Geometry/Mesh/MeshResult.hpp"

#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>

namespace Geometry {

enum class MeshCreationStatus {
    Ok,
    InvalidSegmentCount,
    NonFiniteGeometry,
    DegenerateGeometry,
    InvalidBounds,
    IndexCapacityExceeded,
    TriangleInsertionFailed
};

/**
 * \brief Mesh produced by a \c make_triangle_mesh factory, or the reason it could not be built.
 *
 * A reported failure always carries an empty mesh, never a partial one. Allocation failures are not
 * reported here: they propagate from the underlying containers, so the factories are not noexcept.
 */
template <typename T, typename TIndex = std::uint32_t>
struct MeshCreationResult {
    TriangleHalfedgeMesh<T, 3, TIndex> mesh;
    MeshCreationStatus error = MeshCreationStatus::Ok;

    GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
    GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

/**
 * \brief Default crease angle (radians) applied by the factories.
 *
 * Chosen between the small dihedral of a tessellated wall (which stays smooth) and the sharp
 * cap/apex/box transitions (which become creases). Callers can re-tag with \c mark_creases_by_angle
 * or \c set_crease to override.
 */
template <typename T>
inline constexpr T default_crease_angle = static_cast<T>(0.5); // ~28.6 degrees

namespace detail {
template <typename T>
bool mesh_position_is_finite(const linal::vec3<T>& position) {
    return std::isfinite(position[0]) && std::isfinite(position[1]) && std::isfinite(position[2]);
}

template <typename T>
T mesh_vector_length(const linal::vec3<T>& vector) {
    return std::hypot(vector[0], vector[1], vector[2]);
}

// Closed triangular surfaces have three halfedges per face. Check the largest
// element count by division before computing counts or allocating storage.
template <typename TIndex>
bool mesh_counts_fit(std::size_t segments, std::size_t halfedgesPerSegment) {
    const auto limit =
        std::min<std::uintmax_t>(std::numeric_limits<TIndex>::max(), std::numeric_limits<std::size_t>::max());
    return segments <= limit / halfedgesPerSegment;
}

template <typename T, typename TIndex>
MeshCreationStatus add_mesh_creation_triangle(TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                             typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle first,
                                             typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle second,
                                             typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle third) {
    const auto& origin = mesh.get_vertex(first).position;
    const linal::vec3<T> edge1{mesh.get_vertex(second).position - origin};
    const linal::vec3<T> edge2{mesh.get_vertex(third).position - origin};
    // Callers validate all vertex positions finite before inserting, so the edge lengths here are
    // finite too; only degeneracy (a collapsed edge or face) can still arise and must be caught.
    const T length1 = mesh_vector_length(edge1);
    const T length2 = mesh_vector_length(edge2);
    if (length1 == T{0} || length2 == T{0})
        return MeshCreationStatus::DegenerateGeometry;
    // Scaling avoids squaring tiny lengths and detects positions that collapsed
    // through rounding, even when the input dimensions were nonzero.
    const auto normal = linal::cross(linal::vec3<T>{edge1 / length1}, linal::vec3<T>{edge2 / length2});
    if (mesh_vector_length(normal) == T{0})
        return MeshCreationStatus::DegenerateGeometry;
    return add_triangle(mesh, first, second, third).is_valid() ? MeshCreationStatus::Ok
                                                               : MeshCreationStatus::TriangleInsertionFailed;
}

template <bool IsCylinder, std::floating_point T, typename TIndex>
MeshCreationResult<T, TIndex> make_round_triangle_mesh(const Segment3<T>& segment, T radius, std::size_t segments) {
    using Result = MeshCreationResult<T, TIndex>;
    using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
    using VertexHandle = typename Mesh::VertexHandle;
    const auto failure = [](MeshCreationStatus error) { return Result{{}, error}; };
    if (segments < 3)
        return failure(MeshCreationStatus::InvalidSegmentCount);
    const auto source = segment.get_source();
    const auto target = segment.get_target();
    if (!mesh_position_is_finite(source) || !mesh_position_is_finite(target) || !std::isfinite(radius))
        return failure(MeshCreationStatus::NonFiniteGeometry);
    const linal::vec3<T> delta{target - source};
    const T height = mesh_vector_length(delta);
    if (!std::isfinite(height))
        return failure(MeshCreationStatus::NonFiniteGeometry);
    if (radius <= T{0} || height == T{0})
        return failure(MeshCreationStatus::DegenerateGeometry);
    if (!mesh_counts_fit<TIndex>(segments, IsCylinder ? 12 : 6))
        return failure(MeshCreationStatus::IndexCapacityExceeded);

    const linal::vec3<T> axis{delta / height};
    typename linal::vec3<T>::size_type least = 0;
    for (typename linal::vec3<T>::size_type i = 1; i < 3; ++i)
        if (std::abs(axis[i]) < std::abs(axis[least]))
            least = i;
    linal::vec3<T> reference{};
    reference[least] = T{1};
    const auto cross = linal::cross(axis, reference);
    const linal::vec3<T> first{cross / mesh_vector_length(cross)};
    const auto second = linal::cross(axis, first);

    const auto position = [&](std::size_t i, bool upper) {
        const T angle = T{2} * std::numbers::pi_v<T> * (static_cast<T>(i) / static_cast<T>(segments));
        return linal::vec3<T>{(upper ? target : source) +
                              radius * (std::cos(angle) * first + std::sin(angle) * second)};
    };

    // Sample each ring position once, validating finiteness while filling, and reuse the stored
    // positions for insertion. Computing them twice would double the per-vertex sin/cos work.
    std::vector<linal::vec3<T>> lowerPositions, upperPositions;
    lowerPositions.reserve(segments);
    if constexpr (IsCylinder)
        upperPositions.reserve(segments);
    for (std::size_t i = 0; i < segments; ++i) {
        lowerPositions.push_back(position(i, false));
        if (!mesh_position_is_finite(lowerPositions.back()))
            return failure(MeshCreationStatus::NonFiniteGeometry);
        if constexpr (IsCylinder) {
            upperPositions.push_back(position(i, true));
            if (!mesh_position_is_finite(upperPositions.back()))
                return failure(MeshCreationStatus::NonFiniteGeometry);
        }
    }

    Mesh mesh;
    std::vector<VertexHandle> lower, upper;
    lower.reserve(segments);
    if constexpr (IsCylinder)
        upper.reserve(segments);
    for (std::size_t i = 0; i < segments; ++i)
        lower.push_back(mesh.add_vertex(lowerPositions[i]));
    if constexpr (IsCylinder)
        for (std::size_t i = 0; i < segments; ++i)
            upper.push_back(mesh.add_vertex(upperPositions[i]));
    const auto bottom = mesh.add_vertex(source);
    const auto top = mesh.add_vertex(target);
    MeshCreationStatus error = MeshCreationStatus::Ok;
    const auto triangle = [&](VertexHandle v0, VertexHandle v1, VertexHandle v2) {
        error = add_mesh_creation_triangle(mesh, v0, v1, v2);
        return error == MeshCreationStatus::Ok;
    };
    for (std::size_t i = 0; i < segments; ++i)
        if (!triangle(bottom, lower[(i + 1) % segments], lower[i]))
            return failure(error);
    for (std::size_t i = 0; i < segments; ++i) {
        const std::size_t next = (i + 1) % segments;
        if constexpr (IsCylinder) {
            if (!triangle(lower[i], lower[next], upper[next]) || !triangle(lower[i], upper[next], upper[i]))
                return failure(error);
        } else if (!triangle(lower[i], lower[next], top))
            return failure(error);
    }
    if constexpr (IsCylinder)
        for (std::size_t i = 0; i < segments; ++i)
            if (!triangle(top, upper[i], upper[(i + 1) % segments]))
                return failure(error);
    mark_creases_by_angle(mesh, default_crease_angle<T>);
    return {std::move(mesh), MeshCreationStatus::Ok};
}
} // namespace detail

template <std::floating_point T, typename TIndex = std::uint32_t>
GEO_NODISCARD MeshCreationResult<T, TIndex> make_triangle_mesh(const Cone<T>& shape, std::size_t segments = 32) {
    return detail::make_round_triangle_mesh<false, T, TIndex>(shape.get_segment(), shape.get_radius(), segments);
}

template <std::floating_point T, typename TIndex = std::uint32_t>
GEO_NODISCARD MeshCreationResult<T, TIndex> make_triangle_mesh(const Cylinder<T>& shape, std::size_t segments = 32) {
    return detail::make_round_triangle_mesh<true, T, TIndex>(shape.get_segment(), shape.get_radius(), segments);
}

template <std::floating_point T, typename TIndex = std::uint32_t>
GEO_NODISCARD MeshCreationResult<T, TIndex> make_triangle_mesh(const Cuboid<T>& shape) {
    using Result = MeshCreationResult<T, TIndex>;
    using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
    const auto failure = [](MeshCreationStatus error) { return Result{{}, error}; };
    if (!detail::mesh_position_is_finite(shape.get_origin()))
        return failure(MeshCreationStatus::NonFiniteGeometry);
    std::array<linal::vec3<T>, 3> directions;
    const auto& sides = shape.get_side_vectors();
    for (const auto& side: sides)
        if (!detail::mesh_position_is_finite(side))
            return failure(MeshCreationStatus::NonFiniteGeometry);
    for (std::size_t i = 0; i < 3; ++i) {
        const T length = detail::mesh_vector_length(sides[i]);
        if (!std::isfinite(length))
            return failure(MeshCreationStatus::NonFiniteGeometry);
        if (length == T{0})
            return failure(MeshCreationStatus::DegenerateGeometry);
        directions[i] = sides[i] / length;
    }
    const T determinant = linal::dot(directions[0], linal::cross(directions[1], directions[2]));
    if (determinant == T{0})
        return failure(MeshCreationStatus::DegenerateGeometry);
    if (!detail::mesh_counts_fit<TIndex>(1, 36))
        return failure(MeshCreationStatus::IndexCapacityExceeded);
    const auto positions = calc_cuboid_vertices(shape);
    for (const auto& position: positions)
        if (!detail::mesh_position_is_finite(position))
            return failure(MeshCreationStatus::NonFiniteGeometry);
    Mesh mesh;
    std::array<typename Mesh::VertexHandle, 8> vertices;
    for (std::size_t i = 0; i < 8; ++i)
        vertices[i] = mesh.add_vertex(positions[i]);
    constexpr std::array<std::array<std::size_t, 3>, 12> triangles{{{0, 2, 1},
                                                                    {0, 3, 2},
                                                                    {0, 1, 5},
                                                                    {0, 5, 4},
                                                                    {1, 2, 6},
                                                                    {1, 6, 5},
                                                                    {2, 3, 7},
                                                                    {2, 7, 6},
                                                                    {3, 0, 4},
                                                                    {3, 4, 7},
                                                                    {4, 5, 6},
                                                                    {4, 6, 7}}};
    for (auto triangle: triangles) {
        if (determinant < T{0})
            std::swap(triangle[1], triangle[2]);
        const auto error = detail::add_mesh_creation_triangle(mesh,
                                                              vertices[triangle[0]],
                                                              vertices[triangle[1]],
                                                              vertices[triangle[2]]);
        if (error != MeshCreationStatus::Ok)
            return failure(error);
    }
    mark_creases_by_angle(mesh, default_crease_angle<T>);
    return {std::move(mesh), MeshCreationStatus::Ok};
}

template <std::floating_point T, typename TIndex = std::uint32_t>
GEO_NODISCARD MeshCreationResult<T, TIndex> make_triangle_mesh(const AABB<T, 3>& shape) {
    using Result = MeshCreationResult<T, TIndex>;
    const auto failure = [](MeshCreationStatus error) { return Result{{}, error}; };
    const auto minimum = shape.get_min();
    const auto maximum = shape.get_max();
    if (!detail::mesh_position_is_finite(minimum) || !detail::mesh_position_is_finite(maximum))
        return failure(MeshCreationStatus::NonFiniteGeometry);
    for (typename linal::vec3<T>::size_type i = 0; i < 3; ++i)
        if (minimum[i] > maximum[i])
            return failure(MeshCreationStatus::InvalidBounds);
    for (typename linal::vec3<T>::size_type i = 0; i < 3; ++i)
        if (minimum[i] == maximum[i])
            return failure(MeshCreationStatus::DegenerateGeometry);
    return make_triangle_mesh<T, TIndex>(Cuboid<T>{minimum, linal::vec3<T>{maximum - minimum}});
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MAKETRIANGLEMESH_HPP
