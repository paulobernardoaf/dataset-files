





















#if !defined(AVFILTER_MEDIAN_H)
#define AVFILTER_MEDIAN_H

#include "avfilter.h"

typedef struct MedianContext {
const AVClass *class;

int planes;
int radius;
int radiusV;
float percentile;

int planewidth[4];
int planeheight[4];
int depth;
int nb_planes;
int nb_threads;

uint16_t **coarse, **fine;
int coarse_size, fine_size;
int bins;
int t;

void (*hadd)(uint16_t *dst, const uint16_t *src, int bins);
void (*hsub)(uint16_t *dst, const uint16_t *src, int bins);
void (*hmuladd)(uint16_t *dst, const uint16_t *src, int f, int bins);

void (*filter_plane)(AVFilterContext *ctx, const uint8_t *ssrc, int src_linesize,
uint8_t *ddst, int dst_linesize, int width, int height,
int slice_h_start, int slice_h_end, int jobnr);
} MedianContext;

#endif
