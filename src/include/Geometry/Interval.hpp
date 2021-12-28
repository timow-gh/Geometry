#ifndef GEOMETRY_INTERVAL_HPP
#define GEOMETRY_INTERVAL_HPP

#include <Core/Utils/Compiler.hpp>
#include <cstdint>

namespace Geometry
{
template <typename T>
class Interval {
    T m_start;
    T m_end;

  public:
    constexpr Interval() = default;
    constexpr Interval(T start, T end);

    CORE_NODISCARD T getStart() const;
    CORE_NODISCARD T getEnd() const;

    void setStart(T start);
    void setEnd(T end);

    //! return value:
    //! 0 -> no intersection
    //! 1 -> intersection at single point (start of intersection)
    //! 2 -> intersection is an interval
    CORE_NODISCARD uint32_t intersection(const Interval& interval,
                                         Interval& intersection) const;

    bool operator==(const Interval& rhs) const;
    bool operator!=(const Interval& rhs) const;
};
template <typename T>
constexpr Interval<T>::Interval(T start, T end) : m_start(start), m_end(end)
{
}
template <typename T>
T Interval<T>::getStart() const
{
    return m_start;
}
template <typename T>
T Interval<T>::getEnd() const
{
    return m_end;
}
template <typename T>
void Interval<T>::setStart(T start)
{
    m_start = start;
}
template <typename T>
void Interval<T>::setEnd(T end)
{
    m_end = end;
}
template <typename T>
uint32_t Interval<T>::intersection(const Interval& interval,
                                   Interval& intersection) const
{
    const T a = m_start;
    const T b = m_end;
    const T c = interval.getStart();
    const T d = interval.getEnd();

    // TODO Eps

    if (b < c || a > d)
        return 0;

    if (b > c)
    {
        if (a < d)
        {
            if (a < c)
                intersection.setStart(c);
            else
                intersection.setStart(a);
            if (b > d)
                intersection.setEnd(d);
            else
                intersection.setEnd(b);
            return 2;
        }
        else
        {
            // a == d
            intersection.setStart(a);
            return 1;
        }
    }
    else
    {
        // b = c
        intersection.setStart(b);
        return 1;
    }
}
template <typename T>
bool Interval<T>::operator==(const Interval& rhs) const
{
    return m_start == rhs.m_start && m_end == rhs.m_end;
}
template <typename T>
bool Interval<T>::operator!=(const Interval& rhs) const
{
    return !(rhs == *this);
}
} // namespace Geometry

#endif // GEOMETRY_INTERVAL_HPP
