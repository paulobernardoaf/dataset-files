#include <limits.h>
#include <stdio.h>
#include "libavutil/internal.h"
#include "libavutil/lfg.h"
#include "libavutil/lls.h"
int main(void)
{
LLSModel m;
int i, order;
AVLFG lfg;
av_lfg_init(&lfg, 1);
avpriv_init_lls(&m, 3);
for (i = 0; i < 100; i++) {
LOCAL_ALIGNED(32, double, var, [4]);
double eval;
var[0] = (av_lfg_get(&lfg) / (double) UINT_MAX - 0.5) * 2;
var[1] = var[0] + av_lfg_get(&lfg) / (double) UINT_MAX - 0.5;
var[2] = var[1] + av_lfg_get(&lfg) / (double) UINT_MAX - 0.5;
var[3] = var[2] + av_lfg_get(&lfg) / (double) UINT_MAX - 0.5;
m.update_lls(&m, var);
avpriv_solve_lls(&m, 0.001, 0);
for (order = 0; order < 3; order++) {
eval = m.evaluate_lls(&m, var + 1, order);
printf("real:%9f order:%d pred:%9f var:%f coeffs:%f %9f %9f\n",
var[0], order, eval, sqrt(m.variance[order] / (i + 1)),
m.coeff[order][0], m.coeff[order][1],
m.coeff[order][2]);
}
}
return 0;
}
