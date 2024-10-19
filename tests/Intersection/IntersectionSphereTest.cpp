#include <Geometry/Intersect/IntersectSphere.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>
#include <optional>

using namespace Geometry;

TEST(Sphere_Line3d, Intersection)
{
  Sphere sphere{linal::double3{}, 1.0};
  Line3d line{linal::double3{}, linal::double3X};
  auto sphereIntersec = Geometry::intersect(sphere, line);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::double3X);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::double3X);
}

TEST(Sphere_Line3d, Touching)
{
  Sphere sphere{linal::double3{}, 1.0};
  Line3d line{linal::double3Z, linal::double3X};
  auto sphereIntersec = Geometry::intersect(sphere, line);
  EXPECT_EQ(sphereIntersec.first.value(), linal::double3Z);
  EXPECT_FALSE(sphereIntersec.second);
}

TEST(Sphere_Line3d, No_Intersection)
{
  Sphere sphere{linal::double3{}, 1.0};
  Line3d line{linal::double3{0, 0, 2}, linal::double3X};
  auto sphereIntersec = Geometry::intersect(sphere, line);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Ray3d, Intersection)
{
  Sphere sphere{linal::double3{}, 1.0};
  Ray3d ray{linal::double3{-3, 0, 0}, linal::double3X};
  auto sphereIntersec = Geometry::intersect(sphere, ray);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::double3X);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::double3X);
}

TEST(Sphere_Ray3d, Touching)
{
  Sphere sphere{linal::double3{}, 1.0};
  Ray3d ray{linal::double3Z, linal::double3X};
  auto sphereIntersec = Geometry::intersect(sphere, ray);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_FALSE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::double3Z);
}

TEST(Sphere_Ray3d, InsideSphere)
{
  Sphere sphere{linal::double3{}, 1.0};
  Ray3d ray{linal::double3{}, linal::double3X};
  auto sphereIntersec = Geometry::intersect(sphere, ray);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_FALSE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::double3X);
}

TEST(Sphere_Ray3d, No_Intersection)
{
  Sphere sphere{linal::double3{}, 1.0};
  Ray3d ray{linal::double3{0, 3, 0}, linal::double3X};
  auto sphereIntersec = Geometry::intersect(sphere, ray);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Segment3d, Intersection)
{
  Sphere sphere{linal::double3{}, 1.0};
  Segment3d seg{linal::double3{-2, 0, 0}, linal::double3{2, 0, 0}};
  auto sphereIntersec = Geometry::intersect(sphere, seg);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::double3X);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::double3X);
}

TEST(Sphere_Segment3d, Intersection_LHS)
{
  Sphere sphere{linal::double3{}, 1.0};
  Segment3d seg{linal::double3{-2, 0, 0}, linal::double3{0, 0, 0}};
  auto sphereIntersec = Geometry::intersect(sphere, seg);
  EXPECT_FALSE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first, std::nullopt);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::double3X);
}

TEST(Sphere_Segment3d, Intersection_RHS)
{
  Sphere sphere{linal::double3{}, 1.0};
  Segment3d seg{linal::double3{0, 0, 0}, linal::double3{2, 0, 0}};
  auto sphereIntersec = Geometry::intersect(sphere, seg);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_FALSE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::double3X);
  EXPECT_EQ(sphereIntersec.second, std::nullopt);
}

TEST(Sphere_Segment3d, Inside_Sphere)
{
  Sphere sphere{linal::double3{}, 1.0};
  Segment3d seg{linal::double3{-0.5, 0, 0}, linal::double3{0.5, 0, 0}};
  auto sphereIntersec = Geometry::intersect(sphere, seg);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Segment3d, No_Intersection)
{
  Sphere sphere{linal::double3{}, 1.0};
  Segment3d seg{linal::double3{-2, 2, 0}, linal::double3{2, 2, 0}};
  auto sphereIntersec = Geometry::intersect(sphere, seg);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}
