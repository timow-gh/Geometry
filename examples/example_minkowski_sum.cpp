#include "draw.hpp"
#include "geoqik_init.hpp"
#include "grid.hpp"
#include "origin.hpp"

#include <linal/vec.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <span>
#include <stdexcept>
#include <vector>

void draw_as_segments(std::span<linal::float2> polygon, const example::color& color) {
    size_t size = polygon.size();
    for (size_t i = 0; i < size; ++i) {
        const auto& p0 = polygon[i];
        const auto& p1 = polygon[(i + 1) % size];
        example::draw(Geometry::Segment2d{p0, p1}, color);
    }
}

[[nodiscard]] static bool is_convex(std::span<linal::float2> polygon, float epsilon = 1e-6f) {
    size_t size = polygon.size();
    if (size < 3)
        return false; // A polygon must have at least 3 vertices

    int orientation = 0;
    for (size_t i = 0; i < size; ++i) {
        const auto& p0 = polygon[i];
        const auto& p1 = polygon[(i + 1) % size];
        const auto& p2 = polygon[(i + 2) % size];

        float cross_product = (p1[0] - p0[0]) * (p2[1] - p1[1]) - (p1[1] - p0[1]) * (p2[0] - p1[0]);
        if (std::abs(cross_product) < epsilon) {
            continue; // Collinear points, skip
        }

        const int current_orientation = cross_product > 0.0f ? 1 : -1;
        if (orientation == 0) {
            orientation = current_orientation;
        } else if (current_orientation != orientation) {
            return false; // Found a change in direction, not convex
        }
    }
    return orientation != 0; // All cross products had the same sign and the polygon has non-zero area
}

std::vector<linal::float2> convex_hull(const std::span<linal::float2>& shape1) {
    std::vector<linal::float2> hull;

    // Find the leftmost point
    auto leftmost = std::min_element(shape1.begin(), shape1.end(), [](const linal::float2& a, const linal::float2& b) {
        return a[0] < b[0];
    });

    linal::float2 current = *leftmost;
    do {
        hull.push_back(current);
        linal::float2 next = shape1[0];

        for (const auto& point: shape1) {
            if (point == current)
                continue;

            float cross_product =
                (next[0] - current[0]) * (point[1] - current[1]) - (next[1] - current[1]) * (point[0] - current[0]);
            if (cross_product < 0 || (cross_product == 0 && linal::dot(point - current, point - current) >
                                                                linal::dot(next - current, next - current))) {
                next = point;
            }
        }

        current = next;
    } while (current != *leftmost);

    return hull;
}

[[nodiscard]] static bool
is_collinear(const linal::float2& p0, const linal::float2& p1, const linal::float2& p2, float epsilon = 1e-6f) {
    float cross_product = (p1[0] - p0[0]) * (p2[1] - p1[1]) - (p1[1] - p0[1]) * (p2[0] - p1[0]);
    return std::abs(cross_product) < epsilon;
}

[[nodiscard]] static float cross(const linal::float2& lhs, const linal::float2& rhs) {
    return lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

/**
 * Converts a convex polygon to the canonical representation required by the
 * linear-time Minkowski-sum edge merge. The represented geometric region does
 * not change; only redundant vertices, winding, and the starting vertex do.
 *
 * Normalization performs these steps:
 * 1. Remove consecutive duplicate vertices and an optional repeated closing
 *    vertex (for example, `{a, b, c, a}` becomes `{a, b, c}`).
 * 2. Require at least three distinct vertices and a non-zero signed area.
 * 3. Reverse clockwise input so that all polygons use counterclockwise winding.
 * 4. Verify that the polygon is convex, allowing collinear boundary vertices.
 * 5. Remove collinear boundary vertices because they create redundant edges.
 * 6. Rotate the vertex sequence so that the lowest, then leftmost, vertex is
 *    first. For a counterclockwise convex polygon, this makes the outgoing edge
 *    directions cyclically sorted for the Minkowski merge.
 *
 * The input span is never modified. `epsilon` is used for area, convexity, and
 * collinearity tests.
 *
 * @throws std::invalid_argument if the input is degenerate or non-convex.
 */
[[nodiscard]] static std::vector<linal::float2>
normalize_convex_polygon(std::span<const linal::float2> polygon, float epsilon = 1e-6f) {
    std::vector<linal::float2> normalized;
    normalized.reserve(polygon.size());

    for (const auto& point: polygon) {
        if (normalized.empty() || point != normalized.back()) {
            normalized.push_back(point);
        }
    }
    if (normalized.size() > 1 && normalized.front() == normalized.back()) {
        normalized.pop_back();
    }

    if (normalized.size() < 3) {
        throw std::invalid_argument("A Minkowski-sum polygon must have at least three distinct vertices");
    }

    double signed_area_twice = 0.0;
    for (size_t i = 0; i < normalized.size(); ++i) {
        const auto& point = normalized[i];
        const auto& next = normalized[(i + 1) % normalized.size()];
        signed_area_twice +=
            static_cast<double>(point[0]) * static_cast<double>(next[1]) -
            static_cast<double>(point[1]) * static_cast<double>(next[0]);
    }

    if (std::abs(signed_area_twice) <= static_cast<double>(epsilon)) {
        throw std::invalid_argument("A Minkowski-sum polygon must have non-zero area");
    }
    if (signed_area_twice < 0.0) {
        std::reverse(normalized.begin(), normalized.end());
    }

    for (size_t i = 0; i < normalized.size(); ++i) {
        const auto edge1 = normalized[(i + 1) % normalized.size()] - normalized[i];
        const auto edge2 = normalized[(i + 2) % normalized.size()] - normalized[(i + 1) % normalized.size()];
        if (cross(edge1, edge2) < -epsilon) {
            throw std::invalid_argument("Minkowski sum requires convex polygons");
        }
    }

    std::vector<linal::float2> without_collinear_vertices;
    without_collinear_vertices.reserve(normalized.size());
    for (size_t i = 0; i < normalized.size(); ++i) {
        const auto& previous = normalized[(i + normalized.size() - 1) % normalized.size()];
        const auto& current = normalized[i];
        const auto& next = normalized[(i + 1) % normalized.size()];
        if (!is_collinear(previous, current, next, epsilon)) {
            without_collinear_vertices.push_back(current);
        }
    }

    if (without_collinear_vertices.size() < 3) {
        throw std::invalid_argument("A Minkowski-sum polygon must have non-zero area");
    }

    const auto lowest_leftmost =
        std::min_element(without_collinear_vertices.begin(), without_collinear_vertices.end(),
                         [](const linal::float2& lhs, const linal::float2& rhs) {
                             return lhs[1] < rhs[1] || (lhs[1] == rhs[1] && lhs[0] < rhs[0]);
                         });
    std::rotate(without_collinear_vertices.begin(), lowest_leftmost, without_collinear_vertices.end());
    return without_collinear_vertices;
}

// function minkowskiSum(A, B):
//     removeConsecutiveDuplicateVertices(A)
//     removeConsecutiveDuplicateVertices(B)

//     ensureCounterclockwise(A)
//     ensureCounterclockwise(B)

//     rotateToLowestLeftmostVertex(A)
//     rotateToLowestLeftmostVertex(B)

//     edgesA = edgeVectors(A)
//     edgesB = edgeVectors(B)

//     i = 0
//     j = 0

//     current = A[0] + B[0]
//     result = [current]

//     while i < edgesA.size or j < edgesB.size:
//         if i == edgesA.size:
//             edge = edgesB[j]
//             j += 1

//         else if j == edgesB.size:
//             edge = edgesA[i]
//             i += 1

//         else:
//             turn = cross(edgesA[i], edgesB[j])

//             if turn > 0:
//                 edge = edgesA[i]
//                 i += 1

//             else if turn < 0:
//                 edge = edgesB[j]
//                 j += 1

//             else:
//                 // Parallel edges with the same angular position
//                 edge = edgesA[i] + edgesB[j]
//                 i += 1
//                 j += 1

//         current = current + edge
//         result.push_back(current)

//     // The last point normally equals the first point.
//     result.pop_back()

//     removeCollinearVertices(result)

//     return result

std::vector<linal::float2> minkowski_sum(const std::vector<linal::float2>& shape1,
                                         const std::vector<linal::float2>& shape2,
                                         float epsilon = 1e-6f) {
    if (shape1.empty() || shape2.empty()) {
        return {};
    }

    const auto polygon1 = normalize_convex_polygon(shape1, epsilon);
    const auto polygon2 = normalize_convex_polygon(shape2, epsilon);

    std::vector<linal::float2> result;
    result.reserve(polygon1.size() + polygon2.size());

    size_t i = 0;
    size_t j = 0;

    linal::float2 current = polygon1[0] + polygon2[0];
    result.push_back(current);

    while (i < polygon1.size() || j < polygon2.size()) {
        linal::float2 edge;

        if (i == polygon1.size()) {
            edge = polygon2[(j + 1) % polygon2.size()] - polygon2[j];
            j++;
        } else if (j == polygon2.size()) {
            edge = polygon1[(i + 1) % polygon1.size()] - polygon1[i];
            i++;
        } else {
            const linal::float2 edgeA = polygon1[(i + 1) % polygon1.size()] - polygon1[i];
            const linal::float2 edgeB = polygon2[(j + 1) % polygon2.size()] - polygon2[j];

            const float turn = cross(edgeA, edgeB);

            if (turn > epsilon) {
                edge = edgeA;
                i++;
            } else if (turn < -epsilon) {
                edge = edgeB;
                j++;
            } else {
                // Parallel edges with the same angular position
                edge = edgeA + edgeB;
                i++;
                j++;
            }
        }

        current = current + edge;
        result.push_back(current);
        draw(Geometry::Segment2d{result[result.size() - 2], result[result.size() - 1]}, example::red());
    }

    // The last point normally equals the first point.
    result.pop_back();

    // Remove collinear vertices
    std::vector<linal::float2> final_result;
    final_result.reserve(result.size());
    for (size_t k = 0; k < result.size(); ++k) {
        const auto& previous = result[(k + result.size() - 1) % result.size()];
        const auto& current_point = result[k];
        const auto& next = result[(k + 1) % result.size()];

        if (!is_collinear(previous, current_point, next, epsilon)) {
            final_result.push_back(current_point);
        }
    }

    return final_result;
}

std::vector<linal::float2> minkowski_difference(
    const std::vector<linal::float2>& shape1,
    const std::vector<linal::float2>& shape2,
    float epsilon = 1e-6f)
{
    if (shape1.empty() || shape2.empty()) {
        return {};
    }

    std::vector<linal::float2> negated_shape2;
    negated_shape2.reserve(shape2.size());

    for (const auto& point : shape2) {
        negated_shape2.push_back(-point);
        // Or, if unary minus is unavailable:
        // negated_shape2.push_back({-point.x, -point.y});
    }
    draw_as_segments(negated_shape2, example::orange());

    return minkowski_sum(shape1, negated_shape2, epsilon);
}

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    geoqik_draw();

    // Example usage of the Minkowski sum
    std::vector<linal::float2> shape1 = {{0.0f, 0.0f}, {2.0f, 0.0f}, {2.0f, 3.0f}, {0.5f, 3.5f}, {0.0f, 3.0f}};

    std::vector<linal::float2> shape2 = {{0.5f, 0.5f}, {1.5f, 0.5f}, {1.5f, 1.5f}, {0.5f, 1.5f}};

    std::transform(shape1.begin(), shape1.end(), shape1.begin(), [](const linal::float2& point) {
        return point + linal::float2{0.8f, 1.2f};
    });
    std::transform(shape2.begin(), shape2.end(), shape2.begin(), [](const linal::float2& point) {
        return point + linal::float2{1.0f, 1.0f};
    });
    assert(is_convex(shape1) && "Shape 1 is not convex");
    assert(is_convex(shape2) && "Shape 2 is not convex");

    draw_as_segments(shape1, example::blue());
    draw_as_segments(shape2, example::green());

    // std::vector<linal::float2> minkowskiSum = minkowski_sum(shape1, shape2);
    std::vector<linal::float2> minkowskiDiff = minkowski_difference(shape1, shape2);

    geoqik_replay_options_t replayOptions{};
    replayOptions.entriesPerSecond = 60.0;
    replayOptions.speedMultiplier = 1.0;
    replayOptions.maxEntriesPerFrame = 1024;
    replayOptions.startPaused = 0;
    replayOptions.entriesPerStep = 1;
    replayOptions.stepKeys = NULL; /* null or empty = default: right arrow and D */
    replayOptions.stepKeyCount = 1;
    replayOptions.backwardStepKeys = NULL; /* null or empty = default: left arrow and A */
    replayOptions.backwardStepKeyCount = 1;
    replayOptions.resumeKeys = NULL; /* null or empty = default: space */
    replayOptions.resumeKeyCount = 1;
    replayOptions.pauseKeys = NULL; /* null or empty = default: space */
    replayOptions.pauseKeyCount = 1;
    replayOptions.increaseEntriesPerStepKeys = NULL; /* null or empty = default: up arrow and W */
    replayOptions.increaseEntriesPerStepKeyCount = 1;
    replayOptions.decreaseEntriesPerStepKeys = NULL; /* null or empty = default: down arrow and S */
    replayOptions.decreaseEntriesPerStepKeyCount = 1;

    geoqik_replay_current_log(&replayOptions);

    geoqik_wait_for_exit_and_cleanup();

    return 0;
}
