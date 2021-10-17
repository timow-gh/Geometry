#ifndef MESH_HEADER
#define MESH_HEADER

#include "LinAl/LinearAlgebra.hpp"
#include <cstddef>
#include <iostream>
#include <vector>

namespace Geometry
{

template <typename T>
class MeshPoints
{
  public:
    MeshPoints() = default;

    [[nodiscard]] std::size_t add(const LinAl::Vec3<T>& vector);

    [[nodiscard]] LinAl::Vec3Vector<T>& getPoints();
    [[nodiscard]] const LinAl::Vec3Vector<T>& getPoints() const;
    void setPoints(const LinAl::Vec3Vector<T>& points);

    [[nodiscard]] bool contains(const LinAl::Vec3<T>& vector,
                                std::size_t& index);
    [[nodiscard]] LinAl::Vec3<T> getPoint(std::size_t index) const;

    [[nodiscard]] std::size_t size() const;

    void transform(const LinAl::Vec3<T>& translation);

  private:
    LinAl::Vec3Vector<T> m_points;
};

template <typename T>
std::size_t MeshPoints<T>::add(const LinAl::Vec3<T>& vector)
{
    m_points.push_back(vector);
    return m_points.size() - 1;
}

// TODO: Implement a kd-tree and check for existing points for large meshes
// with a lot of insertions

template <typename T>
bool MeshPoints<T>::contains(const LinAl::Vec3<T>& vector, std::size_t& index)
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

template <typename T>
LinAl::Vec3<T> MeshPoints<T>::getPoint(std::size_t index) const
{
    return m_points[index];
}
template <typename T>
std::size_t MeshPoints<T>::size() const
{
    return m_points.size();
}
template <typename T>
void MeshPoints<T>::transform(const LinAl::Vec3<T>& translation)
{
    for (auto& vec: m_points)
        vec += translation;
}
template <typename T>
const LinAl::Vec3Vector<T>& MeshPoints<T>::getPoints() const
{
    return m_points;
}
template <typename T>
LinAl::Vec3Vector<T>& MeshPoints<T>::getPoints()
{
    return m_points;
}
template <typename T>
void MeshPoints<T>::setPoints(const LinAl::Vec3Vector<T>& points)
{
    m_points = points;
}

template <typename T, typename U>
Core::TVector<U> vectorsToComponents(const LinAl::Vec3Vector<T>& vectors)
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