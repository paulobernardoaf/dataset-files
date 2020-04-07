




















#include "libavutil/common.h"
#include "libavutil/lls.h"

#define LPC_USE_DOUBLE
#include "lpc.h"
#include "libavutil/avassert.h"





static void lpc_apply_welch_window_c(const int32_t *data, int len,
double *w_data)
{
int i, n2;
double w;
double c;

n2 = (len >> 1);
c = 2.0 / (len - 1.0);

if (len & 1) {
for(i=0; i<n2; i++) {
w = c - i - 1.0;
w = 1.0 - (w * w);
w_data[i] = data[i] * w;
w_data[len-1-i] = data[len-1-i] * w;
}
return;
}

w_data+=n2;
data+=n2;
for(i=0; i<n2; i++) {
w = c - n2 + i;
w = 1.0 - (w * w);
w_data[-i-1] = data[-i-1] * w;
w_data[+i ] = data[+i ] * w;
}
}





static void lpc_compute_autocorr_c(const double *data, int len, int lag,
double *autoc)
{
int i, j;

for(j=0; j<lag; j+=2){
double sum0 = 1.0, sum1 = 1.0;
for(i=j; i<len; i++){
sum0 += data[i] * data[i-j];
sum1 += data[i] * data[i-j-1];
}
autoc[j ] = sum0;
autoc[j+1] = sum1;
}

if(j==lag){
double sum = 1.0;
for(i=j-1; i<len; i+=2){
sum += data[i ] * data[i-j ]
+ data[i+1] * data[i-j+1];
}
autoc[j] = sum;
}
}




static void quantize_lpc_coefs(double *lpc_in, int order, int precision,
int32_t *lpc_out, int *shift, int min_shift,
int max_shift, int zero_shift)
{
int i;
double cmax, error;
int32_t qmax;
int sh;


qmax = (1 << (precision - 1)) - 1;


cmax = 0.0;
for(i=0; i<order; i++) {
cmax= FFMAX(cmax, fabs(lpc_in[i]));
}


if(cmax * (1 << max_shift) < 1.0) {
*shift = zero_shift;
memset(lpc_out, 0, sizeof(int32_t) * order);
return;
}


sh = max_shift;
while((cmax * (1 << sh) > qmax) && (sh > min_shift)) {
sh--;
}



if(sh == 0 && cmax > qmax) {
double scale = ((double)qmax) / cmax;
for(i=0; i<order; i++) {
lpc_in[i] *= scale;
}
}


error=0;
for(i=0; i<order; i++) {
error -= lpc_in[i] * (1 << sh);
lpc_out[i] = av_clip(lrintf(error), -qmax, qmax);
error -= lpc_out[i];
}
*shift = sh;
}

static int estimate_best_order(double *ref, int min_order, int max_order)
{
int i, est;

est = min_order;
for(i=max_order-1; i>=min_order-1; i--) {
if(ref[i] > 0.10) {
est = i+1;
break;
}
}
return est;
}

int ff_lpc_calc_ref_coefs(LPCContext *s,
const int32_t *samples, int order, double *ref)
{
double autoc[MAX_LPC_ORDER + 1];

s->lpc_apply_welch_window(samples, s->blocksize, s->windowed_samples);
s->lpc_compute_autocorr(s->windowed_samples, s->blocksize, order, autoc);
compute_ref_coefs(autoc, order, ref, NULL);

return order;
}

double ff_lpc_calc_ref_coefs_f(LPCContext *s, const float *samples, int len,
int order, double *ref)
{
int i;
double signal = 0.0f, avg_err = 0.0f;
double autoc[MAX_LPC_ORDER+1] = {0}, error[MAX_LPC_ORDER+1] = {0};
const double a = 0.5f, b = 1.0f - a;


for (i = 0; i <= len / 2; i++) {
double weight = a - b*cos((2*M_PI*i)/(len - 1));
s->windowed_samples[i] = weight*samples[i];
s->windowed_samples[len-1-i] = weight*samples[len-1-i];
}

s->lpc_compute_autocorr(s->windowed_samples, len, order, autoc);
signal = autoc[0];
compute_ref_coefs(autoc, order, ref, error);
for (i = 0; i < order; i++)
avg_err = (avg_err + error[i])/2.0f;
return signal/avg_err;
}







int ff_lpc_calc_coefs(LPCContext *s,
const int32_t *samples, int blocksize, int min_order,
int max_order, int precision,
int32_t coefs[][MAX_LPC_ORDER], int *shift,
enum FFLPCType lpc_type, int lpc_passes,
int omethod, int min_shift, int max_shift, int zero_shift)
{
double autoc[MAX_LPC_ORDER+1];
double ref[MAX_LPC_ORDER] = { 0 };
double lpc[MAX_LPC_ORDER][MAX_LPC_ORDER];
int i, j, pass = 0;
int opt_order;

av_assert2(max_order >= MIN_LPC_ORDER && max_order <= MAX_LPC_ORDER &&
lpc_type > FF_LPC_TYPE_FIXED);
av_assert0(lpc_type == FF_LPC_TYPE_CHOLESKY || lpc_type == FF_LPC_TYPE_LEVINSON);


if (blocksize != s->blocksize || max_order != s->max_order ||
lpc_type != s->lpc_type) {
ff_lpc_end(s);
ff_lpc_init(s, blocksize, max_order, lpc_type);
}

if(lpc_passes <= 0)
lpc_passes = 2;

if (lpc_type == FF_LPC_TYPE_LEVINSON || (lpc_type == FF_LPC_TYPE_CHOLESKY && lpc_passes > 1)) {
s->lpc_apply_welch_window(samples, blocksize, s->windowed_samples);

s->lpc_compute_autocorr(s->windowed_samples, blocksize, max_order, autoc);

compute_lpc_coefs(autoc, max_order, &lpc[0][0], MAX_LPC_ORDER, 0, 1);

for(i=0; i<max_order; i++)
ref[i] = fabs(lpc[i][i]);

pass++;
}

if (lpc_type == FF_LPC_TYPE_CHOLESKY) {
LLSModel *m = s->lls_models;
LOCAL_ALIGNED(32, double, var, [FFALIGN(MAX_LPC_ORDER+1,4)]);
double av_uninit(weight);
memset(var, 0, FFALIGN(MAX_LPC_ORDER+1,4)*sizeof(*var));

for(j=0; j<max_order; j++)
m[0].coeff[max_order-1][j] = -lpc[max_order-1][j];

for(; pass<lpc_passes; pass++){
avpriv_init_lls(&m[pass&1], max_order);

weight=0;
for(i=max_order; i<blocksize; i++){
for(j=0; j<=max_order; j++)
var[j]= samples[i-j];

if(pass){
double eval, inv, rinv;
eval= m[pass&1].evaluate_lls(&m[(pass-1)&1], var+1, max_order-1);
eval= (512>>pass) + fabs(eval - var[0]);
inv = 1/eval;
rinv = sqrt(inv);
for(j=0; j<=max_order; j++)
var[j] *= rinv;
weight += inv;
}else
weight++;

m[pass&1].update_lls(&m[pass&1], var);
}
avpriv_solve_lls(&m[pass&1], 0.001, 0);
}

for(i=0; i<max_order; i++){
for(j=0; j<max_order; j++)
lpc[i][j]=-m[(pass-1)&1].coeff[i][j];
ref[i]= sqrt(m[(pass-1)&1].variance[i] / weight) * (blocksize - max_order) / 4000;
}
for(i=max_order-1; i>0; i--)
ref[i] = ref[i-1] - ref[i];
}

opt_order = max_order;

if(omethod == ORDER_METHOD_EST) {
opt_order = estimate_best_order(ref, min_order, max_order);
i = opt_order-1;
quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i],
min_shift, max_shift, zero_shift);
} else {
for(i=min_order-1; i<max_order; i++) {
quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i],
min_shift, max_shift, zero_shift);
}
}

return opt_order;
}

av_cold int ff_lpc_init(LPCContext *s, int blocksize, int max_order,
enum FFLPCType lpc_type)
{
s->blocksize = blocksize;
s->max_order = max_order;
s->lpc_type = lpc_type;

s->windowed_buffer = av_mallocz((blocksize + 2 + FFALIGN(max_order, 4)) *
sizeof(*s->windowed_samples));
if (!s->windowed_buffer)
return AVERROR(ENOMEM);
s->windowed_samples = s->windowed_buffer + FFALIGN(max_order, 4);

s->lpc_apply_welch_window = lpc_apply_welch_window_c;
s->lpc_compute_autocorr = lpc_compute_autocorr_c;

if (ARCH_X86)
ff_lpc_init_x86(s);

return 0;
}

av_cold void ff_lpc_end(LPCContext *s)
{
av_freep(&s->windowed_buffer);
}
