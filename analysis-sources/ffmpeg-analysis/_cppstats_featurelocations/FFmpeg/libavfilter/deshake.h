




















#if !defined(AVFILTER_DESHAKE_H)
#define AVFILTER_DESHAKE_H

#include "config.h"
#include "avfilter.h"
#include "transform.h"
#include "libavutil/pixelutils.h"


enum SearchMethod {
EXHAUSTIVE, 
SMART_EXHAUSTIVE, 
SEARCH_COUNT
};

typedef struct IntMotionVector {
int x; 
int y; 
} IntMotionVector;

typedef struct MotionVector {
double x; 
double y; 
} MotionVector;

typedef struct Transform {
MotionVector vec; 
double angle; 
double zoom; 
} Transform;

#define MAX_R 64

typedef struct DeshakeContext {
const AVClass *class;
int counts[2*MAX_R+1][2*MAX_R+1]; 
double *angles; 
unsigned angles_size;
AVFrame *ref; 
int rx; 
int ry; 
int edge; 
int blocksize; 
int contrast; 
int search; 
av_pixelutils_sad_fn sad; 
Transform last; 
int refcount; 
FILE *fp;
Transform avg;
int cw; 
int ch;
int cx;
int cy;
char *filename; 
int opencl;
int (* transform)(AVFilterContext *ctx, int width, int height, int cw, int ch,
const float *matrix_y, const float *matrix_uv, enum InterpolateMethod interpolate,
enum FillMethod fill, AVFrame *in, AVFrame *out);
} DeshakeContext;

#endif 
