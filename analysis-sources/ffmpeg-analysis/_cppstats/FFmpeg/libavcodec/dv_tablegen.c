#include <stdlib.h>
#define CONFIG_HARDCODED_TABLES 0
#include "dv_tablegen.h"
#include "tableprint.h"
#include <inttypes.h>
WRITE_1D_FUNC_ARGV(dv_vlc_pair, 7,
"{0x%"PRIx32", %"PRIu32"}", data[i].vlc, data[i].size)
WRITE_2D_FUNC(dv_vlc_pair)
int main(void)
{
dv_vlc_map_tableinit();
write_fileheader();
printf("static const struct dv_vlc_pair dv_vlc_map[DV_VLC_MAP_RUN_SIZE][DV_VLC_MAP_LEV_SIZE] = {\n");
write_dv_vlc_pair_2d_array(dv_vlc_map, DV_VLC_MAP_RUN_SIZE, DV_VLC_MAP_LEV_SIZE);
printf("};\n");
return 0;
}
