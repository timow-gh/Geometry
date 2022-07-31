#ifndef GEOMETRY_DISTANCELINE_HPP
#define GEOMETRY_DISTANCELINE_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Line.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
CORE_NODISCARD CORE_CONSTEXPR T distance(const LinAl::Vec<T, D>& vec, const Line<T, D>& line)
{
    return LinAl::norm2(LinAl::rejection(LinAl::Vec<T, D>{line.getOrigin() - vec}, line.getDirection()));
}

template <typename T, std::size_t D>
CORE_NODISCARD CORE_CONSTEXPR T distance(const Line<T, D>& line, const LinAl::Vec<T, D>& vec)
{
    return distance(vec, line);
}

template <typename T, std::size_t D>
CORE_NODISCARD CORE_CONSTEXPR T distance(const Line<T, D>& lhs, const Line<T, D>& rhs)
{
    LinAl::Vec<T, D> cross = LinAl::cross(lhs.getDirection(), rhs.getDirection());
    T crossLen = LinAl::norm2(cross);
    if (Core::isZero(crossLen))
        return distance(lhs.getOrigin(), rhs);
    return LinAl::dot(LinAl::Vec<T, D>{lhs.getOrigin() - rhs.getOrigin()}, cross) / crossLen;
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCELINE_HPP
