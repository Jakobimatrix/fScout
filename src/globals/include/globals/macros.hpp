#pragma once


#include <stdio.h>

#include <cassert>
#include <string>

#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp& value) {
#if defined(__clang__)
  asm volatile("" : "+r,m"(value) : : "memory");
#else
  asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

#if defined(__clang__) || defined(__GNUC__)
#define CURRENT_FUNC __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define CURRENT_FUNC __FUNCSIG__
#else
#define CURRENT_FUNC __func__
#endif

#define RED(string) "\x1b[31m" string "\x1b[0m"
#define ORANGE(string) "\033[38:5:208:0m" string "\033[0m"

// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c
#define DEF_DEBUG(str) \
  fprintf(stderr, "[DEBUG]\t %s::%s()  Line: %d: %s\n", __FILE__, __func__, __LINE__, str)

#define DEF_F_DEBUG(fmt, ...)                                                                         \
  do {                                                                                                \
    fprintf(stderr, "[DEBUG]\t %s::%s()  Line: %d: " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__); \
  } while (0);                                                                                        \
  printf("\n")

#define DEF_WARNING(str) \
  fprintf(stderr, ORANGE("[WARN]") "\t %s::%s()  Line: %d: %s\n", __FILE__, __func__, __LINE__, str)

#define DEF_F_WARNING(fmt, ...)                                                                                 \
  do {                                                                                                          \
    fprintf(stderr, ORANGE("[WARN]") "\t %s::%s()  Line: %d: " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__); \
  } while (0);                                                                                                  \
  printf("\n")

#define DEF_ERROR(str) \
  fprintf(stderr, RED("[ERROR]") "\t %s::%s()  Line: %d: %s\n", __FILE__, __func__, __LINE__, str)

#define DEF_F_ERROR(fmt, ...)                                                                                 \
  do {                                                                                                        \
    fprintf(stderr, RED("[ERROR]") "\t %s::%s()  Line: %d: " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__); \
  } while (0);                                                                                                \
  printf("\n")

#define DEF_ASSERT(expr, str)                                                                            \
  if (!static_cast<bool>(expr)) {                                                                        \
    fprintf(stderr, RED("[ASSERT]") "   %s::%s()  Line: %d: %s\n\n", __FILE__, __func__, __LINE__, str); \
    assert(static_cast<bool>(expr) &&                                                                    \
           "This is a debug assert. See error message above!");                                          \
  }

#define DEF_F_ASSERT(expr, fmt, ...)                                                                             \
  if (!static_cast<bool>(expr)) {                                                                                \
    do {                                                                                                         \
      fprintf(stderr, RED("[ASSERT]") "   %s::%s()  Line: %d: " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__); \
    } while (0);                                                                                                 \
    printf("\n\n");                                                                                              \
    assert(static_cast<bool>(expr) &&                                                                            \
           "This is a debug assert. See error message above!");                                                  \
  }

#undef ERROR  // windof has this macro defined TODO instead rename my macro?

#ifdef NDEBUG

#define DEBUG(str)
#define F_DEBUG(fmt, ...)
#define WARNING(str)
#define F_WARNING(fmt, ...)
#define ERROR(str) DEF_ERROR(str)
#define F_ERROR(fmt, ...) DEF_F_ERROR(fmt, __VA_ARGS__)
#define ASSERT(expr, str) DEF_ERROR(str)
#define F_ASSERT(expr, fmt, ...) DEF_F_ERROR(fmt, __VA_ARGS__)

#else

#define DEBUG(str) DEF_DEBUG(str)
#define F_DEBUG(fmt, ...) DEF_F_DEBUG(fmt, __VA_ARGS__)
#define WARNING(str) DEF_ERROR(str)
#define F_WARNING(fmt, ...) DEF_F_ERROR(fmt, __VA_ARGS__)
#define ERROR(str) DEF_ASSERT(false, str)
#define F_ERROR(fmt, ...) DEF_F_ASSERT(false, fmt, __VA_ARGS__)
#define ASSERT(expr, str) DEF_ASSERT(expr, str)
#define F_ASSERT(expr, fmt, ...) DEF_F_ASSERT(expr, fmt, __VA_ARGS__)

#endif

// Compiler-specific macros for suppressing/enabling warnings

#if defined(__clang__)  // Clang compiler
#define DO_PRAGMA(X) _Pragma(#X)
#define SUPPRESS_WARNING(warning)  \
  DO_PRAGMA(clang diagnostic push) \
  DO_PRAGMA(clang diagnostic ignored warning)
#define ENABLE_WARNING() DO_PRAGMA(clang diagnostic pop)
#elif defined(__GNUC__)  // GCC compiler
#define DO_PRAGMA(X) _Pragma(#X)
#define SUPPRESS_WARNING(warning) \
  DO_PRAGMA(GCC diagnostic push)  \
  DO_PRAGMA(GCC diagnostic ignored warning)
#define ENABLE_WARNING() DO_PRAGMA(GCC diagnostic pop)
#else
#define SUPPRESS_WARNING(warning)  // Fallback if the compiler is unsupported
#define ENABLE_WARNING()
#endif
