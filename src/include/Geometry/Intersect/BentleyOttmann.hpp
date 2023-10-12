#ifndef GEOMETRY_BENTLEYOTTMANN
#define GEOMERTY_BENTLEYOTTMANN

#include "Geometry/Intersect/IntersectSegment.hpp"
#include "Geometry/Segment.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <set>
#include <vector>

namespace Geometry
{

template <typename T>
struct BOSegment
{
  Segment2<T> segment;
  int id; // Unique identifier for each segment
};

enum class EventType : std::uint8_t
{
  Start,
  End
};

template <typename T>
struct Event
{
  linal::vec2<T> point;
  int segmentId;
  EventType type;
};

// Function to compute y-coordinate of a segment at a given x
template <typename T>
T compute_y(const Segment2<T>& seg, T x)
{
  const linal::vec2<T>& a = seg.get_source();
  const linal::vec2<T>& b = seg.get_target();
  if (a[0] == b[0])
    return a[1]; // Vertical segment
  return a[1] + (b[1] - a[1]) * (x - a[0]) / (b[0] - a[0]);
}

template <typename T>
std::vector<linal::vec2<T>> bentley_ottmann(const std::vector<BOSegment<T>>& boSegments)
{
  using Event = Event<T>;

  std::vector<linal::vec2<T>> intersections;

  auto compEvent = [](const Event& e1, const Event& e2) { return e1.point[0] > e2.point[0]; };
  std::priority_queue<Event, std::vector<Event>, decltype(compEvent)> eventQueue(compEvent);

  auto compStatus = [&boSegments](int id1, int id2)
  {
    const BOSegment<T>& s1 = boSegments[id1];
    const linal::vec2<T>& s1a = s1.segment.get_source();
    const linal::vec2<T>& s1b = s1.segment.get_target();
    const BOSegment<T>& s2 = boSegments[id2];
    const linal::vec2<T>& s2a = s2.segment.get_source();
    const linal::vec2<T>& s2b = s2.segment.get_target();
    T x = std::max(std::min(s1a[0], s1b[0]), std::min(s2a[0], s2b[0]));
    T y1 = compute_y(s1.segment, x);
    T y2 = compute_y(s2.segment, x);
    return y1 < y2;
  };
  std::set<int, decltype(compStatus)> status(compStatus);

  for (int i = 0; i < boSegments.size(); ++i)
  {
    eventQueue.push({boSegments[i].segment.get_source(), i, EventType::Start});
    eventQueue.push({boSegments[i].segment.get_target(), i, EventType::End});
  }

  while (!eventQueue.empty())
  {
    Event currentEvent = eventQueue.top();
    eventQueue.pop();

    if (currentEvent.type == EventType::Start)
    {
      auto it = status.insert(currentEvent.segmentId).first;
      if (it == status.end())
      {
        continue;
      }

      auto above = std::next(it);
      auto below = (it == status.begin()) ? status.end() : std::prev(it);

      if (above != status.end())
      {
        if (std::optional<linal::vec2<T>> intersection = Geometry::intersect(boSegments[*it].segment, boSegments[*above].segment))
        {
          intersections.push_back(*intersection);
        }
      }

      if (below != status.end())
      {
        if (std::optional<linal::vec2<T>> intersection = Geometry::intersect(boSegments[*it].segment, boSegments[*below].segment))
        {
          intersections.push_back(*intersection);
        }
      }
    }
    else
    {
      auto it = status.find(currentEvent.segmentId);
      if (it == status.end())
      {
        continue;
      }

      auto above = std::next(it);
      auto below = (it == status.begin()) ? status.end() : std::prev(it);

      if (above != status.end() && below != status.end())
      {
        if (std::optional<linal::vec2<T>> intersection = Geometry::intersect(boSegments[*above].segment, boSegments[*below].segment))
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