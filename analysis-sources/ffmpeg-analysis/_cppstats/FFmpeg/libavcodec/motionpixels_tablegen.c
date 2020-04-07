#include <stdlib.h>
#define CONFIG_HARDCODED_TABLES 0
#define MAX_NEG_CROP 0
#define ff_crop_tab ((uint8_t *)NULL)
#include "motionpixels_tablegen.h"
#include "tableprint.h"
int main(void)
{
motionpixels_tableinit();
write_fileheader();
printf("static const YuvPixel mp_rgb_yuv_table[1 << 15] = {\n");
write_int8_t_2d_array(mp_rgb_yuv_table, 1 << 15, 3);
printf("};\n");
return 0;
}
