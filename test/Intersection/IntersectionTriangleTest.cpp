#include <Geometry/Intersect/IntersectTriangle.hpp>
#include <Geometry/Triangle.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

// Test fixture for triangle intersection tests
class Triangle_Triangle_Test : public ::testing::Test {
protected:
  // Common triangles used in multiple tests
  Triangle3d m_tri1{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}; // Triangle in XY plane
  Triangle3d m_tri2{{0, 0, 1}, {1, 0, 1}, {0, 1, 1}}; // Parallel to m_tri1, offset by 1 in Z
};

// Test triangles that intersect
TEST_F(Triangle_Triangle_Test, Intersecting)
{
  // Second triangle intersects the first through the Z-axis
  Triangle3d tri2{{0, 0, -1}, {1, 0, -1}, {0.5, 0.5, 1}};
  EXPECT_TRUE(is_intersecting(m_tri1, tri2));
}

// Test triangles that share a vertex
TEST_F(Triangle_Triangle_Test, SharedVertex)
{
  Triangle3d tri2{{0, 0, 0}, {0, 0, 1}, {1, 0, 1}};
  EXPECT_TRUE(is_intersecting(m_tri1, tri2));
}

// Test triangles that share an edge
TEST_F(Triangle_Triangle_Test, SharedEdge)
{
  Triangle3d tri2{{0, 0, 0}, {1, 0, 0}, {0.5, 0.5, 1}};
  EXPECT_TRUE(is_intersecting(m_tri1, tri2));
}

// Test triangles that are parallel but don't intersect
TEST_F(Triangle_Triangle_Test, ParallelNonIntersecting)
{
  EXPECT_FALSE(is_intersecting(m_tri1, m_tri2)); // m_tri2 is parallel and above m_tri1
}

// Test triangles that are coplanar and intersecting
TEST_F(Triangle_Triangle_Test, CoplanarIntersecting)
{
  Triangle3d tri2{{0.5, 0.5, 0}, {1.5, 0.5, 0}, {0.5, 1.5, 0}};
  EXPECT_TRUE(is_intersecting(m_tri1, tri2));
}

// Test triangles that are coplanar but don't intersect
TEST_F(Triangle_Triangle_Test, CoplanarNonIntersecting)
{
  Triangle3d tri2{{2, 0, 0}, {3, 0, 0}, {2, 1, 0}}; // Adjacent but not overlapping
  EXPECT_FALSE(is_intersecting(m_tri1, tri2));
}

// Test triangles where one contains the other
TEST_F(Triangle_Triangle_Test, Containing)
{
  Triangle3d largeTriangle{{-1, -1, 0}, {2, -1, 0}, {0.5, 2, 0}};
  EXPECT_TRUE(is_intersecting(m_tri1, largeTriangle));
}

// Test triangles that almost intersect but don't
TEST_F(Triangle_Triangle_Test, AlmostIntersecting)
{
  Triangle3d tri2{{-0.01, -0.01, 0}, {-1, 0, 0}, {0, -1, 0}}; // Very close but not intersecting
  EXPECT_FALSE(is_intersecting(m_tri1, tri2));
}

// Test triangles that intersect in just one point (a vertex)
TEST_F(Triangle_Triangle_Test, PointIntersection)
{
  Triangle3d tri2{{1, 0, 0}, {2, 0, 0}, {1, 1, 0}}; // Only the vertex at (1,0,0) intersects
  EXPECT_TRUE(is_intersecting(m_tri1, tri2));
}

// Test triangles crossing at a slight angle
TEST_F(Triangle_Triangle_Test, CrossingAtAngle)
{
  Triangle3d tri1{{-1, 0, 0}, {1, 0, 0}, {0, 1, 0.5}};
  Triangle3d tri2{{0, -1, 0}, {0, 1, 0}, {0, 0, 1}};
  EXPECT_TRUE(is_intersecting(tri1, tri2));
}

// Test triangles that share the same plane but don't intersect
TEST_F(Triangle_Triangle_Test, SamePlaneNoIntersection)
{
  Triangle3d tri1{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  Triangle3d tri2{{2, 2, 0}, {3, 2, 0}, {2, 3, 0}};
  EXPECT_FALSE(is_intersecting(tri1, tri2));
}

// Test triangles that are extremely close but don't intersect
TEST_F(Triangle_Triangle_Test, VeryCloseNoIntersection)
{
  Triangle3d tri1{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  Triangle3d tri2{{0, 0, 0.000001}, {1, 0, 0.000001}, {0, 1, 0.000001}}; // Slightly above in Z
  EXPECT_FALSE(is_intersecting(tri1, tri2));
}

// Test self-intersection (should always be true)
TEST_F(Triangle_Triangle_Test, SelfIntersection)
{
  EXPECT_TRUE(is_intersecting(m_tri1, m_tri1));
}

// Test triangles with a small gap between them
TEST_F(Triangle_Triangle_Test, SmallGap)
{
  Triangle3d tri1{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  Triangle3d tri2{{0, 0, 0.01}, {1, 0, 0.01}, {0, 1, 0.01}}; // Small gap in Z
  EXPECT_FALSE(is_intersecting(tri1, tri2));
}