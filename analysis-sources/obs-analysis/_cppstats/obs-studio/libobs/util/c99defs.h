#pragma once
#define UNUSED_PARAMETER(param) (void)param
#if defined(_MSC_VER)
#define OBS_DEPRECATED __declspec(deprecated)
#define FORCE_INLINE __forceinline
#else
#define OBS_DEPRECATED __attribute__((deprecated))
#define FORCE_INLINE inline __attribute__((always_inline))
#endif
#if defined(_MSC_VER)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
