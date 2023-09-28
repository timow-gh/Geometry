#ifndef GEOMETRY_BENTLEYOTTMANN
#define GEOMERTY_BENTLEYOTTMANN

#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <set>
#include <vector>

namespace Geometry
{

struct Point
{
  float x, y, z;
};

struct Segment
{
  Point a, b;
  int id; // Unique identifier for each segment
};

struct Event
{
  Point point;
  int segmentId;
  bool isStart;
};

// Function to compute y-coordinate of a segment at a given x
float ComputeY(const Segment& seg, float x)
{
  if (seg.a.x == seg.b.x)
    return seg.a.y; // Vertical segment
  return seg.a.y + (seg.b.y - seg.a.y) * (x - seg.a.x) / (seg.b.x - seg.a.x);
}

std::optional<Point> algebraic_line_intersection(const Point& A, const Point& B, const Point& C, const Point& D)
{
  constexpr double TOLERANCE = 1e-9;

  double dx1 = B.x - A.x, dy1 = B.y - A.y;
  double dx2 = D.x - C.x, dy2 = D.y - C.y;
  double deltax = C.x - A.x, deltay = C.y - A.y;

  double denom = dx1 * dy2 - dy1 * dx2;

  if (std::abs(denom) < TOLERANCE)
  {
    return std::nullopt;
  }

  double t = (deltax * dy2 - deltay * dx2) / denom;
  double s = (deltax * dy1 - deltay * dx1) / denom;

  if (t >= 0 && t <= 1 && s >= 0 && s <= 1)
  {
    return Point{static_cast<float>(A.x + t * dx1), static_cast<float>(A.y + t * dy1), 0};
  }

  return std::nullopt;
}

std::vector<Point> BentleyOttmann(const std::vector<Segment>& segments)
{
  std::vector<Point> intersections;

  auto compEvent = [](const Event& e1, const Event& e2) { return e1.point.x > e2.point.x; };
  std::priority_queue<Event, std::vector<Event>, decltype(compEvent)> eventQueue(compEvent);

  auto compStatus = [&segments](int id1, int id2)
  {
    const Segment& s1 = segments[id1];
    const Segment& s2 = segments[id2];
    float x = std::max(std::min(s1.a.x, s1.b.x), std::min(s2.a.x, s2.b.x));
    float y1 = ComputeY(s1, x);
    float y2 = ComputeY(s2, x);
    return y1 < y2;
  };
  std::set<int, decltype(compStatus)> status(compStatus);

  for (int i = 0; i < segments.size(); ++i)
  {
    eventQueue.push({segments[i].a, i, true});
    eventQueue.push({segments[i].b, i, false});
  }

  while (!eventQueue.empty())
  {
    Event currentEvent = eventQueue.top();
    eventQueue.pop();

    if (currentEvent.isStart)
    {
      auto it = status.insert(currentEvent.segmentId).first;
      if (it == status.end())
        continue;

      auto above = std::next(it);
      auto below = (it == status.begin()) ? status.end() : std::prev(it);

      if (above != status.end())
      {
        if (auto intersection = algebraic_line_intersection(segments[*it].a, segments[*it].b, segments[*above].a, segments[*above].b))
        {
          intersections.push_back(*intersection);
        }
      }

      if (below != status.end())
      {
        if (auto intersection = algebraic_line_intersection(segments[*it].a, segments[*it].b, segments[*below].a, segments[*below].b))
        {
          intersections.push_back(*intersection);
        }
      }
    }
    else
    {
      auto it = status.find(currentEvent.segmentId);
      if (it == status.end())
        continue;

      auto above = std::next(it);
      auto below = (it == status.begin()) ? status.end() : std::prev(it);

      if (above != status.end() && below != status.end())
      {
        if (auto intersection = algebraic_line_intersection(segments[*above].a, segments[*above].b, segments[*below].a, segments[*below].b))
        {
          intersections.push_back(*intersection);
        }
      }

      status.erase(it);
    }
  }

  return intersections;
}

} // namespace Geometry

#endif