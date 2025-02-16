#include <stdint.h>
#include "libavutil/attributes.h"
#include "dvdata.h"
#if CONFIG_SMALL
#define DV_VLC_MAP_RUN_SIZE 15
#define DV_VLC_MAP_LEV_SIZE 23
#else
#define DV_VLC_MAP_RUN_SIZE 64
#define DV_VLC_MAP_LEV_SIZE 512 
#endif
typedef struct dv_vlc_pair {
uint32_t vlc;
uint32_t size;
} dv_vlc_pair;
#if CONFIG_HARDCODED_TABLES
#define dv_vlc_map_tableinit()
#include "libavcodec/dv_tables.h"
#else
static struct dv_vlc_pair dv_vlc_map[DV_VLC_MAP_RUN_SIZE][DV_VLC_MAP_LEV_SIZE];
static av_cold void dv_vlc_map_tableinit(void)
{
int i, j;
for (i = 0; i < NB_DV_VLC - 1; i++) {
if (ff_dv_vlc_run[i] >= DV_VLC_MAP_RUN_SIZE)
continue;
#if CONFIG_SMALL
if (ff_dv_vlc_level[i] >= DV_VLC_MAP_LEV_SIZE)
continue;
#endif
if (dv_vlc_map[ff_dv_vlc_run[i]][ff_dv_vlc_level[i]].size != 0)
continue;
dv_vlc_map[ff_dv_vlc_run[i]][ff_dv_vlc_level[i]].vlc =
ff_dv_vlc_bits[i] << (!!ff_dv_vlc_level[i]);
dv_vlc_map[ff_dv_vlc_run[i]][ff_dv_vlc_level[i]].size =
ff_dv_vlc_len[i] + (!!ff_dv_vlc_level[i]);
}
for (i = 0; i < DV_VLC_MAP_RUN_SIZE; i++) {
#if CONFIG_SMALL
for (j = 1; j < DV_VLC_MAP_LEV_SIZE; j++) {
if (dv_vlc_map[i][j].size == 0) {
dv_vlc_map[i][j].vlc = dv_vlc_map[0][j].vlc |
(dv_vlc_map[i - 1][0].vlc <<
dv_vlc_map[0][j].size);
dv_vlc_map[i][j].size = dv_vlc_map[i - 1][0].size +
dv_vlc_map[0][j].size;
}
}
#else
for (j = 1; j < DV_VLC_MAP_LEV_SIZE / 2; j++) {
if (dv_vlc_map[i][j].size == 0) {
dv_vlc_map[i][j].vlc = dv_vlc_map[0][j].vlc |
(dv_vlc_map[i - 1][0].vlc <<
dv_vlc_map[0][j].size);
dv_vlc_map[i][j].size = dv_vlc_map[i - 1][0].size +
dv_vlc_map[0][j].size;
}
dv_vlc_map[i][((uint16_t) (-j)) & 0x1ff].vlc = dv_vlc_map[i][j].vlc | 1;
dv_vlc_map[i][((uint16_t) (-j)) & 0x1ff].size = dv_vlc_map[i][j].size;
}
#endif
}
}
#endif 
