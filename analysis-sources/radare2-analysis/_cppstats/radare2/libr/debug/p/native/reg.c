static char *r_debug_native_reg_profile(RDebug *dbg) {
#if __WINDOWS__
if (dbg->bits & R_SYS_BITS_64) {
#include "reg/windows-x64.h"
} else {
#include "reg/windows-x86.h"
}
#elif (__OpenBSD__ || __NetBSD__)
#if __i386__
#include "reg/netbsd-x86.h"
#elif __x86_64__
#include "reg/netbsd-x64.h"
#else
#error "Unsupported BSD architecture"
#endif
#elif __KFBSD__ || __FreeBSD__
#if __i386__ || __i386
#include "reg/kfbsd-x86.h"
#elif __x86_64__ || __amd64__
#include "reg/kfbsd-x64.h"
#elif __aarch64__
#include "reg/kfbsd-arm64.h"
#else
#error "Unsupported BSD architecture"
#endif
#else
#warning Unsupported Kernel
return NULL;
#endif
}
