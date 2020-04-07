#if defined(USE_CONSOLE)
#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_USE_CONSOLE
#endif
#include "allegro5/platform/alplatf.h"
#if defined ALLEGRO_WATCOM
#include "allegro5/platform/alwatcom.h"
#elif defined ALLEGRO_MINGW32
#include "allegro5/platform/almngw32.h"
#elif defined ALLEGRO_BCC32
#include "allegro5/platform/albcc32.h"
#elif defined ALLEGRO_MSVC
#include "allegro5/platform/almsvc.h"
#elif defined ALLEGRO_IPHONE
#include "allegro5/platform/aliphonecfg.h"
#elif defined ALLEGRO_MACOSX
#include "allegro5/platform/alosxcfg.h"
#elif defined ALLEGRO_ANDROID
#include "allegro5/platform/alandroidcfg.h"
#elif defined ALLEGRO_RASPBERRYPI
#include "allegro5/platform/alraspberrypicfg.h"
#elif defined ALLEGRO_UNIX
#include "allegro5/platform/alucfg.h"
#elif defined ALLEGRO_SDL
#include "allegro5/platform/allegro_sdl_config.h"
#else
#error platform not supported
#endif
#include "allegro5/platform/astdint.h"
#include "allegro5/platform/astdbool.h"
#if defined(__GNUC__)
#define ALLEGRO_GCC
#if !defined(AL_INLINE)
#if defined(__cplusplus)
#define AL_INLINE(type, name, args, code) static inline type name args; static inline type name args code
#elif __GNUC_STDC_INLINE__
#define AL_INLINE(type, name, args, code) extern __inline__ __attribute__((__gnu_inline__)) type name args; extern __inline__ __attribute__((__gnu_inline__)) type name args code
#else
#define AL_INLINE(type, name, args, code) extern __inline__ type name args; extern __inline__ type name args code
#endif
#endif
#if !defined(AL_INLINE_STATIC)
#if defined(__cplusplus)
#define AL_INLINE_STATIC(type, name, args, code) AL_INLINE(type, name, args, code)
#else
#define AL_INLINE_STATIC(type, name, args, code) static __inline__ type name args; static __inline__ type name args code
#endif
#endif
#define AL_PRINTFUNC(type, name, args, a, b) AL_FUNC(type, name, args) __attribute__ ((format (printf, a, b)))
#if !defined(INLINE)
#define INLINE __inline__
#endif
#if !defined(ZERO_SIZE_ARRAY)
#if __GNUC__ < 3
#define ZERO_SIZE_ARRAY(type, name) __extension__ type name[0]
#else
#define ZERO_SIZE_ARRAY(type, name) type name[] 
#endif
#endif
#if defined(ALLEGRO_GUESS_INTTYPES_OK)
#define int64_t signed long long
#define uint64_t unsigned long long
#endif
#if defined(__i386__)
#define ALLEGRO_I386
#endif
#if defined(__amd64__)
#define ALLEGRO_AMD64
#endif
#if defined(__arm__)
#define ALLEGRO_ARM
#endif
#if !defined(AL_FUNC_DEPRECATED)
#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define AL_FUNC_DEPRECATED(type, name, args) AL_FUNC(__attribute__ ((deprecated)) type, name, args)
#define AL_PRINTFUNC_DEPRECATED(type, name, args, a, b) AL_PRINTFUNC(__attribute__ ((deprecated)) type, name, args, a, b)
#define AL_INLINE_DEPRECATED(type, name, args, code) AL_INLINE(__attribute__ ((deprecated)) type, name, args, code)
#endif
#endif
#if !defined(AL_ALIAS)
#define AL_ALIAS(DECL, CALL) static __attribute__((unused)) __inline__ DECL { return CALL; }
#endif
#if !defined(AL_ALIAS_VOID_RET)
#define AL_ALIAS_VOID_RET(DECL, CALL) static __attribute__((unused)) __inline__ void DECL { CALL; }
#endif
#endif
#if !defined(INLINE)
#define INLINE
#endif
#if !defined(ZERO_SIZE_ARRAY)
#define ZERO_SIZE_ARRAY(type, name) type name[]
#endif
#if !defined(AL_VAR)
#define AL_VAR(type, name) extern type name
#endif
#if !defined(AL_ARRAY)
#define AL_ARRAY(type, name) extern type name[]
#endif
#if !defined(AL_FUNC)
#define AL_FUNC(type, name, args) type name args
#endif
#if !defined(AL_PRINTFUNC)
#define AL_PRINTFUNC(type, name, args, a, b) AL_FUNC(type, name, args)
#endif
#if !defined(AL_METHOD)
#define AL_METHOD(type, name, args) type (*name) args
#endif
#if !defined(AL_FUNCPTR)
#define AL_FUNCPTR(type, name, args) extern type (*name) args
#endif
#if !defined(AL_FUNCPTRARRAY)
#define AL_FUNCPTRARRAY(type, name, args) extern type (*name[]) args
#endif
#if !defined(AL_INLINE)
#define AL_INLINE(type, name, args, code) type name args;
#endif
#if !defined(AL_FUNC_DEPRECATED)
#define AL_FUNC_DEPRECATED(type, name, args) AL_FUNC(type, name, args)
#define AL_PRINTFUNC_DEPRECATED(type, name, args, a, b) AL_PRINTFUNC(type, name, args, a, b)
#define AL_INLINE_DEPRECATED(type, name, args, code) AL_INLINE(type, name, args, code)
#endif
#if !defined(AL_ALIAS)
#define AL_ALIAS(DECL, CALL) static INLINE DECL { return CALL; }
#endif
#if !defined(AL_ALIAS_VOID_RET)
#define AL_ALIAS_VOID_RET(DECL, CALL) static INLINE void DECL { CALL; }
#endif
#if defined(__cplusplus)
extern "C" {
#endif
#if defined(ALLEGRO_LITTLE_ENDIAN)
#define _AL_READ3BYTES(p) ((*(unsigned char *)(p)) | (*((unsigned char *)(p) + 1) << 8) | (*((unsigned char *)(p) + 2) << 16))
#define _AL_WRITE3BYTES(p,c) ((*(unsigned char *)(p) = (c)), (*((unsigned char *)(p) + 1) = (c) >> 8), (*((unsigned char *)(p) + 2) = (c) >> 16))
#elif defined ALLEGRO_BIG_ENDIAN
#define _AL_READ3BYTES(p) ((*(unsigned char *)(p) << 16) | (*((unsigned char *)(p) + 1) << 8) | (*((unsigned char *)(p) + 2)))
#define _AL_WRITE3BYTES(p,c) ((*(unsigned char *)(p) = (c) >> 16), (*((unsigned char *)(p) + 1) = (c) >> 8), (*((unsigned char *)(p) + 2) = (c)))
#else
#error endianess not defined
#endif
#if defined(__cplusplus)
}
#endif
