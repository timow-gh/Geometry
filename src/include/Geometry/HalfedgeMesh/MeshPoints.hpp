#ifndef MESH_HEADER
#define MESH_HEADER

#include <Core/Utils/Compiler.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <cstddef>
#include <iostream>
#include <vector>

namespace Geometry
{

template <typename T>
class MeshPoints {
  public:
    MeshPoints() = default;

    // TODO: Implement a kd-tree and check for existing points for large meshes
    // with a lot of insertions
    CORE_NODISCARD CORE_CONSTEXPR std::size_t add(const LinAl::Vec3<T>& vector)
    {
        m_points.push_back(vector);
        return m_points.size() - 1;
    }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3Vector<T>& getPoints()
    {
        return m_points;
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3Vector<T>& getPoints() const
    {
        return m_points;
    }

    CORE_CONSTEXPR void setPoints(const LinAl::Vec3Vector<T>& points)
    {
        m_points = points;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const LinAl::Vec3<T>& vector, std::size_t& index)
    {
        for (std::size_t i = 0; i < m_points.size(); ++i)
        {
            if (m_points[i] == vector)
            {
                index = i;
                return true;
            }
        }
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3<T> getPoint(std::size_t index) const
    {
        return m_points[index];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t size() const
    {
        return m_points.size();
    }

    CORE_CONSTEXPR void transform(const LinAl::Vec3<T>& translation)
    {
        for (auto& vec: m_points)
            vec += translation;
    }

  private:
    LinAl::Vec3Vector<T> m_points;
};

template <typename T, typename U>
CORE_CONSTEXPR Core::TVector<U> vectorsToComponents(const LinAl::Vec3Vector<T>& vectors)
{
    Core::TVector<U> result;
    for (const LinAl::Vec3<T>& vector: vectors)
        for (std::size_t i{0}; i < 3; ++i)
        {
            result.push_back(static_cast<U>(vector[i]));
        }
    return result;
}

} // namespace Geometry

#endif