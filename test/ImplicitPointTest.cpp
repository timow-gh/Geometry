#include <Geometry/Predicates.hpp>
#include <Geometry/detail/ImplicitPoint.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

#include <cstdint>

using namespace Geometry;

namespace
{

using Vec2 = linal::double2;
using Vec3 = linal::double3;
using Point = detail::ImplicitPoint<double>;
using Kind = detail::ImplicitPointKind;

// Exact comparison: linal's vector equality has a tolerance.
void expect_exactly(const Vec3& actual, const Vec3& expected)
{
  EXPECT_EQ(actual[0], expected[0]);
  EXPECT_EQ(actual[1], expected[1]);
  EXPECT_EQ(actual[2], expected[2]);
}

TEST(ImplicitPointTest, explicit_point_is_its_input_vertex)
{
  const Vec3 vertex{1.5, -2.0, 3.25};
  const Point point = Point::create_explicit(vertex);

  EXPECT_EQ(point.kind(), Kind::Explicit);
  expect_exactly(point.position(), vertex);
  ASSERT_EQ(point.definition().size(), 1U);
  expect_exactly(point.definition()[0], vertex);
}

TEST(ImplicitPointTest, default_point_is_the_explicit_origin)
{
  const Point point;
  EXPECT_EQ(point.kind(), Kind::Explicit);
  expect_exactly(point.position(), Vec3{0.0, 0.0, 0.0});
}

TEST(ImplicitPointTest, edge_plane_point_keeps_definition_and_lies_on_both)
{
  // The plane x + y + z = 3 and the diagonal through the origin meet at (1, 1, 1), which is
  // representable, so the cached position is exact.
  const Vec3 source{0.0, 0.0, 0.0};
  const Vec3 target{2.0, 2.0, 2.0};
  const Vec3 planeFirst{3.0, 0.0, 0.0};
  const Vec3 planeSecond{0.0, 3.0, 0.0};
  const Vec3 planeThird{0.0, 0.0, 3.0};
  const Point point = Point::create_edge_plane(source, target, planeFirst, planeSecond, planeThird);

  EXPECT_EQ(point.kind(), Kind::EdgePlane);
  expect_exactly(point.position(), Vec3{1.0, 1.0, 1.0});
  ASSERT_EQ(point.definition().size(), 5U);
  expect_exactly(point.definition()[0], source);
  expect_exactly(point.definition()[1], target);
  expect_exactly(point.definition()[2], planeFirst);
  expect_exactly(point.definition()[3], planeSecond);
  expect_exactly(point.definition()[4], planeThird);

  EXPECT_EQ(orient3d(Point::create_explicit(planeFirst), Point::create_explicit(planeSecond), Point::create_explicit(planeThird), point),
            Orientation::Zero);
}

TEST(ImplicitPointTest, edge_plane_point_divides_edge_by_distance_ratio)
{
  // Distances 1 and 3 from the plane z = 0: a quarter of the way along the edge.
  const Point point = Point::create_edge_plane(Vec3{0.0, 0.0, -1.0}, Vec3{4.0, 8.0, 3.0}, Vec3{0.0, 0.0, 0.0},
                                               Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0});
  expect_exactly(point.position(), Vec3{1.0, 2.0, 0.0});

  // Reversing the edge defines the same point.
  const Point reversed = Point::create_edge_plane(Vec3{4.0, 8.0, 3.0}, Vec3{0.0, 0.0, -1.0}, Vec3{0.0, 0.0, 0.0},
                                                  Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0});
  expect_exactly(reversed.position(), Vec3{1.0, 2.0, 0.0});
}

TEST(ImplicitPointTest, edge_edge_point_is_lifted_onto_the_tilted_plane)
{
  // Both edges lie in the plane z = x, whose normal (-1, 0, 1) drops axis 0 (ties go to the lowest).
  const Vec3 firstSource{0.0, 0.0, 0.0};
  const Vec3 firstTarget{2.0, 2.0, 2.0};
  const Vec3 secondSource{0.0, 2.0, 0.0};
  const Vec3 secondTarget{2.0, 0.0, 2.0};
  const Point point = Point::create_edge_edge(firstSource, firstTarget, secondSource, secondTarget, 0);

  EXPECT_EQ(point.kind(), Kind::EdgeEdge);
  EXPECT_EQ(point.axis(), 0);
  expect_exactly(point.position(), Vec3{1.0, 1.0, 1.0});
  ASSERT_EQ(point.definition().size(), 4U);
  expect_exactly(point.definition()[2], secondSource);
  expect_exactly(point.definition()[3], secondTarget);
}

TEST(ImplicitPointTest, orient3d_on_explicit_points_equals_plain_orient3d)
{
  const Vec3 first{0.0, 0.0, 0.0};
  const Vec3 second{1.0, 0.0, 0.0};
  const Vec3 third{0.0, 1.0, 0.0};
  for (const Vec3& query : {Vec3{0.2, 0.3, 1.0}, Vec3{0.2, 0.3, -1.0}, Vec3{5.0, -7.0, 0.0}})
  {
    EXPECT_EQ(orient3d(Point::create_explicit(first), Point::create_explicit(second), Point::create_explicit(third),
                       Point::create_explicit(query)),
              orient3d(first, second, third, query));
  }
}

TEST(ImplicitPointTest, orient2d_projects_by_dropping_the_axis)
{
  // Counterclockwise seen from +y, i.e. in the (z, x) projection that drops axis 1.
  const Point first = Point::create_explicit(Vec3{0.0, 5.0, 0.0});
  const Point second = Point::create_explicit(Vec3{0.0, -3.0, 1.0});
  const Point query = Point::create_explicit(Vec3{1.0, 7.0, 0.0});
  EXPECT_EQ(orient2d(first, second, query, 1), orient2d(Vec2{0.0, 0.0}, Vec2{1.0, 0.0}, Vec2{0.0, 1.0}));
  EXPECT_EQ(orient2d(first, second, query, 1), Orientation::Positive);
  // NOLINTNEXTLINE(readability-suspicious-call-argument): swapped on purpose to flip the orientation
  EXPECT_EQ(orient2d(second, first, query, 1), Orientation::Negative);

  // An intersection point on the line through first and second.
  const Point crossing = Point::create_edge_edge(Vec3{0.0, 0.0, 0.5}, Vec3{0.0, 0.0, 1.5}, Vec3{-1.0, 0.0, 1.0},
                                                 Vec3{1.0, 0.0, 1.0}, 1);
  EXPECT_EQ(orient2d(first, second, crossing, 1), Orientation::Zero);
}

} // namespace
