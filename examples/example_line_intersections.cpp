#include <Geometry/Intersect/IntersectSegment.hpp>
#include <Geometry/Segment.hpp>
#include <geoqik/GeoQik.hpp>

// static void draw_vec(const linal::vec3<double>& vec, float r, float g, float b, float a)
// {
//   geoqik_add_line_with_color(0.0, 0.0, 0.0, vec[0], vec[1], vec[2], r, g, b, a);
// }

static void draw_line(const Geometry::Segment3d& line, float r, float g, float b, float a)
{
  geoqik_add_line_with_color(line.get_source()[0],
                             line.get_source()[1],
                             line.get_source()[2],
                             line.get_target()[0],
                             line.get_target()[1],
                             line.get_target()[2],
                             r,
                             g,
                             b,
                             a);
}

int main()
{
  geoqik_init();

  geoqik_add_point(0.0, 0.0, 0.0);
  geoqik_add_point(1.0, 0.0, 0.0);
  geoqik_add_point(0.0, 1.0, 0.0);
  geoqik_add_point(0.0, 0.0, 1.0);

  geoqik_draw();

  // Example usage of the Line class
  Geometry::Segment3d seg1{{0.0, 0.0, 0.0}, {2.0, 2.0, 2.0}};
  linal::double3 seg2Source{0.0, 1.0, 1.0};
  linal::double3 seg2Target{1.0, 1.0, 1.0};
  Geometry::Segment3d seg2{seg2Source, seg2Source + (seg2Target - seg2Source) * 1.5};
  draw_line(seg1, 0.0f, 1.0f, 0.0f, 0.8f);
  draw_line(seg2, 0.0f, 1.0f, 0.0f, 0.8f);

  std::optional<linal::vec3<double>> intersection = Geometry::intersect(seg1, seg2);
  if (intersection.has_value())
  {
    geoqik_set_point_color(1.0f, 0.0f, 0.0f, 1.0f); // Red color for intersection point
    linal::vec3<double>& intersectionPoint = intersection.value();
    geoqik_add_point(intersectionPoint[0], intersectionPoint[1], intersectionPoint[2]);
  }

  geoqik_wait_for_exit_and_cleanup();

  return 0;
}
