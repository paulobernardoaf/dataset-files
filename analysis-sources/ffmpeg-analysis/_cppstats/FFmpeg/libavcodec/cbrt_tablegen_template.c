#include <stdlib.h>
#define CONFIG_HARDCODED_TABLES 0
#include "libavutil/tablegen.h"
#include "cbrt_tablegen.h"
#include "tableprint.h"
int main(void)
{
AAC_RENAME(ff_cbrt_tableinit)();
write_fileheader();
#if USE_FIXED
WRITE_ARRAY("const", uint32_t, ff_cbrt_tab_fixed);
#else
WRITE_ARRAY("const", uint32_t, ff_cbrt_tab);
#endif
return 0;
}
