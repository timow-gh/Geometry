#include <Geometry/Intersection/IntersectionSphere.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>
#include <linal/vec3.hpp>
#include <optional>

using namespace Geometry;

TEST(Sphere_Line3d, Intersection)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Line3d line{linal::ZERO_VEC3D, linal::X_VEC3D};
  auto sphereIntersec = Geometry::intersection(sphere, line);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::X_VEC3D);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::X_VEC3D);
}

TEST(Sphere_Line3d, Touching)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Line3d line{linal::Z_VEC3D, linal::X_VEC3D};
  auto sphereIntersec = Geometry::intersection(sphere, line);
  EXPECT_EQ(sphereIntersec.first.value(), linal::Z_VEC3D);
  EXPECT_FALSE(sphereIntersec.second);
}

TEST(Sphere_Line3d, No_Intersection)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Line3d line{linal::vec3d{0, 0, 2}, linal::X_VEC3D};
  auto sphereIntersec = Geometry::intersection(sphere, line);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Ray3d, Intersection)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Ray3d ray{linal::vec3d{-3, 0, 0}, linal::X_VEC3D};
  auto sphereIntersec = Geometry::intersection(sphere, ray);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::X_VEC3D);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::X_VEC3D);
}

TEST(Sphere_Ray3d, Touching)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Ray3d ray{linal::Z_VEC3D, linal::X_VEC3D};
  auto sphereIntersec = Geometry::intersection(sphere, ray);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_FALSE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::Z_VEC3D);
}

TEST(Sphere_Ray3d, InsideSphere)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Ray3d ray{linal::ZERO_VEC3D, linal::X_VEC3D};
  auto sphereIntersec = Geometry::intersection(sphere, ray);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_FALSE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::X_VEC3D);
}

TEST(Sphere_Ray3d, No_Intersection)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Ray3d ray{linal::vec3d{0, 3, 0}, linal::X_VEC3D};
  auto sphereIntersec = Geometry::intersection(sphere, ray);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Segment3d, Intersection)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Segment3d seg{linal::vec3d{-2, 0, 0}, linal::vec3d{2, 0, 0}};
  auto sphereIntersec = Geometry::intersection(sphere, seg);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::X_VEC3D);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::X_VEC3D);
}

TEST(Sphere_Segment3d, Intersection_LHS)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Segment3d seg{linal::vec3d{-2, 0, 0}, linal::vec3d{0, 0, 0}};
  auto sphereIntersec = Geometry::intersection(sphere, seg);
  EXPECT_FALSE(sphereIntersec.first);
  EXPECT_TRUE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first, std::nullopt);
  EXPECT_EQ(sphereIntersec.second.value(), -linal::X_VEC3D);
}

TEST(Sphere_Segment3d, Intersection_RHS)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Segment3d seg{linal::vec3d{0, 0, 0}, linal::vec3d{2, 0, 0}};
  auto sphereIntersec = Geometry::intersection(sphere, seg);
  EXPECT_TRUE(sphereIntersec.first);
  EXPECT_FALSE(sphereIntersec.second);
  EXPECT_EQ(sphereIntersec.first.value(), linal::X_VEC3D);
  EXPECT_EQ(sphereIntersec.second, std::nullopt);
}

TEST(Sphere_Segment3d, Inside_Sphere)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Segment3d seg{linal::vec3d{-0.5, 0, 0}, linal::vec3d{0.5, 0, 0}};
  auto sphereIntersec = Geometry::intersection(sphere, seg);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Segment3d, No_Intersection)
{
  Sphere sphere{linal::ZERO_VEC3D, 1.0};
  Segment3d seg{linal::vec3d{-2, 2, 0}, linal::vec3d{2, 2, 0}};
  auto sphereIntersec = Geometry::intersection(sphere, seg);
  EXPECT_FALSE(sphereIntersec.hasIntersection());
}
