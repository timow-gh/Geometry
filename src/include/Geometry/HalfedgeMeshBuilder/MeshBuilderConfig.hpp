#ifndef MESHLER_MESHBUILDERCONFIG_HPP
#define MESHLER_MESHBUILDERCONFIG_HPP

#include <cstdint>

namespace Geometry
{

struct MeshBuilderConfig
{
  std::uint32_t polarCount{10};
  std::uint32_t azimuthCount{20};
};

} // namespace Geometry

#endif // MESHLER_MESHBUILDERCONFIG_HPP
