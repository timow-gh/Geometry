#ifndef GEOMETRY_POLYLINE_H
#define GEOMETRY_POLYLINE_H

#include <Geometry/Utils/Compiler.hpp>
#include <linal/Containers.hpp>
#include <linal/Vec3.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class PolyLine {
  linal::VecVector<T, D> m_points;

public:
  GEO_CONSTEXPR PolyLine(const linal::VecVector<T, D>& mPoints)
      : m_points(mPoints)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const linal::VecVector<T, D>& getPoints() const { return m_points; }
};

template <typename T>
using PolyLine2 = PolyLine<T, 2>;
using PolyLine2f = PolyLine2<float>;
using PolyLine2d = PolyLine2<double>;

template <typename T>
using PolyLine3 = PolyLine<T, 3>;
using PolyLine3f = PolyLine3<float>;
using PolyLine3d = PolyLine3<double>;

} // namespace Geometry

#endif // GEOMETRY_POLYLINE_H
