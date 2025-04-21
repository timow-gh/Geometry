#ifndef GEOMETRY_INTERSECTTRIANGLE_HPP
#define GEOMETRY_INTERSECTTRIANGLE_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <Geometry/Triangle.hpp>
#include <linal/vec.hpp>

namespace Geometry
{

  namespace details
  {
  /**
   * @brief Projects points onto an axis and tests for overlap
   * 
   * @tparam T Floating point type
   * @param axis The axis to project onto
   * @param p1 First point of the first triangle (already at origin 0,0,0)
   * @param p2 Second point of the first triangle
   * @param p3 Third point of the first triangle
   * @param q1 First point of the second triangle
   * @param q2 Second point of the second triangle
   * @param q3 Third point of the second triangle
   * @return true if projections overlap, false otherwise (i.e., there is a separating axis)
   */
  template <typename T>
  GEO_NODISCARD constexpr bool is_separating_axis(
      const linal::vec3<T>& axis,
      const linal::vec3<T>& p1, 
      const linal::vec3<T>& p2, 
      const linal::vec3<T>& p3,
      const linal::vec3<T>& q1, 
      const linal::vec3<T>& q2, 
      const linal::vec3<T>& q3) noexcept
  {
      // Calculate the projections of the points onto the axis
      T P1 = linal::dot(axis, p1);
      T P2 = linal::dot(axis, p2);
      T P3 = linal::dot(axis, p3);
      T Q1 = linal::dot(axis, q1);
      T Q2 = linal::dot(axis, q2);
      T Q3 = linal::dot(axis, q3);
      
      // Find the min and max values for both triangles along the current test axis
      // This determines the "interval" that each triangle occupies when projected onto this axis
      T mx1 = std::max(P1, std::max(P2, P3));  // Maximum projection value for first triangle
      T mn1 = std::min(P1, std::min(P2, P3));  // Minimum projection value for first triangle
      T mx2 = std::max(Q1, std::max(Q2, Q3));  // Maximum projection value for second triangle
      T mn2 = std::min(Q1, std::min(Q2, Q3));  // Minimum projection value for second triangle
  
      // Check for separation: If the min of one is greater than the max of the other,
      // then we've found a separating axis and the triangles don't intersect
      if (mn1 > mx2) return false;  // First triangle is completely "after" second triangle along this axis
      if (mn2 > mx1) return false;  // Second triangle is completely "after" first triangle along this axis
      
      // No separation found on this axis - the intervals overlap
      return true;
  }
  
  /**
   * @brief Tests whether two triangles intersect using the Separating Axis Theorem.
   * 
   * The algorithm follows these key steps:
   *  - First establishes a new coordinate system with p1 at (0,0,0)
   *  - Computes edge vectors for both triangles (e1,e2,e3 and f1,f2,f3)
   *  - Computes normal vectors for both triangles (n1 and m1)
   *  - Computes a series of potential separating axes by taking cross products
   *    - Face normals
   *    - Edge-edge cross products
   *    - Additional vectors for testing
   *  - Tests 17 potential separating axes
   *    - 2 face normal directions
   *    - 9 edge-edge cross product directions
   *    - 6 additional directions from edge-normal products
   * - If no separating axis is found, the triangles must intersect
   * 
   * Might want to to consider implementing:
   *  - "A Fast Triangle-Triangle Intersection Test" by Tomas Möller (1997), Journal of Graphics Tools, Vol. 2, No. 2, pp. 25-30.
   * 
   * @tparam T Floating point type
   * @param P1 First point of the first triangle
   * @param P2 Second point of the first triangle
   * @param P3 Third point of the first triangle
   * @param Q1 First point of the second triangle
   * @param Q2 Second point of the second triangle
   * @param Q3 Third point of the second triangle
   * @return true if triangles intersect, false otherwise
   */
  template <typename T>
  GEO_NODISCARD constexpr bool TriangleTriangleSAT(
      const linal::vec3<T>& P1, 
      const linal::vec3<T>& P2, 
      const linal::vec3<T>& P3,
      const linal::vec3<T>& Q1, 
      const linal::vec3<T>& Q2, 
      const linal::vec3<T>& Q3) noexcept
  {
      // We establish a new coordinate system so that p1 is at (0,0,0).
      linal::vec3<T> p1{0, 0, 0}; // p1 is the origin in the new coordinate system
      linal::vec3<T> p2 = P2 - P1; 
      linal::vec3<T> p3 = P3 - P1;
      
      linal::vec3<T> q1 = Q1 - P1;
      linal::vec3<T> q2 = Q2 - P1;
      linal::vec3<T> q3 = Q3 - P1;
      
      // Calculate edge vectors for both triangles
      linal::vec3<T> e1 = p2 - p1;
      linal::vec3<T> e2 = p3 - p2;
      linal::vec3<T> e3 = p1 - p3;
      
      linal::vec3<T> f1 = q2 - q1;
      linal::vec3<T> f2 = q3 - q2;
      linal::vec3<T> f3 = q1 - q3;
      
      // Calculate normal vectors for both triangles
      linal::vec3<T> n1 = linal::cross(e1, e2);
      linal::vec3<T> m1 = linal::cross(f1, f2);
      
      // Calculate additional potential separating axes
      linal::vec3<T> g1 = linal::cross(e1, n1);
      linal::vec3<T> g2 = linal::cross(e2, n1);
      linal::vec3<T> g3 = linal::cross(e3, n1);
      linal::vec3<T> h1 = linal::cross(f1, m1);
      linal::vec3<T> h2 = linal::cross(f2, m1);
      linal::vec3<T> h3 = linal::cross(f3, m1);
      
      // Edge-edge cross products
      linal::vec3<T> ef11 = linal::cross(e1, f1);
      linal::vec3<T> ef12 = linal::cross(e1, f2);
      linal::vec3<T> ef13 = linal::cross(e1, f3);
      linal::vec3<T> ef21 = linal::cross(e2, f1);
      linal::vec3<T> ef22 = linal::cross(e2, f2);
      linal::vec3<T> ef23 = linal::cross(e2, f3);
      linal::vec3<T> ef31 = linal::cross(e3, f1);
      linal::vec3<T> ef32 = linal::cross(e3, f2);
      linal::vec3<T> ef33 = linal::cross(e3, f3);
      
      // Now test all 17 potential separating axes
      // If any axis separates the triangles, they don't intersect
      
      // Test face normal directions (2 axes)
      if (!details::is_separating_axis(n1, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(m1, p1, p2, p3, q1, q2, q3)) return false;
      
      // Test edge-edge cross products (9 axes)
      if (!details::is_separating_axis(ef11, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef12, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef13, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef21, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef22, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef23, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef31, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef32, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(ef33, p1, p2, p3, q1, q2, q3)) return false;
      
      // Test additional axes from edge-normal products (6 axes)
      if (!details::is_separating_axis(g1, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(g2, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(g3, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(h1, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(h2, p1, p2, p3, q1, q2, q3)) return false;
      if (!details::is_separating_axis(h3, p1, p2, p3, q1, q2, q3)) return false;
      
      // No separating axis found, the triangles must intersect
      return true;
  }

 } // namespace details

  /**
   * @brief Tests whether two triangles intersect.
   * 
   * @tparam T Floating point type
   * @param t1 First triangle
   * @param t2 Second triangle
   * @return true if triangles intersect, false otherwise
   */
  template <typename T>
  GEO_NODISCARD constexpr bool is_intersecting(const Triangle<T, 3>& t1, const Triangle<T, 3>& t2) noexcept
  {
      GEO_ASSERT(!t1.is_degenerate());
      GEO_ASSERT(!t2.is_degenerate());

      return details::TriangleTriangleSAT(t1[0], t1[1], t1[2], t2[0], t2[1], t2[2]);
  }

} // namespace Geometry

#endif