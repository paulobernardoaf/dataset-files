#include <stdint.h>
#include "libavutil/avassert.h"
#include "libavutil/lls.h"
#include "aac_defines.h"
#define ORDER_METHOD_EST 0
#define ORDER_METHOD_2LEVEL 1
#define ORDER_METHOD_4LEVEL 2
#define ORDER_METHOD_8LEVEL 3
#define ORDER_METHOD_SEARCH 4
#define ORDER_METHOD_LOG 5
#define MIN_LPC_ORDER 1
#define MAX_LPC_ORDER 32
enum FFLPCType {
FF_LPC_TYPE_DEFAULT = -1, 
FF_LPC_TYPE_NONE = 0, 
FF_LPC_TYPE_FIXED = 1, 
FF_LPC_TYPE_LEVINSON = 2, 
FF_LPC_TYPE_CHOLESKY = 3, 
FF_LPC_TYPE_NB , 
};
typedef struct LPCContext {
int blocksize;
int max_order;
enum FFLPCType lpc_type;
double *windowed_buffer;
double *windowed_samples;
void (*lpc_apply_welch_window)(const int32_t *data, int len,
double *w_data);
void (*lpc_compute_autocorr)(const double *data, int len, int lag,
double *autoc);
LLSModel lls_models[2];
} LPCContext;
int ff_lpc_calc_coefs(LPCContext *s,
const int32_t *samples, int blocksize, int min_order,
int max_order, int precision,
int32_t coefs[][MAX_LPC_ORDER], int *shift,
enum FFLPCType lpc_type, int lpc_passes,
int omethod, int min_shift, int max_shift, int zero_shift);
int ff_lpc_calc_ref_coefs(LPCContext *s,
const int32_t *samples, int order, double *ref);
double ff_lpc_calc_ref_coefs_f(LPCContext *s, const float *samples, int len,
int order, double *ref);
int ff_lpc_init(LPCContext *s, int blocksize, int max_order,
enum FFLPCType lpc_type);
void ff_lpc_init_x86(LPCContext *s);
void ff_lpc_end(LPCContext *s);
#if USE_FIXED
typedef int LPC_TYPE;
typedef unsigned LPC_TYPE_U;
#else
#if defined(LPC_USE_DOUBLE)
typedef double LPC_TYPE;
typedef double LPC_TYPE_U;
#else
typedef float LPC_TYPE;
typedef float LPC_TYPE_U;
#endif
#endif 
static inline void compute_ref_coefs(const LPC_TYPE *autoc, int max_order,
LPC_TYPE *ref, LPC_TYPE *error)
{
int i, j;
LPC_TYPE err;
LPC_TYPE gen0[MAX_LPC_ORDER], gen1[MAX_LPC_ORDER];
for (i = 0; i < max_order; i++)
gen0[i] = gen1[i] = autoc[i + 1];
err = autoc[0];
ref[0] = -gen1[0] / err;
err += gen1[0] * ref[0];
if (error)
error[0] = err;
for (i = 1; i < max_order; i++) {
for (j = 0; j < max_order - i; j++) {
gen1[j] = gen1[j + 1] + ref[i - 1] * gen0[j];
gen0[j] = gen1[j + 1] * ref[i - 1] + gen0[j];
}
ref[i] = -gen1[0] / err;
err += gen1[0] * ref[i];
if (error)
error[i] = err;
}
}
static inline int AAC_RENAME(compute_lpc_coefs)(const LPC_TYPE *autoc, int max_order,
LPC_TYPE *lpc, int lpc_stride, int fail,
int normalize)
{
int i, j;
LPC_TYPE err = 0;
LPC_TYPE *lpc_last = lpc;
av_assert2(normalize || !fail);
if (normalize)
err = *autoc++;
if (fail && (autoc[max_order - 1] == 0 || err <= 0))
return -1;
for(i=0; i<max_order; i++) {
LPC_TYPE r = AAC_SRA_R(-autoc[i], 5);
if (normalize) {
for(j=0; j<i; j++)
r -= lpc_last[j] * autoc[i-j-1];
r /= err;
err *= FIXR(1.0) - (r * r);
}
lpc[i] = r;
for(j=0; j < (i+1)>>1; j++) {
LPC_TYPE f = lpc_last[ j];
LPC_TYPE b = lpc_last[i-1-j];
lpc[ j] = f + (LPC_TYPE_U)AAC_MUL26(r, b);
lpc[i-1-j] = b + (LPC_TYPE_U)AAC_MUL26(r, f);
}
if (fail && err < 0)
return -1;
lpc_last = lpc;
lpc += lpc_stride;
}
return 0;
}
