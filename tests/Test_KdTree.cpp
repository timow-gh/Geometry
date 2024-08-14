#include <Geometry/KdTree.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class TestKdTree : public ::testing::Test {
protected:
  void insert_three_points() {
    m_kdTree.insert(linal::double3{0.0, 0.0, 0.0});
    m_kdTree.insert(linal::double3{1.0, 1.0, 1.0});
    m_kdTree.insert(linal::double3{2.0, 2.0, 2.0});
  }

  KdTree<linal::double3> m_kdTree;
};

TEST_F(TestKdTree, empty) {
  EXPECT_EQ(m_kdTree.size(), 0);
  EXPECT_TRUE(m_kdTree.empty());

  bool res = m_kdTree.search(linal::double3{0.0, 0.0, 0.0});
  EXPECT_FALSE(res);

  try
  { [[maybe_unused]] linal::double3 nearest = m_kdTree.nearest(linal::double3{0.0, 0.0, 0.0}); }
  catch (std::runtime_error& e)
  { EXPECT_STREQ(e.what(), "KdTree is empty"); }
  catch (...)
  { FAIL(); }
}

TEST_F(TestKdTree, insert) {
  m_kdTree.insert(linal::double3{0.0, 0.0, 0.0});
  EXPECT_EQ(m_kdTree.size(), 1);
}

TEST_F(TestKdTree, insertSamePoint) {
  m_kdTree.insert(linal::double3{0.0, 0.0, 0.0});
  m_kdTree.insert(linal::double3{0.0, 0.0, 0.0});
  EXPECT_EQ(m_kdTree.size(), 1);
}

TEST_F(TestKdTree, insertMultiplePoints) {
  insert_three_points();
  EXPECT_EQ(m_kdTree.size(), 3);
}

TEST_F(TestKdTree, search_empty) {
  auto result = m_kdTree.search(linal::double3{0.0, 0.0, 0.0});
  EXPECT_FALSE(result);
}

TEST_F(TestKdTree, search_success) {
  insert_three_points();
  auto result = m_kdTree.search(linal::double3{1.0, 1.0, 1.0});
  EXPECT_TRUE(result);
}

TEST_F(TestKdTree, search_failure) {
  insert_three_points();
  auto result = m_kdTree.search(linal::double3{3.0, 3.0, 3.0});
  EXPECT_FALSE(result);
}

TEST_F(TestKdTree, nearest_success) {
  insert_three_points();
  auto nearest = m_kdTree.nearest(linal::double3{3.0, 3.0, 3.0});
  auto expected = linal::double3{2.0, 2.0, 2.0};
  EXPECT_EQ(nearest, expected);
}

TEST_F(TestKdTree, nearest_failure) {
  insert_three_points();
  try
  { [[maybe_unused]] auto nearest = m_kdTree.nearest(linal::double3{0.0, 0.0, 0.0}); }
  catch (std::runtime_error& e)
  { EXPECT_STREQ(e.what(), "KdTree is empty"); }
  catch (...)
  { FAIL(); }
}
