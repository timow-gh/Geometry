#ifndef GLFWTESTAPP_RAY_HPP
#define GLFWTESTAPP_RAY_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <Geometry/Distance/DistanceRay.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
class Ray {
    LinAl::Vec<T, D> m_origin;
    LinAl::Vec<T, D> m_direction;

  public:
    CORE_CONSTEXPR Ray(const LinAl::Vec<T, D>& origin, const LinAl::Vec<T, D>& direction);

    CORE_CONSTEXPR bool operator==(const Ray& rhs) const;
    CORE_CONSTEXPR bool operator!=(const Ray& rhs) const;

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getOrigin() const;
    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec<T, D>& getDirection() const;

    CORE_NODISCARD CORE_CONSTEXPR T distance(const Ray<T, D>& ray,
                                             const LinAl::Vec<T, D>& vec) const;
    CORE_NODISCARD CORE_CONSTEXPR T distance(const LinAl::Vec<T, D>& vec) const;
};

template <typename T, std::size_t D>
CORE_CONSTEXPR Ray<T, D>::Ray(const LinAl::Vec<T, D>& origin, const LinAl::Vec<T, D>& direction)
    : m_origin(origin), m_direction(direction)
{
}

template <typename T, std::size_t D>
CORE_CONSTEXPR bool Ray<T, D>::operator==(const Ray& rhs) const
{
    return m_origin == rhs.m_origin && m_direction == rhs.m_direction;
}

template <typename T, std::size_t D>
CORE_CONSTEXPR bool Ray<T, D>::operator!=(const Ray& rhs) const
{
    return !(rhs == *this);
}

template <typename T, std::size_t D>
CORE_CONSTEXPR const LinAl::Vec<T, D>& Ray<T, D>::getOrigin() const
{
    return m_origin;
}

template <typename T, std::size_t D>
CORE_CONSTEXPR const LinAl::Vec<T, D>& Ray<T, D>::getDirection() const
{
    return m_direction;
}

template <typename T, std::size_t D>
CORE_CONSTEXPR T Ray<T, D>::distance(const Ray<T, D>& ray, const LinAl::Vec<T, D>& vec) const
{
    return Geometry::distance(ray, vec);
}

template <typename T, std::size_t D>
CORE_CONSTEXPR T Ray<T, D>::distance(const LinAl::Vec<T, D>& vec) const
{
    return Geometry::distance(*this, vec);
}

template <typename T>
using Ray2 = Ray<T, 2>;
using Ray2f = Ray2<float_t>;
using Ray2d = Ray2<double_t>;

template <typename T>
using Ray3 = Ray<T, 3>;
using Ray3f = Ray3<float_t>;
using Ray3d = Ray3<double_t>;
} // namespace Geometry

#endif // GLFWTESTAPP_RAY_HPP
