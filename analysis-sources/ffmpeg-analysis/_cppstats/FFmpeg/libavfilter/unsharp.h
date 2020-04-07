#include "config.h"
#include "avfilter.h"
#define MIN_MATRIX_SIZE 3
#define MAX_MATRIX_SIZE 63
typedef struct UnsharpFilterParam {
int msize_x; 
int msize_y; 
int amount; 
int steps_x; 
int steps_y; 
int scalebits; 
int32_t halfscale; 
uint32_t *sr; 
uint32_t **sc; 
} UnsharpFilterParam;
typedef struct UnsharpContext {
const AVClass *class;
int lmsize_x, lmsize_y, cmsize_x, cmsize_y;
float lamount, camount;
UnsharpFilterParam luma; 
UnsharpFilterParam chroma; 
int hsub, vsub;
int nb_threads;
int opencl;
int (* apply_unsharp)(AVFilterContext *ctx, AVFrame *in, AVFrame *out);
} UnsharpContext;
