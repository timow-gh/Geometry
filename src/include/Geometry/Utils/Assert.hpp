#ifndef GEOMETRY_ASSERT_HPP
#define GEOMETRY_ASSERT_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <cstdio>

// Check if we're on windows and have access to __debugbreak().
#if !defined(GEO_DEBUGBREAK)
#if defined(_WIN32) && defined(__has_include)
#if __has_include(<intrin.h>)
#define GEO_DEBUGBREAK 1
#endif
#endif
#endif

#if !defined(GEO_DEBUGBREAK)
#define GEO_DEBUGBREAK 0
#endif

// Check if we're on a compiler that supports __builtin_trap().
#if !defined(GEO_BUILTIN_TRAP)
#if defined(__GNUC__) || defined(__clang__)
#define GEO_BUILTIN_TRAP 1
#else
#define GEO_BUILTIN_TRAP 0
#endif
#endif

#if !defined(GEO_BUILTIN_TRAP)
#define GEO_BUILTIN_TRAP 0
#endif

#if GEO_DEBUGBREAK
#include <intrin.h> // Provides access to the __debugbreak() function on windows.
#define GEO_ASSERT_TRAP() ::__debugbreak()
#elif GEO_BUILTIN_TRAP
#define GEO_ASSERT_TRAP() __builtin_trap()
#else
#define GEO_ASSERT_TRAP() ::std::abort()
#endif

namespace Geometry
{
inline void assertion(const char* fileName, int line, const char* funcName, const char* message)
{
  std::fprintf(stderr, "%s:%d: internal check failed in '%s': '%s'\n", fileName, line, funcName, message);
}
} // namespace Geometry

#if defined(NDEBUG) && NDEBUG
#define GEO_ASSERT(...)
#else
#define GEO_ASSERT(...)                                                                                                                    \
  do                                                                                                                                       \
  {                                                                                                                                        \
    if (GEO_UNLIKELY(!(__VA_ARGS__)))                                                                                                      \
    {                                                                                                                                      \
      ::linal::assertion(__FILE__, __LINE__, __func__, #__VA_ARGS__);                                                                      \
      GEO_ASSERT_TRAP();                                                                                                                   \
    }                                                                                                                                      \
  }                                                                                                                                        \
  while (false)
#endif

#endif // GEOMETRY_ASSERT_HPP
