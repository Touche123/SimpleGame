#define _USE_MATH_DEFINES
#define TO_RADIANS(deg) ((deg) * (3.14159265f / 180.0f))
// #include <cmath>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef uint64_t uptr;
typedef int64_t sptr;

#define CORE_GAME_NAME "Game W/O Engine"
#define CORE_GAME_VERSION 0

#ifdef _WIN32
#define CORE_OS_WINDOWS
#endif

#define CORE_ID_DEFINE(Name)  \
    typedef struct Name Name; \
    struct Name {             \
        u32 raw;              \
    }

#define CORE_ID_NULL 0

#ifdef __GNUC__
#define CORE_LIKELY(expr) __builtin_expect((expr), 1);
#define CORE_UNLIKELY(expr) __builtin_expect((expr), 0)
#else
#define CORE_LIKELY(expr) expr
#define CORE_UNLIKELY(expr) expr
#endif

#define CORE_UNUSED(expr) ((void)(expr))

#ifndef CORE_ENABLE_DEBUG_ASSERTIONS
#define CORE_ENABLE_DEBUG_ASSERTIONS
#endif

#ifndef CORE_ABORT
#define CORE_ABORT(...) core_abort(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef CORE_ASSERT
#define CORE_ASSERT(cond, ...) \
    if (CORE_UNLIKELY(!(cond))) core_assert_failed(#cond, __FILE__, __LINE__, __VA_ARGS__)
#endif

void core_assert_failed(const char* cond, const char* file, int line, const char* message, ...);
_Noreturn uintptr_t core_abort(const char* file, int line, const char* message, ...);

#define for_range(var, start, end) for (int var = (start); var < (end); var++)