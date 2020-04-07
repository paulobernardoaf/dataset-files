#include "avfilter.h"
typedef struct FlipContext {
const AVClass *class;
int max_step[4]; 
int planewidth[4]; 
int planeheight[4]; 
void (*flip_line[4])(const uint8_t *src, uint8_t *dst, int w);
} FlipContext;
int ff_hflip_init(FlipContext *s, int step[4], int nb_planes);
void ff_hflip_init_x86(FlipContext *s, int step[4], int nb_planes);
