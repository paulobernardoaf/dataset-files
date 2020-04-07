#include "config.h"
#include "libavcodec/ppc/fdct.h"
static const struct algo fdct_tab_arch[] = {
#if HAVE_ALTIVEC
{ "altivecfdct", ff_fdct_altivec, FF_IDCT_PERM_NONE, AV_CPU_FLAG_ALTIVEC },
#endif
{ 0 }
};
static const struct algo idct_tab_arch[] = {
{ 0 }
};
