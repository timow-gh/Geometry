#ifndef FILAPP_CIRCLE_HPP
#define FILAPP_CIRCLE_HPP

#include <Core/Math/Eps.hpp>
#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class Circle2 {
    LinAl::Vec2<T> m_origin;
    T m_radius;

  public:
    CORE_CONSTEXPR Circle2(const LinAl::Vec2<T>& origin, T radius)
        : m_origin(origin), m_radius(radius)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec2<T>& getOrigin() const
    {
        return m_origin;
    }

    CORE_NODISCARD CORE_CONSTEXPR T getRadius() const
    {
        return m_radius;
    }
};

using Circle2f = Circle2<float_t>;
using Circle2d = Circle2<double_t>;

template <typename T>
class Circle3 {
    LinAl::Vec3<T> m_origin;
    T m_radius;
    LinAl::Vec3<T> m_normal;

  public:
    CORE_CONSTEXPR Circle3(const LinAl::Vec3<T>& origin, T radius, const LinAl::Vec3<T>& normal)
        : m_origin(origin), m_radius(radius), m_normal(normal)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getOrigin() const
    {
        return m_origin;
    }

    CORE_NODISCARD CORE_CONSTEXPR T getRadius() const
    {
        return m_radius;
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3<T>& getNormal() const
    {
        return m_normal;
    }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::HMatrix<T> calcTransformation() const
    {
        LinAl::HMatrix<T> transformation =
            LinAl::rotationAlign(LinAl::HVec<T>{0, 0, 1, 1},
                                 LinAl::HVec<T>{m_normal[0], m_normal[1], m_normal[2], 1});
        LinAl::setTranslation(transformation, m_origin);
        return transformation;
    }
};

template <typename T>
Circle3<T> transformation(const Circle3<T>& circle, const LinAl::HMatrix<T>& transformation)
{
    LinAl::HVec<T> origin = transformation * LinAl::vec3ToHVec(circle.getOrigin());
    return Circle3<T>{LinAl::hVecToVec3(origin), circle.getRadius(), circle.getNormal()};
}

using Circle3f = Circle3<float_t>;
using Circle3d = Circle3<double_t>;

} // namespace Geometry

#endif // FILAPP_CIRCLE_HPP
