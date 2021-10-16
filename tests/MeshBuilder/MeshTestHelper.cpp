#include "MeshTestHelper.h"

namespace MeshTestHelper
{
testing::AssertionResult isValidIndex(std::size_t index)
{
    if (index != Geometry::INVALID_INDEX)
        return testing::AssertionSuccess();
    else
        return testing::AssertionFailure() << "Invalid index.";
}
} // namespace MeshTestHelper