#include <Geometry/Predicates.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

#include <array>
#include <cstdint>

using namespace Geometry;

namespace
{

using Vec2 = linal::double2;
using Vec3 = linal::double3;

TEST(PredicatesTest, orient2d_counterclockwise_is_positive)
{
  EXPECT_EQ(orient2d(Vec2{0.0, 0.0}, Vec2{1.0, 0.0}, Vec2{0.0, 1.0}), Orientation::Positive);
  EXPECT_EQ(orient2d(Vec2{0.0, 0.0}, Vec2{1.0, 0.0}, Vec2{0.0, -1.0}), Orientation::Negative);
}

TEST(PredicatesTest, orient2d_collinear_integer_points_are_exactly_zero)
{
  EXPECT_EQ(orient2d(Vec2{0.0, 0.0}, Vec2{3.0, 7.0}, Vec2{6.0, 14.0}), Orientation::Zero);
  // Query between the endpoints and query coinciding with an endpoint.
  EXPECT_EQ(orient2d(Vec2{-2.0, 5.0}, Vec2{4.0, -1.0}, Vec2{1.0, 2.0}), Orientation::Zero);
  EXPECT_EQ(orient2d(Vec2{-2.0, 5.0}, Vec2{4.0, -1.0}, Vec2{4.0, -1.0}), Orientation::Zero);
}

TEST(PredicatesTest, orient2d_swap_flips_and_rotation_keeps_sign)
{
  const Vec2 first{0.5, -1.0};
  const Vec2 second{3.0, 2.0};
  const Vec2 third{-1.0, 4.0};
  const Orientation reference = orient2d(first, second, third);
  ASSERT_EQ(reference, Orientation::Positive);

  EXPECT_EQ(orient2d(second, first, third), Orientation::Negative);
  EXPECT_EQ(orient2d(first, third, second), Orientation::Negative);
  EXPECT_EQ(orient2d(third, second, first), Orientation::Negative);
  EXPECT_EQ(orient2d(second, third, first), reference);
  EXPECT_EQ(orient2d(third, first, second), reference);
}

TEST(PredicatesTest, orient3d_normal_side_is_positive)
{
  // Counterclockwise seen from +z: the right-hand normal points to +z.
  const Vec3 first{0.0, 0.0, 0.0};
  const Vec3 second{1.0, 0.0, 0.0};
  const Vec3 third{0.0, 1.0, 0.0};
  EXPECT_EQ(orient3d(first, second, third, Vec3{0.2, 0.2, 1.0}), Orientation::Positive);
  EXPECT_EQ(orient3d(first, second, third, Vec3{0.2, 0.2, -1.0}), Orientation::Negative);
  // The query need not project into the triangle: the predicate is about the plane.
  EXPECT_EQ(orient3d(first, second, third, Vec3{-5.0, 7.0, 0.5}), Orientation::Positive);
}

TEST(PredicatesTest, orient3d_coplanar_integer_points_are_exactly_zero)
{
  // All points satisfy x + y + z = 3, on a plane not aligned with any axis.
  const Vec3 first{3.0, 0.0, 0.0};
  const Vec3 second{0.0, 3.0, 0.0};
  const Vec3 third{0.0, 0.0, 3.0};
  EXPECT_EQ(orient3d(first, second, third, Vec3{1.0, 1.0, 1.0}), Orientation::Zero);
  EXPECT_EQ(orient3d(first, second, third, Vec3{5.0, -4.0, 2.0}), Orientation::Zero);
  EXPECT_EQ(orient3d(first, second, third, second), Orientation::Zero);
}

TEST(PredicatesTest, orient3d_swap_flips_and_even_permutation_keeps_sign)
{
  const Vec3 first{1.0, -2.0, 0.5};
  const Vec3 second{4.0, 1.0, -1.0};
  const Vec3 third{-1.0, 3.0, 2.0};
  const Vec3 query{0.0, 0.0, 6.0};
  const Orientation reference = orient3d(first, second, third, query);
  ASSERT_NE(reference, Orientation::Zero);
  const auto opposite = static_cast<Orientation>(-static_cast<std::int8_t>(reference));

  EXPECT_EQ(orient3d(second, first, third, query), opposite);
  EXPECT_EQ(orient3d(first, third, second, query), opposite);
  EXPECT_EQ(orient3d(first, second, query, third), opposite);
  EXPECT_EQ(orient3d(query, second, third, first), opposite);
  EXPECT_EQ(orient3d(second, third, first, query), reference);
  EXPECT_EQ(orient3d(second, first, query, third), reference);
}

TEST(PredicatesTest, dominant_axis_picks_largest_magnitude)
{
  EXPECT_EQ(detail::dominant_axis(Vec3{3.0, -1.0, 2.0}), 0);
  EXPECT_EQ(detail::dominant_axis(Vec3{0.5, -4.0, 2.0}), 1);
  EXPECT_EQ(detail::dominant_axis(Vec3{0.5, 1.0, -2.0}), 2);
}

TEST(PredicatesTest, dominant_axis_ties_choose_lowest_index)
{
  EXPECT_EQ(detail::dominant_axis(Vec3{1.0, -1.0, 1.0}), 0);
  EXPECT_EQ(detail::dominant_axis(Vec3{0.0, 2.0, -2.0}), 1);
  EXPECT_EQ(detail::dominant_axis(Vec3{1e-300, 0.0, 0.0}), 0);
}

TEST(PredicatesTest, project_dropping_axis_keeps_cyclic_order)
{
  const Vec3 point{1.0, 2.0, 3.0};
  EXPECT_EQ(detail::project_dropping_axis(point, 0), (Vec2{2.0, 3.0}));
  EXPECT_EQ(detail::project_dropping_axis(point, 1), (Vec2{3.0, 1.0}));
  EXPECT_EQ(detail::project_dropping_axis(point, 2), (Vec2{1.0, 2.0}));
}

TEST(PredicatesTest, projected_orientation_follows_normal_component)
{
  // A tilted triangle and its reverse, so every axis sees both normal signs.
  const std::array<Vec3, 3> triangle{Vec3{0.0, 0.0, 0.0}, Vec3{4.0, 1.0, 2.0}, Vec3{1.0, 3.0, -1.0}};
  const std::array<Vec3, 3> reversed{triangle[0], triangle[2], triangle[1]};

  for (const std::array<Vec3, 3>& corners : {triangle, reversed})
  {
    const Vec3 normal = detail::triangle_orientation(corners[0], corners[1], corners[2]);
    for (std::uint8_t axis = 0; axis < 3; ++axis)
    {
      const Orientation projected = orient2d(detail::project_dropping_axis(corners[0], axis),
                                             detail::project_dropping_axis(corners[1], axis),
                                             detail::project_dropping_axis(corners[2], axis));
      EXPECT_EQ(projected, detail::orientation_from_determinant(normal[axis])) << "axis " << int{axis};
    }
  }
}

} // namespace
