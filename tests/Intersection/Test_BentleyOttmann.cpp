//#include <gtest/gtest.h>

#include "Geometry/Intersect/BentleyOttmann.hpp"
#include <iostream>

using namespace Geometry;

int main()
{
  std::vector<Segment> segments = {{{0, 4, 0}, {4, 0, 0}, 0}, {{0, 0, 0}, {4, 4, 0}, 1}};

  std::vector<Point> intersections = BentleyOttmann(segments);

  for (const auto& point: intersections)
  {
    std::cout << "Intersection at (" << point.x << ", " << point.y << ")\n";
  }

  return 0;
}

