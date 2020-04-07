

















#include "libavutil/avassert.h"
#include "libavutil/mathematics.h"
#include "libavutil/attributes.h"
#include "kbdwin.h"

#define BESSEL_I0_ITER 50 

av_cold void ff_kbd_window_init(float *window, float alpha, int n)
{
int i, j;
double sum = 0.0, bessel, tmp;
double local_window[FF_KBD_WINDOW_MAX];
double alpha2 = (alpha * M_PI / n) * (alpha * M_PI / n);

av_assert0(n <= FF_KBD_WINDOW_MAX);

for (i = 0; i < n; i++) {
tmp = i * (n - i) * alpha2;
bessel = 1.0;
for (j = BESSEL_I0_ITER; j > 0; j--)
bessel = bessel * tmp / (j * j) + 1;
sum += bessel;
local_window[i] = sum;
}

sum++;
for (i = 0; i < n; i++)
window[i] = sqrt(local_window[i] / sum);
}

av_cold void ff_kbd_window_init_fixed(int32_t *window, float alpha, int n)
{
int i;
float local_window[FF_KBD_WINDOW_MAX];

ff_kbd_window_init(local_window, alpha, n);
for (i = 0; i < n; i++)
window[i] = (int)floor(2147483647.0 * local_window[i] + 0.5);
}
