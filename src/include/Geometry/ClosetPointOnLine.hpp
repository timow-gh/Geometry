#ifndef GEOMETRY_CLOSETPOINTONLINE_HPP
#define GEOMETRY_CLOSETPOINTONLINE_HPP

#include "Geometry/Line.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <optional>

namespace Geometry::details
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
 *
 * @return The parameters that describe the points on the two lines that are closest to each other or std::nullopt if the lines are
 * parallel.
 */
template <typename T>
GEO_NODISCARD constexpr std::optional<CPOLParameters<T>> closest_point_on_line_parameters(linal::vec3<T> lhsSource,
                                                                                          linal::vec3<T> lhsDir,
                                                                                          linal::vec3<T> rhsSource,
                                                                                          linal::vec3<T> rhsDir,
                                                                                          T eps = linal::eps<T>::value) noexcept
{
  // Calculate the parameter values that minimize the distance between the two lines
  linal::vec3<T> w0 = lhsSource - rhsSource;
  T a = linal::dot(lhsDir, lhsDir);
  T b = linal::dot(lhsDir, rhsDir);
  T c = linal::dot(rhsDir, rhsDir);
  T d = linal::dot(lhsDir, w0);
  T e = linal::dot(rhsDir, w0);
  T denom = a * c - b * b;

  if (linal::isZero(denom, eps))
  {
    // The lines are parallel
    return {};
  }
  else
  {
    T t = (b * e - c * d) / denom;
    T s = (a * e - b * d) / denom;
    return CPOLParameters<T>{t, s};
  }
}

} // namespace Geometry::details

#endif // GEOMETRY_CLOSETPOINTONLINE_HPP
