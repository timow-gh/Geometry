#ifndef GEOMETRY_COMPILER_HPP
#define GEOMETRY_COMPILER_HPP

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_attribute(noreturn)
#define GEOMETRY_NORETURN __attribute__((noreturn))
#else
#define GEO_NORETURN
#endif

#if __has_attribute(packed)
#define GEO_PACKED __attribute__((packed))
#else
#define GEO_PACKED
#endif

#if __has_builtin(__builtin_expect)
#ifdef __cplusplus
#define GEO_LIKELY(exp) (__builtin_expect(!!(exp), true))
#define GEO_UNLIKELY(exp) (__builtin_expect(!!(exp), false))
#else
#define GEO_LIKELY(exp) (__builtin_expect(!!(exp), 1))
#define GEO_UNLIKELY(exp) (__builtin_expect(!!(exp), 0))
#endif
#else
#define GEO_LIKELY(exp) (!!(exp))
#define GEO_UNLIKELY(exp) (!!(exp))
#endif

#if __has_builtin(__builtin_prefetch)
#define GEO_PREFETCH(exp) (__builtin_prefetch(exp))
#else
#define GEO_PREFETCH(exp)
#endif

#if __has_builtin(__builtin_assume)
#define GEO_ASSUME(exp) (__builtin_assume(exp))
#else
#define UTILS_ASSUME(exp)
#endif

#if __has_attribute(always_inline)
#define GEO_ALWAYS_INLINE __attribute__((always_inline))
#else
#define GEO_ALWAYS_INLINE
#endif

#if __has_attribute(noinline)
#define GEO_NOINLINE __attribute__((noinline))
#else
#define GEO_NOINLINE
#endif

#if __has_attribute(maybe_unused)
#define GEO_UNUSED [[maybe_unused]]
#define GEO_UNUSED_IN_RELEASE [[maybe_unused]]
#elif __has_attribute(unused)
#define GEO_UNUSED __attribute__((unused))
#define GEO_UNUSED_IN_RELEASE __attribute__((unused))
#else
#define GEO_UNUSED
#define GEO_UNUSED_IN_RELEASE
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#define GEO_RESTRICT __restrict
#elif (defined(__clang__) || defined(__GNUC__))
#define GEO_RESTRICT __restrict__
#else
#define GEO_RESTRICT
#endif

#ifndef GEO_NODISCARD
#define GEO_NODISCARD [[nodiscard]]
#else
#define GEO_NODISCARD
#endif

#ifndef GEO_NOEXCEPT
#define GEO_NOEXCEPT noexcept
#else
#define GEO_NOEXCEPT
#endif

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 200704L) || defined(__cpp_constexpr))
#define GEO_CONSTEXPR constexpr
#else
#define GEO_CONSTEXPR
#endif

#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#endif // GEOMETRY_COMPILER_HPP
