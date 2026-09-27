#include <Geometry/PolygonTriangulation.hpp>
#include <Geometry/PolygonTriangulationCosts.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <linal/vec.hpp>
#include <numbers>
#include <span>
#include <utility>
#include <vector>

using namespace Geometry;

namespace
{

using Vec2 = linal::vec2<double>;
using Vec3 = linal::vec3<double>;

// Checks the index structure only: n - 2 triangles, sorted corners, every polygon side used once and
// every diagonal shared by exactly two triangles.
void expect_valid_triangulation(std::size_t cornerCount, std::span<const PolygonTriangle> triangles)
{
  ASSERT_EQ(triangles.size(), triangulation_triangle_count(cornerCount));
  std::vector<std::size_t> edgeUses(cornerCount * cornerCount, 0);
  for (const auto& [first, apex, last] : triangles)
  {
    ASSERT_LT(first, apex);
    ASSERT_LT(apex, last);
    ASSERT_LT(last, cornerCount);
    ++edgeUses[first * cornerCount + apex];
    ++edgeUses[apex * cornerCount + last];
    ++edgeUses[first * cornerCount + last];
  }
  for (std::size_t first = 0; first < cornerCount; ++first)
  {
    for (std::size_t last = first + 1; last < cornerCount; ++last)
    {
      const std::size_t uses = edgeUses[first * cornerCount + last];
      const bool isSide = last == first + 1 || (first == 0 && last == cornerCount - 1);
      if (isSide)
      {
        EXPECT_EQ(uses, 1U) << "side " << first << "-" << last;
      }
      else
      {
        EXPECT_TRUE(uses == 0 || uses == 2) << "diagonal " << first << "-" << last;
      }
    }
  }
}

double signed_doubled_area(const Vec2& first, const Vec2& apex, const Vec2& last)
{
  return (apex[0] - first[0]) * (last[1] - first[1]) - (apex[1] - first[1]) * (last[0] - first[0]);
}

double polygon_doubled_area(std::span<const Vec2> corners)
{
  double area = 0.0;
  for (std::size_t i = 2; i < corners.size(); ++i)
  {
    area += signed_doubled_area(corners[0], corners[i - 1], corners[i]);
  }
  return area;
}

bool contains_diagonal(std::span<const PolygonTriangle> triangles, std::size_t first, std::size_t last)
{
  return std::any_of(triangles.begin(), triangles.end(), [&](const PolygonTriangle& triangle) {
    return std::find(triangle.begin(), triangle.end(), first) != triangle.end()
           && std::find(triangle.begin(), triangle.end(), last) != triangle.end();
  });
}

std::vector<Vec2> regular_polygon(std::size_t cornerCount)
{
  std::vector<Vec2> corners;
  corners.reserve(cornerCount);
  for (std::size_t i = 0; i < cornerCount; ++i)
  {
    const double angle = 2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(cornerCount);
    corners.push_back(Vec2{std::cos(angle), std::sin(angle)});
  }
  return corners;
}

// A quad whose two triangulations are priced by hand: diagonal (0, 2) gives the triangles
// (0, 1, 2) and (0, 2, 3), diagonal (1, 3) gives (0, 1, 3) and (1, 2, 3).
template <typename TCost>
struct QuadCosts
{
  TCost zeroOneTwo;
  TCost zeroTwoThree;
  TCost zeroOneThree;
  TCost oneTwoThree;

  TCost operator()(std::size_t first, std::size_t apex, std::size_t last) const
  {
    if (first == 0 && apex == 1 && last == 2)
    {
      return zeroOneTwo;
    }
    if (first == 0 && apex == 2 && last == 3)
    {
      return zeroTwoThree;
    }
    if (first == 0 && apex == 1 && last == 3)
    {
      return zeroOneThree;
    }
    return oneTwoThree;
  }
};

constexpr bool triangulates_hexagon_at_compile_time()
{
  constexpr std::size_t cornerCount = 6;
  std::array<TriangulationCell<double>, triangulation_scratch_size(cornerCount)> scratch{};
  std::array<PolygonTriangle, triangulation_triangle_count(cornerCount)> triangles{};
  const auto uniform = [](std::size_t, std::size_t, std::size_t) noexcept { return 1.0; };
  return minimum_cost_triangulation(cornerCount, uniform, scratch, triangles) && triangles[0][0] == 0
         && triangles[0][2] == cornerCount - 1;
}

static_assert(triangulates_hexagon_at_compile_time());

} // namespace

TEST(PolygonTriangulation, RespectsForbiddenDiagonals)
{
  std::array<TriangulationCell<double>, triangulation_scratch_size(7)> scratch{};
  std::array<PolygonTriangle, triangulation_triangle_count(7)> triangles{};
  const auto uniform = [](std::size_t, std::size_t, std::size_t) { return 1.0; };

  const auto forbidZeroTwo = [](std::size_t first, std::size_t last) { return !(first == 0 && last == 2); };
  ASSERT_TRUE(minimum_cost_triangulation(4, uniform, scratch, triangles, forbidZeroTwo));
  const std::span<const PolygonTriangle> quad{triangles.data(), 2};
  expect_valid_triangulation(4, quad);
  EXPECT_TRUE(contains_diagonal(quad, 1, 3));

  const auto noDiagonals = [](std::size_t, std::size_t) { return false; };
  EXPECT_FALSE(minimum_cost_triangulation(5, uniform, scratch, triangles, noDiagonals));
  EXPECT_TRUE(minimum_cost_triangulation(3, uniform, scratch, triangles, noDiagonals));

  const auto forbidden = [](std::size_t, std::size_t, std::size_t) { return std::numeric_limits<double>::infinity(); };
  EXPECT_FALSE(minimum_cost_triangulation(6, forbidden, scratch, triangles));
  ASSERT_TRUE(minimum_cost_triangulation(7, uniform, scratch, triangles));
  expect_valid_triangulation(7, triangles);
}

TEST(PolygonTriangulation, StackStorageForSmallPolygon)
{
  constexpr std::size_t cornerCount = 8;
  const std::vector<Vec2> corners = regular_polygon(cornerCount);
  std::array<TriangulationCell<double>, triangulation_scratch_size(cornerCount)> scratch{};
  std::array<PolygonTriangle, triangulation_triangle_count(cornerCount)> triangles{};

  ASSERT_TRUE(minimum_cost_triangulation(cornerCount, AspectRatioCost<double, 2>{corners}, scratch, triangles));
  expect_valid_triangulation(cornerCount, triangles);
}

TEST(PolygonTriangulation, HeapStorageForLargePolygonIsReusable)
{
  std::vector<TriangulationCell<double>> scratch;
  std::vector<PolygonTriangle> triangles;
  for (const std::size_t cornerCount : {50U, 20U})
  {
    const std::vector<Vec2> corners = regular_polygon(cornerCount);
    scratch.resize(triangulation_scratch_size(cornerCount));
    triangles.resize(triangulation_triangle_count(cornerCount));
    ASSERT_TRUE(minimum_cost_triangulation(cornerCount, AspectRatioCost<double, 2>{corners}, scratch, triangles));
    expect_valid_triangulation(cornerCount, triangles);
  }
}

TEST(PolygonTriangulation, BuiltInCostsKeepConcavePolygonInside)
{
  // A comb: the teeth hide most corners from each other, so many diagonals leave the polygon.
  const std::vector<Vec2> corners{{0.0, 0.0}, {5.0, 0.0}, {5.0, 3.0}, {4.0, 1.0}, {3.0, 3.0},
                                  {2.0, 1.0}, {1.0, 3.0}, {0.5, 1.0}, {0.0, 3.0}};
  const std::size_t cornerCount = corners.size();
  const double polygonArea = polygon_doubled_area(corners);
  ASSERT_GT(polygonArea, 0.0);

  std::vector<TriangulationCell<double>> scratch(triangulation_scratch_size(cornerCount));
  std::vector<PolygonTriangle> triangles(triangulation_triangle_count(cornerCount));
  const auto expect_inside = [&](const auto& cost) {
    ASSERT_TRUE(minimum_cost_triangulation(cornerCount, cost, scratch, triangles));
    expect_valid_triangulation(cornerCount, triangles);
    double triangleArea = 0.0;
    for (const auto& [first, apex, last] : triangles)
    {
      const double doubledArea = signed_doubled_area(corners[first], corners[apex], corners[last]);
      EXPECT_GT(doubledArea, 0.0);
      triangleArea += doubledArea;
    }
    // Positively oriented triangles covering exactly the polygon's area cannot overlap or stick out.
    EXPECT_NEAR(triangleArea, polygonArea, 1e-12);
  };
  expect_inside(AspectRatioCost<double, 2>{corners});
  expect_inside(AreaCost<double, 2>{corners});
  expect_inside(EdgeLengthCost<double, 2>{corners});
}

TEST(PolygonTriangulation, BuiltInCostsPreferTheShortDiagonalOfARhombus)
{
  const std::array<Vec2, 4> corners{Vec2{0.0, 0.0}, Vec2{2.0, -1.0}, Vec2{4.0, 0.0}, Vec2{2.0, 1.0}};
  std::array<TriangulationCell<double>, triangulation_scratch_size(4)> scratch{};
  std::array<PolygonTriangle, triangulation_triangle_count(4)> triangles{};

  ASSERT_TRUE(minimum_cost_triangulation(4, EdgeLengthCost<double, 2>{corners}, scratch, triangles));
  EXPECT_TRUE(contains_diagonal(triangles, 1, 3));
  ASSERT_TRUE(minimum_cost_triangulation(4, AspectRatioCost<double, 2>{corners}, scratch, triangles));
  EXPECT_TRUE(contains_diagonal(triangles, 1, 3));
}

TEST(PolygonTriangulation, MaxObjectiveOptimizesTheWorstTriangle)
{
  // Diagonal (0, 2) sums to 11 but its worst triangle costs 10; diagonal (1, 3) sums to 12 with
  // a worst triangle of 6.
  const QuadCosts<double> costs{1.0, 10.0, 6.0, 6.0};
  std::array<TriangulationCell<double>, triangulation_scratch_size(4)> scratch{};
  std::array<PolygonTriangle, triangulation_triangle_count(4)> triangles{};

  ASSERT_TRUE(minimum_cost_triangulation(4, costs, scratch, triangles));
  EXPECT_TRUE(contains_diagonal(triangles, 0, 2));
  ASSERT_TRUE(minimum_cost_triangulation(4, costs, scratch, triangles, AllowAllDiagonals{}, MaxObjective<double>{}));
  EXPECT_TRUE(contains_diagonal(triangles, 1, 3));
}

TEST(PolygonTriangulation, PairCostsBreakTiesWithTheSecondCriterion)
{
  using Cost = std::pair<double, double>;
  // Minimizes the worst first element, then the summed second element.
  struct WorstThenSum
  {
    static Cost identity() { return {-std::numeric_limits<double>::infinity(), 0.0}; }
    static Cost forbidden() { return {std::numeric_limits<double>::infinity(), 0.0}; }
    static Cost combine(const Cost& lhs, const Cost& rhs) { return {std::max(lhs.first, rhs.first), lhs.second + rhs.second}; }
  };
  const QuadCosts<Cost> costs{{1.0, 1.0}, {1.0, 5.0}, {1.0, 2.0}, {1.0, 2.0}};
  std::array<TriangulationCell<Cost>, triangulation_scratch_size(4)> scratch{};
  std::array<PolygonTriangle, triangulation_triangle_count(4)> triangles{};

  ASSERT_TRUE(minimum_cost_triangulation(4, costs, scratch, triangles, AllowAllDiagonals{}, WorstThenSum{}));
  EXPECT_TRUE(contains_diagonal(triangles, 1, 3));
}

TEST(PolygonTriangulation, AreaCostFillsNonPlanarHole)
{
  // A saddle-shaped ring, as left by removing the apex of a crumpled fan.
  constexpr std::size_t cornerCount = 8;
  std::vector<Vec3> corners;
  for (std::size_t i = 0; i < cornerCount; ++i)
  {
    const double angle = 2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(cornerCount);
    corners.push_back(Vec3{std::cos(angle), std::sin(angle), i % 2 == 0 ? 0.4 : -0.4});
  }
  std::array<TriangulationCell<double>, triangulation_scratch_size(cornerCount)> scratch{};
  std::array<PolygonTriangle, triangulation_triangle_count(cornerCount)> triangles{};

  const AreaCost<double, 3> area{corners};
  ASSERT_TRUE(minimum_cost_triangulation(cornerCount, area, scratch, triangles));
  expect_valid_triangulation(cornerCount, triangles);
  for (const auto& [first, apex, last] : triangles)
  {
    EXPECT_LT(area(first, apex, last), std::numeric_limits<double>::infinity());
  }
}
