#include <stdlib.h>
#include "libavcodec/aac_defines.h"
#define CONFIG_HARDCODED_TABLES 0
#if USE_FIXED
#define WRITE_FUNC write_int32_t_array
#else
#define WRITE_FUNC write_float_array
#endif
#define SINETABLE_CONST
#define SINETABLE(size) INTFLOAT AAC_RENAME(ff_sine_##size)[size]
#define SINETABLE120960(size) INTFLOAT AAC_RENAME(ff_sine_##size)[size]
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#include "sinewin_tablegen.h"
#include "tableprint.h"
int main(void)
{
int i;
write_fileheader();
for (i = 5; i <= 13; i++) {
AAC_RENAME(ff_init_ff_sine_windows)(i);
printf("SINETABLE(%4i) = {\n", 1 << i);
WRITE_FUNC(AAC_RENAME(ff_sine_windows)[i], 1 << i);
printf("};\n");
}
return 0;
}
