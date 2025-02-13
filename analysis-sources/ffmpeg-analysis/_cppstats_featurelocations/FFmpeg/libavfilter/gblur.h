


























#if !defined(AVFILTER_GBLUR_H)
#define AVFILTER_GBLUR_H
#include "avfilter.h"

typedef struct GBlurContext {
const AVClass *class;

float sigma;
float sigmaV;
int steps;
int planes;

int depth;
int planewidth[4];
int planeheight[4];
float *buffer;
float boundaryscale;
float boundaryscaleV;
float postscale;
float postscaleV;
float nu;
float nuV;
int nb_planes;
void (*horiz_slice)(float *buffer, int width, int height, int steps, float nu, float bscale);
} GBlurContext;
void ff_gblur_init(GBlurContext *s);
void ff_gblur_init_x86(GBlurContext *s);
#endif
