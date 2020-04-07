#define AVSC_USE_STDCALL 1
#define FRAME_ALIGN 64
#if defined(_M_AMD64) || defined(__x86_64)
#define X86_64
#elif defined(_M_IX86) || defined(__i386__)
#define X86_32
#else
#error Unsupported CPU architecture.
#endif
#if defined(_MSC_VER)
#define MSVC
#elif defined(__GNUC__)
#define GCC
#elif defined(__clang__)
#define CLANG
#else
#error Unsupported compiler.
#endif
#if defined(GCC)
#undef __forceinline
#define __forceinline inline
#endif
