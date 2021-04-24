#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "gtest/gtest.h"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/Ray.hpp"

using namespace Geometry;
using namespace LinAl;

TEST(Ray_Constructor, Default_Constructor) {
  Ray ray{LinAl::Vec3f{}, LinAl::X_VEC3F};
}

#pragma clang diagnostic pop