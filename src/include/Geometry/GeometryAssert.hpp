#ifndef GEOMETRY_GEOMETRYASSERT_HPP
#define GEOMETRY_GEOMETRYASSERT_HPP

#include <Core/Utils/Assert.hpp>

#define GEOMETRY_PRECONDITION_SPHERE_ASSERT(sphere) CORE_PRECONDITION_ASSERT(!Core::isZero(sphere.getRadius()), "Sphere with radius zero.")

#ifdef NDEBUG
#define GEOMETRY_PRECONDITION_SPHERE_DEBUG_ASSERT(sphere)
#else
#define GEOMETRY_PRECONDITION_SPHERE_DEBUG_ASSERT(sphere)                                                                                  \
  CORE_PRECONDITION_DEBUG_ASSERT(!Core::isZero(sphere.getRadius()), "Sphere with radius zero.")
#endif

#define GEOMETRY_PRECONDITION_RAY_DIRECTION_ASSERT(ray)                                                                                    \
  CORE_PRECONDITION_ASSERT(!Core::isZero(LinAl::norm2Squared(ray.getDirection())), "Direction with zero length is not allowed.")

#ifdef NDEBUG
#define GEOMETRY_PRECONDITION_RAY_DIRECTION_DEBUG_ASSERT(ray)
#else
#define GEOMETRY_PRECONDITION_RAY_DIRECTION_DEBUG_ASSERT(ray)                                                                              \
  CORE_PRECONDITION_DEBUG_ASSERT(!Core::isZero(LinAl::norm2Squared(ray.getDirection())), "Direction with zero length is not allowed.")
#endif

#endif // GEOMETRY_GEOMETRYASSERT_HPP
