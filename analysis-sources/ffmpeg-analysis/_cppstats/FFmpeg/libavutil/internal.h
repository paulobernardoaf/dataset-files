#if !defined(DEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "config.h"
#include "attributes.h"
#include "timer.h"
#include "cpu.h"
#include "dict.h"
#include "macros.h"
#include "mem.h"
#include "pixfmt.h"
#include "version.h"
#if ARCH_X86
#include "x86/emms.h"
#endif
#if !defined(emms_c)
#define emms_c() do {} while(0)
#endif
#if !defined(attribute_align_arg)
#if ARCH_X86_32 && AV_GCC_VERSION_AT_LEAST(4,2)
#define attribute_align_arg __attribute__((force_align_arg_pointer))
#else
#define attribute_align_arg
#endif
#endif
#if defined(_WIN32) && CONFIG_SHARED && !defined(BUILDING_avutil)
#define av_export_avutil __declspec(dllimport)
#else
#define av_export_avutil
#endif
#if HAVE_PRAGMA_DEPRECATED
#if defined(__ICL) || defined (__INTEL_COMPILER)
#define FF_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:1478))
#define FF_ENABLE_DEPRECATION_WARNINGS __pragma(warning(pop))
#elif defined(_MSC_VER)
#define FF_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:4996))
#define FF_ENABLE_DEPRECATION_WARNINGS __pragma(warning(pop))
#else
#define FF_DISABLE_DEPRECATION_WARNINGS _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define FF_ENABLE_DEPRECATION_WARNINGS _Pragma("GCC diagnostic pop")
#endif
#else
#define FF_DISABLE_DEPRECATION_WARNINGS
#define FF_ENABLE_DEPRECATION_WARNINGS
#endif
#define FF_MEMORY_POISON 0x2a
#define MAKE_ACCESSORS(str, name, type, field) type av_##name##_get_##field(const str *s) { return s->field; } void av_##name##_set_##field(str *s, type v) { s->field = v; }
#define E1(x) x
#define AV_CHECK_OFFSET(s, m, o) struct check_##o { int x_##o[offsetof(s, m) == o? 1: -1]; }
#define LOCAL_ALIGNED_A(a, t, v, s, o, ...) uint8_t la_##v[sizeof(t s o) + (a)]; t (*v) o = (void *)FFALIGN((uintptr_t)la_##v, a)
#define LOCAL_ALIGNED_D(a, t, v, s, o, ...) DECLARE_ALIGNED(a, t, la_##v) s o; t (*v) o = la_##v
#define LOCAL_ALIGNED(a, t, v, ...) LOCAL_ALIGNED_##a(t, v, __VA_ARGS__)
#if HAVE_LOCAL_ALIGNED
#define LOCAL_ALIGNED_4(t, v, ...) E1(LOCAL_ALIGNED_D(4, t, v, __VA_ARGS__,,))
#else
#define LOCAL_ALIGNED_4(t, v, ...) E1(LOCAL_ALIGNED_A(4, t, v, __VA_ARGS__,,))
#endif
#if HAVE_LOCAL_ALIGNED
#define LOCAL_ALIGNED_8(t, v, ...) E1(LOCAL_ALIGNED_D(8, t, v, __VA_ARGS__,,))
#else
#define LOCAL_ALIGNED_8(t, v, ...) E1(LOCAL_ALIGNED_A(8, t, v, __VA_ARGS__,,))
#endif
#if HAVE_LOCAL_ALIGNED
#define LOCAL_ALIGNED_16(t, v, ...) E1(LOCAL_ALIGNED_D(16, t, v, __VA_ARGS__,,))
#else
#define LOCAL_ALIGNED_16(t, v, ...) E1(LOCAL_ALIGNED_A(16, t, v, __VA_ARGS__,,))
#endif
#if HAVE_LOCAL_ALIGNED
#define LOCAL_ALIGNED_32(t, v, ...) E1(LOCAL_ALIGNED_D(32, t, v, __VA_ARGS__,,))
#else
#define LOCAL_ALIGNED_32(t, v, ...) E1(LOCAL_ALIGNED_A(32, t, v, __VA_ARGS__,,))
#endif
#define FF_ALLOC_OR_GOTO(ctx, p, size, label){p = av_malloc(size);if (!(p) && (size) != 0) {av_log(ctx, AV_LOG_ERROR, "Cannot allocate memory.\n");goto label;}}
#define FF_ALLOCZ_OR_GOTO(ctx, p, size, label){p = av_mallocz(size);if (!(p) && (size) != 0) {av_log(ctx, AV_LOG_ERROR, "Cannot allocate memory.\n");goto label;}}
#define FF_ALLOC_ARRAY_OR_GOTO(ctx, p, nelem, elsize, label){p = av_malloc_array(nelem, elsize);if (!p) {av_log(ctx, AV_LOG_ERROR, "Cannot allocate memory.\n");goto label;}}
#define FF_ALLOCZ_ARRAY_OR_GOTO(ctx, p, nelem, elsize, label){p = av_mallocz_array(nelem, elsize);if (!p) {av_log(ctx, AV_LOG_ERROR, "Cannot allocate memory.\n");goto label;}}
#include "libm.h"
#if CONFIG_SMALL
#define NULL_IF_CONFIG_SMALL(x) NULL
#else
#define NULL_IF_CONFIG_SMALL(x) x
#endif
#if HAVE_SYMVER_ASM_LABEL
#define FF_SYMVER(type, name, args, ver) type ff_##name args __asm__ (EXTERN_PREFIX #name "@" ver); type ff_##name args
#elif HAVE_SYMVER_GNU_ASM
#define FF_SYMVER(type, name, args, ver) __asm__ (".symver ff_" #name "," EXTERN_PREFIX #name "@" ver); type ff_##name args; type ff_##name args
#endif
#if HAVE_THREADS
#define ONLY_IF_THREADS_ENABLED(x) x
#else
#define ONLY_IF_THREADS_ENABLED(x) NULL
#endif
void avpriv_report_missing_feature(void *avc,
const char *msg, ...) av_printf_format(2, 3);
void avpriv_request_sample(void *avc,
const char *msg, ...) av_printf_format(2, 3);
#if HAVE_LIBC_MSVCRT
#include <crtversion.h>
#if defined(_VC_CRT_MAJOR_VERSION) && _VC_CRT_MAJOR_VERSION < 14
#pragma comment(linker, "/include:" EXTERN_PREFIX "avpriv_strtod")
#pragma comment(linker, "/include:" EXTERN_PREFIX "avpriv_snprintf")
#endif
#define avpriv_open ff_open
#define avpriv_tempfile ff_tempfile
#define PTRDIFF_SPECIFIER "Id"
#define SIZE_SPECIFIER "Iu"
#else
#define PTRDIFF_SPECIFIER "td"
#define SIZE_SPECIFIER "zu"
#endif
#if defined(DEBUG)
#define ff_dlog(ctx, ...) av_log(ctx, AV_LOG_DEBUG, __VA_ARGS__)
#else
#define ff_dlog(ctx, ...) do { if (0) av_log(ctx, AV_LOG_DEBUG, __VA_ARGS__); } while (0)
#endif
#if defined(CHECKED)
#define SUINT int
#define SUINT32 int32_t
#else
#define SUINT unsigned
#define SUINT32 uint32_t
#endif
static av_always_inline av_const int64_t ff_rint64_clip(double a, int64_t amin, int64_t amax)
{
int64_t res;
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
if (amin > amax) abort();
#endif
if (a >= 9223372036854775808.0)
return amax;
if (a <= -9223372036854775808.0)
return amin;
res = llrint(a);
if (res > amax)
return amax;
if (res < amin)
return amin;
return res;
}
av_warn_unused_result
int avpriv_open(const char *filename, int flags, ...);
int avpriv_tempfile(const char *prefix, char **filename, int log_offset, void *log_ctx);
int avpriv_set_systematic_pal2(uint32_t pal[256], enum AVPixelFormat pix_fmt);
static av_always_inline av_const int avpriv_mirror(int x, int w)
{
if (!w)
return 0;
while ((unsigned)x > (unsigned)w) {
x = -x;
if (x < 0)
x += 2 * w;
}
return x;
}
void ff_check_pixfmt_descriptors(void);
int avpriv_dict_set_timestamp(AVDictionary **dict, const char *key, int64_t timestamp);
#if FF_API_PSEUDOPAL
#define FF_PSEUDOPAL AV_PIX_FMT_FLAG_PSEUDOPAL
#else
#define FF_PSEUDOPAL 0
#endif
