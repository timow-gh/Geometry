#include <Geometry/KdTree.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class TestKdTreeMap : public ::testing::Test {
protected:
  void insert_three_key_value_pairs()
  {
    m_kdTree.insert(linal::double3{0.0, 0.0, 0.0}, 0u);
    m_kdTree.insert(linal::double3{1.0, 1.0, 1.0}, 1u);
    m_kdTree.insert(linal::double3{2.0, 2.0, 2.0}, 2u);
  }

  KdTree<linal::double3, int> m_kdTree;
};

TEST_F(TestKdTreeMap, empty)
{
  EXPECT_EQ(m_kdTree.size(), 0);
  EXPECT_TRUE(m_kdTree.empty());

  bool res = m_kdTree.search(linal::double3{0.0, 0.0, 0.0});
  EXPECT_FALSE(res);

  try
  {
    [[maybe_unused]] std::pair<linal::double3, int> nearest = m_kdTree.nearest(linal::double3{0.0, 0.0, 0.0});
  }
  catch (std::runtime_error& e)
  {
    EXPECT_STREQ(e.what(), "KdTree is empty");
  }
  catch (...)
  {
    FAIL();
  }
}

TEST_F(TestKdTreeMap, insert)
{
  m_kdTree.insert(linal::double3{0.0, 0.0, 0.0}, 0);
  EXPECT_EQ(m_kdTree.size(), 1);
}

TEST_F(TestKdTreeMap, insertSamePoint)
{
  m_kdTree.insert(linal::double3{0.0, 0.0, 0.0}, 0);
  m_kdTree.insert(linal::double3{0.0, 0.0, 0.0}, 1);
  EXPECT_EQ(m_kdTree.size(), 1);
}

TEST_F(TestKdTreeMap, insertMultiplePoints)
{
  insert_three_key_value_pairs();
  EXPECT_EQ(m_kdTree.size(), 3);
}

TEST_F(TestKdTreeMap, search_empty)
{
  auto result = m_kdTree.search(linal::double3{0.0, 0.0, 0.0});
  EXPECT_FALSE(result);
}

TEST_F(TestKdTreeMap, search_success)
{
  insert_three_key_value_pairs();
  auto result = m_kdTree.search(linal::double3{1.0, 1.0, 1.0});
  EXPECT_TRUE(result);
}

TEST_F(TestKdTreeMap, search_failure)
{
  insert_three_key_value_pairs();
  auto result = m_kdTree.search(linal::double3{3.0, 3.0, 3.0});
  EXPECT_FALSE(result);
}

TEST_F(TestKdTreeMap, nearest_success)
{
  insert_three_key_value_pairs();
  std::pair<linal::double3, int> nearest = m_kdTree.nearest(linal::double3{3.0, 3.0, 3.0});
  int anInt = 2;
  std::pair<linal::double3, int> expected = std::make_pair(linal::double3{2.0, 2.0, 2.0}, anInt);
  EXPECT_EQ(nearest, expected);
}

TEST_F(TestKdTreeMap, nearest_failure)
{
  insert_three_key_value_pairs();
  try
  {
    [[maybe_unused]] auto nearest = m_kdTree.nearest(linal::double3{0.0, 0.0, 0.0});
  }
  catch (std::runtime_error& e)
  {
    EXPECT_STREQ(e.what(), "KdTree is empty");
  }
  catch (...)
  {
    FAIL();
  }
}
