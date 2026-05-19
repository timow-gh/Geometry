#include "Geometry/Intersect/BentleyOttmann.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace Geometry;

TEST(bentley_ottmann, createBentleyOttmann)
{
  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 4}, {4, 0}}, 0}, {Segment2d{{0, 0}, {4, 4}}, 1}};
  std::vector<linal::double2> intersections = bentley_ottmann(segments);
  EXPECT_EQ(1, intersections.size());
}

TEST(bentley_ottmann, MultipleIntersectionsAtSamePoint)
{
  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 0}, {4, 4}}, 0},
                                             {Segment2d{{0, 4}, {4, 0}}, 1},
                                             {Segment2d{{1, 3}, {3, 1}}, 2}};
  std::vector<linal::double2> intersections = bentley_ottmann(segments);
  EXPECT_EQ(2, intersections.size());
  EXPECT_TRUE(linal::is_equal(intersections[0], {2, 2}));
  EXPECT_TRUE(linal::is_equal(intersections[1], {2, 2}));
}

TEST(bentley_ottmann, NotSupported_PartiallyOverlappingSegments)
{
  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 0}, {4, 4}}, 0}, {Segment2d{{2, 2}, {4, 4}}, 1}};
  std::vector<linal::double2> intersections = bentley_ottmann(segments);
  EXPECT_EQ(0, intersections.size());
}

//TEST(bentley_ottmann, SinglePointMultipleSegments)
//{
//  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 0}, {0, 4}}, 0},
//                                             {Segment2d{{0, 2}, {4, 2}}, 1},
//                                             {Segment2d{{0, 2}, {-4, 2}}, 2}};
//  std::vector<linal::double2> intersections = bentley_ottmann(segments);
//  EXPECT_EQ(2, intersections.size());
//  EXPECT_TRUE(linal::is_equal(intersections[0], {0, 2}));
//  EXPECT_TRUE(linal::is_equal(intersections[1], {0, 2}));
//}

TEST(bentley_ottmann, DegenerateCase)
{
  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 0}, {0, 0}}, 0}, {Segment2d{{0, 0}, {1, 1}}, 1}};
  std::vector<linal::double2> intersections = bentley_ottmann(segments);
  EXPECT_EQ(0, intersections.size());
}

TEST(bentley_ottmann, NoIntersections)
{
  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 0}, {1, 0}}, 0}, {Segment2d{{2, 2}, {3, 2}}, 1}};
  std::vector<linal::double2> intersections = bentley_ottmann(segments);
  EXPECT_TRUE(intersections.empty());
}

TEST(bentley_ottmann, CollinearSegments)
{
  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 0}, {4, 4}}, 0}, {Segment2d{{1, 1}, {3, 3}}, 1}};
  std::vector<linal::double2> intersections = bentley_ottmann(segments);
  EXPECT_TRUE(intersections.empty());
}

TEST(bentley_ottmann, ParallelSegments)
{
  std::vector<BOSegment<double>> segments = {{Segment2d{{0, 0}, {1, 1}}, 0}, {Segment2d{{0, 1}, {1, 2}}, 1}};
  std::vector<linal::double2> intersections = bentley_ottmann(segments);
  EXPECT_TRUE(intersections.empty());
}
