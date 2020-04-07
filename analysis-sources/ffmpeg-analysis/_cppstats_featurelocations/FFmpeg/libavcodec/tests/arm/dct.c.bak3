

















#include "config.h"

#include "libavcodec/arm/idct.h"

static const struct algo fdct_tab_arch[] = {
{ 0 }
};

static const struct algo idct_tab_arch[] = {
{ "SIMPLE-ARM", ff_simple_idct_arm, FF_IDCT_PERM_NONE },
{ "INT-ARM", ff_j_rev_dct_arm, FF_IDCT_PERM_LIBMPEG2 },
#if HAVE_ARMV5TE
{ "SIMPLE-ARMV5TE", ff_simple_idct_armv5te, FF_IDCT_PERM_NONE, AV_CPU_FLAG_ARMV5TE },
#endif
#if HAVE_ARMV6
{ "SIMPLE-ARMV6", ff_simple_idct_armv6, FF_IDCT_PERM_LIBMPEG2, AV_CPU_FLAG_ARMV6 },
#endif
#if HAVE_NEON
{ "SIMPLE-NEON", ff_simple_idct_neon, FF_IDCT_PERM_PARTTRANS, AV_CPU_FLAG_NEON },
#endif
{ 0 }
};
