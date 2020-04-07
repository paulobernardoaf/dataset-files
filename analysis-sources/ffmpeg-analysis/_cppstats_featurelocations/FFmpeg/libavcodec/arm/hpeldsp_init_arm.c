




















#include "libavutil/arm/cpu.h"
#include "libavutil/attributes.h"
#include "libavcodec/pixels.h"
#include "hpeldsp_arm.h"

void ff_put_pixels8_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);
void ff_put_pixels8_x2_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);
void ff_put_pixels8_y2_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);
void ff_put_pixels8_xy2_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);

void ff_put_no_rnd_pixels8_x2_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);
void ff_put_no_rnd_pixels8_y2_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);
void ff_put_no_rnd_pixels8_xy2_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);

void ff_put_pixels16_arm(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);

CALL_2X_PIXELS(ff_put_pixels16_x2_arm, ff_put_pixels8_x2_arm, 8)
CALL_2X_PIXELS(ff_put_pixels16_y2_arm, ff_put_pixels8_y2_arm, 8)
CALL_2X_PIXELS(ff_put_pixels16_xy2_arm, ff_put_pixels8_xy2_arm, 8)
CALL_2X_PIXELS(ff_put_no_rnd_pixels16_x2_arm, ff_put_no_rnd_pixels8_x2_arm, 8)
CALL_2X_PIXELS(ff_put_no_rnd_pixels16_y2_arm, ff_put_no_rnd_pixels8_y2_arm, 8)
CALL_2X_PIXELS(ff_put_no_rnd_pixels16_xy2_arm, ff_put_no_rnd_pixels8_xy2_arm,8)

av_cold void ff_hpeldsp_init_arm(HpelDSPContext *c, int flags)
{
int cpu_flags = av_get_cpu_flags();

c->put_pixels_tab[0][0] = ff_put_pixels16_arm;
c->put_pixels_tab[0][1] = ff_put_pixels16_x2_arm;
c->put_pixels_tab[0][2] = ff_put_pixels16_y2_arm;
c->put_pixels_tab[0][3] = ff_put_pixels16_xy2_arm;
c->put_pixels_tab[1][0] = ff_put_pixels8_arm;
c->put_pixels_tab[1][1] = ff_put_pixels8_x2_arm;
c->put_pixels_tab[1][2] = ff_put_pixels8_y2_arm;
c->put_pixels_tab[1][3] = ff_put_pixels8_xy2_arm;

c->put_no_rnd_pixels_tab[0][0] = ff_put_pixels16_arm;
c->put_no_rnd_pixels_tab[0][1] = ff_put_no_rnd_pixels16_x2_arm;
c->put_no_rnd_pixels_tab[0][2] = ff_put_no_rnd_pixels16_y2_arm;
c->put_no_rnd_pixels_tab[0][3] = ff_put_no_rnd_pixels16_xy2_arm;
c->put_no_rnd_pixels_tab[1][0] = ff_put_pixels8_arm;
c->put_no_rnd_pixels_tab[1][1] = ff_put_no_rnd_pixels8_x2_arm;
c->put_no_rnd_pixels_tab[1][2] = ff_put_no_rnd_pixels8_y2_arm;
c->put_no_rnd_pixels_tab[1][3] = ff_put_no_rnd_pixels8_xy2_arm;

if (have_armv6(cpu_flags))
ff_hpeldsp_init_armv6(c, flags);
if (have_neon(cpu_flags))
ff_hpeldsp_init_neon(c, flags);
}
