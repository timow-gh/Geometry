#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <Geometry/Intersection/IntersectionSegment.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace Geometry;
using namespace LinAl;

TEST(Segment3dSegment3d, UniqueIntersection_Orthogonal)
{
    Segment3d segA{Vec3d{1, 1, 0}, X_VEC3D};
    Segment3d segB{Vec3d{1, 1, 0}, Y_VEC3D};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ((Vec3d{1, 1, 0}), intersection.getSource());
}

TEST(Segment3dSegment3d, UniqueIntersection_Collinear_Lhs)
{
    Segment3d segA{Vec3d{0, 0, 0}, X_VEC3D};
    Segment3d segB{Vec3d{-1, 0, 0}, Vec3d{0, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getSource());
}

TEST(Segment3dSegment3d, UniqueIntersection_Collinear_Rhs)
{
    Segment3d segA{Vec3d{0, 0, 0}, X_VEC3D};
    Segment3d segB{X_VEC3D, Vec3d{2, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ(X_VEC3D, intersection.getSource());
}

TEST(Segment3dSegment3d, Skew)
{
    Segment3d segA{Vec3d{0, 0, 0}, X_VEC3D};
    Segment3d segB{Vec3d{1, 1, 1}, Y_VEC3D};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 3);
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getTarget());
}

TEST(Segment3dSegment3d, Collinear_NoIntersection_Lhs)
{
    Segment3d segA{Vec3d{0, 0, 0}, Vec3d{1, 0, 0}};
    Segment3d segB{Vec3d{-2, 0, 0}, Vec3d{-1, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 0);
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getTarget());
}

TEST(Segment3dSegment3d, Collinear_NoIntersection_Rhs)
{
    Segment3d segA{Vec3d{0, 0, 0}, Vec3d{1, 0, 0}};
    Segment3d segB{Vec3d{2, 0, 0}, Vec3d{3, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 0);
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getTarget());
}

TEST(Segment3dSegment3d, Collinear_Overlapping_Lhs)
{
    Segment3d segA{Vec3d{0, 0, 0}, Vec3d{1, 0, 0}};
    Segment3d segB{Vec3d{-1, 0, 0}, Vec3d{0.5, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 2);
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0.5, 0, 0}), intersection.getTarget());
}

TEST(Segment3dSegment3d, Collinear_Overlapping_Rhs)
{
    Segment3d segA{Vec3d{0, 0, 0}, Vec3d{1, 0, 0}};
    Segment3d segB{Vec3d{0.5, 0, 0}, Vec3d{2.0, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 2);
    EXPECT_EQ((Vec3d{0.5, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{1, 0, 0}), intersection.getTarget());
}

TEST(Segment3dSegment3d, Collinear_Overlapping_Exact)
{
    Segment3d segA{Vec3d{0, 0, 0}, Vec3d{1, 0, 0}};
    Segment3d segB{Vec3d{0, 0, 0}, Vec3d{1, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 2);
    EXPECT_EQ(segA, intersection);
}

TEST(Segment3dSegment3d, Collinear_Overlapping_Inside)
{
    Segment3d segA{Vec3d{0, 0, 0}, Vec3d{4, 0, 0}};
    Segment3d segB{Vec3d{1, 0, 0}, Vec3d{2, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(segA, segB, intersection);
    EXPECT_EQ(res, 2);
    EXPECT_EQ(segB, intersection);
}

TEST(Segment3dLine3d, Skew)
{
    Segment3d seg{Vec3d{0, 0, 0}, Vec3d{3, 0, 0}};
    Line3d line{Vec3d{3, -2, 1}, Vec3d{0, 1, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(seg, line, intersection);
    EXPECT_EQ(res, 3);
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getTarget());
}

TEST(Segment3dLine3d, Unique_Intersection_Orthogonal)
{
    Segment3d seg{Vec3d{0, 0, 0}, Vec3d{3, 0, 0}};
    Line3d line{Vec3d{1, -2, 0}, Vec3d{0, 1, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(seg, line, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ((Vec3d{1, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getTarget());
}

TEST(Segment3dLine3d, Unique_Intersection_SegSource)
{
    Segment3d seg{Vec3d{0, 0, 0}, Vec3d{3, 0, 0}};
    Line3d line{Vec3d{0, -2, 0}, Vec3d{0, 1, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(seg, line, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getTarget());
}

TEST(Segment3dLine3d, Unique_Intersection_SegTarget)
{
    Segment3d seg{Vec3d{0, 0, 0}, Vec3d{3, 0, 0}};
    Line3d line{Vec3d{3, -2, 0}, Vec3d{0, 1, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(seg, line, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ((Vec3d{3, 0, 0}), intersection.getSource());
    EXPECT_EQ((Vec3d{0, 0, 0}), intersection.getTarget());
}

TEST(Segment3dLine3d, Collinear)
{
    Segment3d seg{Vec3d{0, 0, 0}, Vec3d{3, 0, 0}};
    Line3d line{Vec3d{1, 0, 0}, Vec3d{1, 0, 0}};
    Segment3d intersection{};
    uint32_t res = calcIntersection(seg, line, intersection);
    EXPECT_EQ(res, 2);
    EXPECT_EQ(seg, intersection);
}

#pragma clang diagnostic pop