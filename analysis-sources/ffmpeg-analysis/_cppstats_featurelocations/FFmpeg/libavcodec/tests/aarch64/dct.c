

















#include "config.h"

#include "libavcodec/aarch64/idct.h"

static const struct algo fdct_tab_arch[] = {
{ 0 }
};

static const struct algo idct_tab_arch[] = {
{ "SIMPLE-NEON", ff_simple_idct_neon, FF_IDCT_PERM_PARTTRANS, AV_CPU_FLAG_NEON },
{ 0 }
};
