#include "libavutil/attributes.h"
#include "takdsp.h"
#include "config.h"
static void decorrelate_ls(int32_t *p1, int32_t *p2, int length)
{
int i;
for (i = 0; i < length; i++) {
int32_t a = p1[i];
int32_t b = p2[i];
p2[i] = a + b;
}
}
static void decorrelate_sr(int32_t *p1, int32_t *p2, int length)
{
int i;
for (i = 0; i < length; i++) {
int32_t a = p1[i];
int32_t b = p2[i];
p1[i] = b - a;
}
}
static void decorrelate_sm(int32_t *p1, int32_t *p2, int length)
{
int i;
for (i = 0; i < length; i++) {
int32_t a = p1[i];
int32_t b = p2[i];
a -= b >> 1;
p1[i] = a;
p2[i] = a + b;
}
}
static void decorrelate_sf(int32_t *p1, int32_t *p2, int length, int dshift, int dfactor)
{
int i;
for (i = 0; i < length; i++) {
int32_t a = p1[i];
int32_t b = p2[i];
b = dfactor * (b >> dshift) + 128 >> 8 << dshift;
p1[i] = b - a;
}
}
av_cold void ff_takdsp_init(TAKDSPContext *c)
{
c->decorrelate_ls = decorrelate_ls;
c->decorrelate_sr = decorrelate_sr;
c->decorrelate_sm = decorrelate_sm;
c->decorrelate_sf = decorrelate_sf;
if (ARCH_X86)
ff_takdsp_init_x86(c);
}
