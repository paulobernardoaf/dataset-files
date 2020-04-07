



















#include "g722dsp.h"
#include "mathops.h"






static void g722_apply_qmf(const int16_t *prev_samples, int xout[2])
{
xout[1] = MUL16(*prev_samples++, 3);
xout[0] = MUL16(*prev_samples++, -11);

MAC16(xout[1], *prev_samples++, -11);
MAC16(xout[0], *prev_samples++, 53);

MAC16(xout[1], *prev_samples++, 12);
MAC16(xout[0], *prev_samples++, -156);

MAC16(xout[1], *prev_samples++, 32);
MAC16(xout[0], *prev_samples++, 362);

MAC16(xout[1], *prev_samples++, -210);
MAC16(xout[0], *prev_samples++, -805);

MAC16(xout[1], *prev_samples++, 951);
MAC16(xout[0], *prev_samples++, 3876);

MAC16(xout[1], *prev_samples++, 3876);
MAC16(xout[0], *prev_samples++, 951);

MAC16(xout[1], *prev_samples++, -805);
MAC16(xout[0], *prev_samples++, -210);

MAC16(xout[1], *prev_samples++, 362);
MAC16(xout[0], *prev_samples++, 32);

MAC16(xout[1], *prev_samples++, -156);
MAC16(xout[0], *prev_samples++, 12);

MAC16(xout[1], *prev_samples++, 53);
MAC16(xout[0], *prev_samples++, -11);

MAC16(xout[1], *prev_samples++, -11);
MAC16(xout[0], *prev_samples++, 3);
}

av_cold void ff_g722dsp_init(G722DSPContext *c)
{
c->apply_qmf = g722_apply_qmf;

if (ARCH_ARM)
ff_g722dsp_init_arm(c);
if (ARCH_X86)
ff_g722dsp_init_x86(c);
}
