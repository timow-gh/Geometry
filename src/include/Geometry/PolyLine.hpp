#ifndef GEOMETRY_POLYLINE_H
#define GEOMETRY_POLYLINE_H

#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class PolyLine {
    LinAl::VecVector<T, D> m_points;

  public:
    CORE_CONSTEXPR PolyLine(const LinAl::VecVector<T, D>& mPoints) : m_points(mPoints)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::VecVector<T, D>& getPoints() const
    {
        return m_points;
    }
};

template <typename T>
using PolyLine2 = PolyLine<T, 2>;
using PolyLine2f = PolyLine2<float_t>;
using PolyLine2d = PolyLine2<double_t>;

template <typename T>
using PolyLine3 = PolyLine<T, 3>;
using PolyLine3f = PolyLine3<float_t>;
using PolyLine3d = PolyLine3<double_t>;

} // namespace Geometry

#endif // GEOMETRY_POLYLINE_H
