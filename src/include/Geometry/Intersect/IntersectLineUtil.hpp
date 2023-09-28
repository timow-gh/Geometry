#ifndef GEOMETRY_INTERSECTLINEUTIL_HPP
#define GEOMETRY_INTERSECTLINEUTIL_HPP

#include "Geometry/Line.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/vec3.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

/** @brief The parameters describe the points on two 3d lines that are closest to each other.
 * 
 * CPOL stands for Closest Point On Line.
 */
template <typename T>
struct CPOLParameters
{
  T t{}; /**< Parameter t for the first line. */
  T s{}; /**< Parameter s for the second line. */
};

/** @brief Calculate the closest point parameters of two 3d lines.
 *
 * The parameters have the values 0 for the source point and 1 for the target point.
 *
 * @param lhsSource The source point of the first line.
 * @param lhsDir The direction vector of the first line, not necessarily normalized.
 * @param rhsSource The source point of the second line.
 * @param rhsDir The direction vector of the second line, not necessarily normalized.
 */
template <typename T>
GEO_NODISCARD constexpr CPOLParameters<T>
closest_point_on_line_parameters(linal::vec3<T> lhsSource, linal::vec3<T> lhsDir, linal::vec3<T> rhsSource, linal::vec3<T> rhsDir) noexcept
{
  // Calculate the parameter values that minimize the distance between the two lines
  linal::vec3<T> w0 = lhsSource - rhsSource;
  T a = linal::dot(lhsDir, lhsDir);
  T b = linal::dot(lhsDir, rhsDir);
  T c = linal::dot(rhsDir, rhsDir);
  T d = linal::dot(lhsDir, w0);
  T e = linal::dot(rhsDir, w0);
  T denom = a * c - b * b;
  T t = (b * e - c * d) / denom;
  T s = (a * e - b * d) / denom;
  return {t, s};
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTLINEUTIL_HPP
