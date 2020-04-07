#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "libavutil/bswap.h"
#define storeneonregs(mem) __asm__ volatile( "vstm %0, {d8-d15}\n\t" :: "r"(mem) : "memory")
#define testneonclobbers(func, ctx, ...) uint64_t neon[2][8]; int ret; storeneonregs(neon[0]); ret = __real_ ##func(ctx, __VA_ARGS__); storeneonregs(neon[1]); if (memcmp(neon[0], neon[1], sizeof(neon[0]))) { int i; av_log(ctx, AV_LOG_ERROR, "NEON REGS CLOBBERED IN %s!\n", #func); for (i = 0; i < 8; i ++) if (neon[0][i] != neon[1][i]) { av_log(ctx, AV_LOG_ERROR, "d%-2d = %016"PRIx64"\n", 8 + i, av_bswap64(neon[0][i])); av_log(ctx, AV_LOG_ERROR, " -> %016"PRIx64"\n", av_bswap64(neon[1][i])); } abort(); } return ret
#define wrap(func) int __real_ ##func; int __wrap_ ##func; int __wrap_ ##func
