#ifndef GEOMETRY_GEOMETRYASSERT_HPP
#define GEOMETRY_GEOMETRYASSERT_HPP

#include <Geometry/Utils/Assert.hpp>

#ifdef NDEBUG
#define GEOMETRY_SPHERE_ASSERT(sphere)
#else
#define GEOMETRY_SPHERE_ASSERT(sphere) GEOMETRY_ASSERT(!linal::isZero(sphere.getRadius()))
#endif

#ifdef NDEBUG
#define GEOMETRY_RAY_DIRECTION_ASSERT(ray)
#else
#define GEOMETRY_RAY_DIRECTION_ASSERT(ray) GEOMETRY_ASSERT(!linal::isZero(linal::norm2Squared(ray.getDirection())))
#endif

#endif // GEOMETRY_GEOMETRYASSERT_HPP
