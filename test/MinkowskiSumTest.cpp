#include <Geometry/MinkowskiSum2D.hpp>
#include <Geometry/MinkowskiSum3D.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>
#include <linal/vec_compare.hpp>
#include <algorithm>
#include <span>
#include <stdexcept>
#include <vector>

using namespace Geometry;

namespace
{

// Thin wrappers so call sites never carry a `<...>` comma inside a gtest macro. The wrapped
// Geometry functions are overloaded on vec2<T>/vec3<T>, so a single argument type selects the
// 2d or 3d variant.

template <typename T, std::uint8_t D>
bool normalized(const std::vector<linal::vec<T, D>>& poly, T eps)
{
  return is_normalized_convex_polygon<T>(std::span<const linal::vec<T, D>>{poly}, eps);
}

template <typename T, std::uint8_t D>
bool convex(const std::vector<linal::vec<T, D>>& poly, T eps)
{
  return is_convex<T>(std::span<const linal::vec<T, D>>{poly}, eps);
}

template <typename T, std::uint8_t D>
bool ccw(const std::vector<linal::vec<T, D>>& poly, T eps)
{
  return is_counter_clockwise<T>(std::span<const linal::vec<T, D>>{poly}, eps);
}

template <typename T, std::uint8_t D>
bool no_repeated(const std::vector<linal::vec<T, D>>& poly, T eps)
{
  return has_no_repeated_vertices<T>(std::span<const linal::vec<T, D>>{poly}, eps);
}

template <typename T, std::uint8_t D>
bool no_collinear(const std::vector<linal::vec<T, D>>& poly, T eps)
{
  return has_no_collinear_vertices<T>(std::span<const linal::vec<T, D>>{poly}, eps);
}

template <typename T, std::uint8_t D>
bool lowest_leftmost_first(const std::vector<linal::vec<T, D>>& poly, T eps)
{
  return starts_at_lowest_leftmost<T>(std::span<const linal::vec<T, D>>{poly}, eps);
}

template <typename T, std::uint8_t D>
std::vector<linal::vec<T, D>> normalize(const std::vector<linal::vec<T, D>>& poly, T eps)
{
  return normalize_convex_polygon<T>(std::span<const linal::vec<T, D>>{poly}, eps);
}

template <typename T, std::uint8_t D>
std::vector<linal::vec<T, D>> negate(const std::vector<linal::vec<T, D>>& poly)
{
  return negate_polygon<T>(std::span<const linal::vec<T, D>>{poly});
}

template <typename T, std::uint8_t D>
std::vector<linal::vec<T, D>> sum(const std::vector<linal::vec<T, D>>& lhs, const std::vector<linal::vec<T, D>>& rhs, T eps)
{
  return minkowski_sum<T>(std::span<const linal::vec<T, D>>{lhs}, std::span<const linal::vec<T, D>>{rhs}, eps);
}

template <typename T, std::uint8_t D>
std::vector<linal::vec<T, D>> diff(const std::vector<linal::vec<T, D>>& lhs, const std::vector<linal::vec<T, D>>& rhs, T eps)
{
  return minkowski_diff<T>(std::span<const linal::vec<T, D>>{lhs}, std::span<const linal::vec<T, D>>{rhs}, eps);
}

// Checks that two polygons describe the same vertex set, independent of the starting index.
template <typename T, std::uint8_t D>
bool same_polygon(const std::vector<linal::vec<T, D>>& lhs, const std::vector<linal::vec<T, D>>& rhs, T eps)
{
  if (lhs.size() != rhs.size())
  {
    return false;
  }
  const std::size_t size = lhs.size();
  for (std::size_t offset = 0; offset < size; ++offset)
  {
    bool matches = true;
    for (std::size_t i = 0; i < size; ++i)
    {
      if (!linal::is_equal(lhs[i], rhs[(i + offset) % size], eps))
      {
        matches = false;
        break;
      }
    }
    if (matches)
    {
      return true;
    }
  }
  return false;
}

// Convex point-in-polygon test for the origin (polygon assumed CCW).
template <typename T>
bool contains_origin(const std::vector<linal::vec<T, 2>>& polygon, T eps)
{
  const std::size_t size = polygon.size();
  for (std::size_t i = 0; i < size; ++i)
  {
    const linal::vec<T, 2> edge = polygon[(i + 1) % size] - polygon[i];
    const linal::vec<T, 2> toOrigin = linal::vec<T, 2>{-polygon[i][0], -polygon[i][1]};
    const T turn = edge[0] * toOrigin[1] - edge[1] * toOrigin[0];
    if (turn < -eps)
    {
      return false;
    }
  }
  return true;
}

constexpr double kEps = linal::eps_v<double>;
constexpr float kEpsF = linal::eps_v<float>;

std::vector<linal::double2> unit_square()
{
  // Already normalized: CCW, no duplicates/collinear, starts at lowest-leftmost.
  return {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
}

} // namespace

// --------------------------------------------------------------------------------------------
// Predicates
// --------------------------------------------------------------------------------------------

TEST(MinkowskiPredicates, NormalizedSquareIsRecognized)
{
  const std::vector<linal::double2> square = unit_square();
  EXPECT_TRUE(normalized(square, kEps));
  EXPECT_TRUE(convex(square, kEps));
  EXPECT_TRUE(ccw(square, kEps));
  EXPECT_TRUE(no_repeated(square, kEps));
  EXPECT_TRUE(no_collinear(square, kEps));
  EXPECT_TRUE(lowest_leftmost_first(square, kEps));
}

TEST(MinkowskiPredicates, ClockwiseSquareIsNotCounterClockwise)
{
  const std::vector<linal::double2> cwise = {{0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}};
  EXPECT_FALSE(ccw(cwise, kEps));
  EXPECT_FALSE(normalized(cwise, kEps));
}

TEST(MinkowskiPredicates, RepeatedVerticesAreDetected)
{
  const std::vector<linal::double2> dup = {{0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
  EXPECT_FALSE(no_repeated(dup, kEps));
  EXPECT_FALSE(normalized(dup, kEps));
}

TEST(MinkowskiPredicates, CollinearVerticesAreDetected)
{
  const std::vector<linal::double2> collinear = {{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}};
  EXPECT_FALSE(no_collinear(collinear, kEps));
  EXPECT_FALSE(normalized(collinear, kEps));
}

TEST(MinkowskiPredicates, WrongStartVertexIsDetected)
{
  const std::vector<linal::double2> rotated = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0}};
  EXPECT_FALSE(lowest_leftmost_first(rotated, kEps));
  EXPECT_FALSE(normalized(rotated, kEps));
}

TEST(MinkowskiPredicates, NonConvexPolygonIsDetected)
{
  // An arrow-head (concave) polygon.
  const std::vector<linal::double2> concave = {{0.0, 0.0}, {2.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {0.0, 2.0}};
  EXPECT_FALSE(convex(concave, kEps));
}

// --------------------------------------------------------------------------------------------
// normalize_convex_polygon
// --------------------------------------------------------------------------------------------

TEST(MinkowskiNormalize, DedupsFixesWindingDropsCollinearAndRotates)
{
  // Clockwise, with a duplicate, a repeated closing vertex and a collinear midpoint.
  const std::vector<linal::double2> raw = {{1.0, 1.0}, {1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0},
                                           {0.5, 0.0}, {1.0, 0.0}, {1.0, 1.0}};
  const std::vector<linal::double2> result = normalize(raw, kEps);

  EXPECT_TRUE(normalized(result, kEps));
  EXPECT_TRUE(same_polygon(result, unit_square(), kEps));
}

TEST(MinkowskiNormalize, AlreadyNormalizedIsStable)
{
  const std::vector<linal::double2> square = unit_square();
  const std::vector<linal::double2> result = normalize(square, kEps);
  EXPECT_EQ(result.size(), square.size());
  EXPECT_TRUE(same_polygon(result, square, kEps));
}

TEST(MinkowskiNormalize, ThrowsOnTooFewVertices)
{
  const std::vector<linal::double2> line = {{0.0, 0.0}, {1.0, 0.0}};
  EXPECT_THROW(normalize(line, kEps), std::invalid_argument);
}

TEST(MinkowskiNormalize, ThrowsOnZeroArea)
{
  const std::vector<linal::double2> degenerate = {{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}};
  EXPECT_THROW(normalize(degenerate, kEps), std::invalid_argument);
}

TEST(MinkowskiNormalize, ThrowsOnNonConvex)
{
  const std::vector<linal::double2> concave = {{0.0, 0.0}, {2.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {0.0, 2.0}};
  EXPECT_THROW(normalize(concave, kEps), std::invalid_argument);
}

// --------------------------------------------------------------------------------------------
// negate_polygon
// --------------------------------------------------------------------------------------------

TEST(MinkowskiNegate, CopyAndInPlaceAgree)
{
  const std::vector<linal::double2> square = unit_square();
  const std::vector<linal::double2> copyNegated = negate(square);

  std::vector<linal::double2> inPlace = square;
  negate_polygon<double>(std::span<linal::double2>{inPlace});

  ASSERT_EQ(copyNegated.size(), inPlace.size());
  for (std::size_t i = 0; i < inPlace.size(); ++i)
  {
    const linal::double2 expected{-square[i][0], -square[i][1]};
    EXPECT_TRUE(linal::is_equal(copyNegated[i], inPlace[i], kEps));
    EXPECT_TRUE(linal::is_equal(copyNegated[i], expected, kEps));
  }
}

// --------------------------------------------------------------------------------------------
// minkowski_sum
// --------------------------------------------------------------------------------------------

TEST(MinkowskiSum2d, SquarePlusSquareIsLargerSquare)
{
  const std::vector<linal::double2> poly_a = unit_square();
  const std::vector<linal::double2> poly_b = unit_square();
  const std::vector<linal::double2> result = sum(poly_a, poly_b, kEps);

  const std::vector<linal::double2> expected = {{0.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}};
  EXPECT_TRUE(normalized(result, kEps));
  EXPECT_TRUE(same_polygon(result, expected, kEps));
}

TEST(MinkowskiSum2d, TrianglePlusSquare)
{
  const std::vector<linal::double2> rawTriangle = {{0.0, 0.0}, {2.0, 0.0}, {0.0, 2.0}};
  const std::vector<linal::double2> triangle = normalize(rawTriangle, kEps);
  const std::vector<linal::double2> square = unit_square();
  const std::vector<linal::double2> result = sum(triangle, square, kEps);

  EXPECT_TRUE(normalized(result, kEps));
  // Sanity-check the extent of the resulting convex polygon.
  double minX = result[0][0];
  double maxX = result[0][0];
  double minY = result[0][1];
  double maxY = result[0][1];
  for (const auto& point: result)
  {
    minX = std::min(minX, point[0]);
    maxX = std::max(maxX, point[0]);
    minY = std::min(minY, point[1]);
    maxY = std::max(maxY, point[1]);
  }
  EXPECT_DOUBLE_EQ(minX, 0.0);
  EXPECT_DOUBLE_EQ(minY, 0.0);
  EXPECT_DOUBLE_EQ(maxX, 3.0);
  EXPECT_DOUBLE_EQ(maxY, 3.0);
}

TEST(MinkowskiSum2f, FloatSquarePlusSquare)
{
  const std::vector<linal::float2> sqr = {{0.0F, 0.0F}, {1.0F, 0.0F}, {1.0F, 1.0F}, {0.0F, 1.0F}};
  const std::vector<linal::float2> result = sum(sqr, sqr, kEpsF);

  const std::vector<linal::float2> expected = {{0.0F, 0.0F}, {2.0F, 0.0F}, {2.0F, 2.0F}, {0.0F, 2.0F}};
  EXPECT_TRUE(normalized(result, kEpsF));
  EXPECT_TRUE(same_polygon(result, expected, kEpsF));
}

// --------------------------------------------------------------------------------------------
// minkowski_diff
// --------------------------------------------------------------------------------------------

TEST(MinkowskiDiff2d, OverlappingSquaresContainOrigin)
{
  // Two overlapping unit squares: the Minkowski difference contains the origin iff they overlap.
  const std::vector<linal::double2> poly_a = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
  const std::vector<linal::double2> poly_b = {{0.5, 0.5}, {1.5, 0.5}, {1.5, 1.5}, {0.5, 1.5}};
  const std::vector<linal::double2> result = diff(poly_a, poly_b, kEps);

  EXPECT_TRUE(normalized(result, kEps));
  EXPECT_TRUE(contains_origin(result, kEps));
}

TEST(MinkowskiDiff2d, DisjointSquaresExcludeOrigin)
{
  const std::vector<linal::double2> poly_a = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
  const std::vector<linal::double2> poly_b = {{5.0, 5.0}, {6.0, 5.0}, {6.0, 6.0}, {5.0, 6.0}};
  const std::vector<linal::double2> result = diff(poly_a, poly_b, kEps);

  EXPECT_FALSE(contains_origin(result, kEps));
}

// --------------------------------------------------------------------------------------------
// 3d coplanar polygons
// --------------------------------------------------------------------------------------------

TEST(MinkowskiSum3d, SquaresInZPlane)
{
  // Two unit squares in the plane z = 2.
  const std::vector<linal::double3> poly_a = {{0.0, 0.0, 2.0}, {1.0, 0.0, 2.0}, {1.0, 1.0, 2.0}, {0.0, 1.0, 2.0}};
  const auto& poly_b = poly_a;
  const std::vector<linal::double3> result = sum(poly_a, poly_b, kEps);

  EXPECT_TRUE(normalized(result, kEps));
  ASSERT_EQ(result.size(), 4U);
  // Every result vertex must stay in the plane z = 2.
  for (const auto& point: result)
  {
    EXPECT_NEAR(point[2], 2.0, 1e-9);
  }
}

TEST(MinkowskiSum3d, SquaresInTiltedPlaneStayCoplanar)
{
  // A unit square living in a plane tilted out of the xy-plane.
  const double side = 1.0;
  const std::vector<linal::double3> poly_a = {{0.0, 0.0, 0.0}, {side, 0.0, 0.0}, {side, side, side}, {0.0, side, side}};
  const auto& poly_b = poly_a;

  const std::vector<linal::double3> result = sum(poly_a, poly_b, kEps);
  EXPECT_TRUE(normalized(result, kEps));
  ASSERT_EQ(result.size(), 4U);

  // All result vertices must be coplanar with poly_a: the plane through poly_a[0] with normal
  // n = (poly_a[1]-poly_a[0]) x (poly_a[3]-poly_a[0]).
  const linal::double3 normal = linal::cross(linal::double3{poly_a[1] - poly_a[0]}, linal::double3{poly_a[3] - poly_a[0]});
  for (const auto& point: result)
  {
    const double planeEq = linal::dot(linal::double3{point - poly_a[0]}, normal);
    EXPECT_NEAR(planeEq, 0.0, 1e-9);
  }
}
