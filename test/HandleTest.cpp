#include <Geometry/Handle.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(HandleTest, defaultConstructorCreatesInvalidHandle)
{
  Handle<unsigned int> const handle;

  EXPECT_FALSE(handle.is_valid());
  EXPECT_EQ(handle.get_value(), Handle<unsigned int>::invalidHandles);
}

TEST(HandleTest, explicitValueCreatesValidHandle)
{
  Handle<unsigned int> const handle{0U};

  EXPECT_TRUE(handle.is_valid());
  EXPECT_EQ(handle.get_value(), 0U);
}
