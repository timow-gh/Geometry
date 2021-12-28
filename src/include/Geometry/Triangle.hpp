#ifndef GEOMETRY_TRIANGLE_HPP
#define GEOMETRY_TRIANGLE_HPP

#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{
template <typename T, std::size_t D>
class Triangle {
    LinAl::VecArray<T, D, 3> m_trianglePoints;

  public:
    CORE_CONSTEXPR Triangle() = default;
    CORE_CONSTEXPR explicit Triangle(const LinAl::VecArray<T, D, 3>& trianglePoints);
    CORE_CONSTEXPR Triangle(const LinAl::Vec<T, D>& first,
                            const LinAl::Vec<T, D>& second,
                            const LinAl::Vec<T, D>& third);

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::VecArray<T, D, 3>& getTrianglePoints() const;
};
template <typename T, std::size_t D>
CORE_CONSTEXPR Triangle<T, D>::Triangle(const LinAl::VecArray<T, D, 3>& trianglePoints)
    : m_trianglePoints(trianglePoints)
{
}

template <typename T, std::size_t D>
CORE_CONSTEXPR Triangle<T, D>::Triangle(const LinAl::Vec<T, D>& first,
                                        const LinAl::Vec<T, D>& second,
                                        const LinAl::Vec<T, D>& third)
    : m_trianglePoints({first, second, third})
{
}

template <typename T, std::size_t D>
CORE_CONSTEXPR const LinAl::VecArray<T, D, 3>& Triangle<T, D>::getTrianglePoints() const
{
    return m_trianglePoints;
}
} // namespace Geometry

#endif // GEOMETRY_TRIANGLE_HPP
