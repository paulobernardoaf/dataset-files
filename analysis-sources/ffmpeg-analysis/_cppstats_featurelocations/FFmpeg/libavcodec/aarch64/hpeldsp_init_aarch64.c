




















#include <stddef.h>
#include <stdint.h>

#include "config.h"

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/aarch64/cpu.h"
#include "libavcodec/hpeldsp.h"

void ff_put_pixels16_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_x2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_y2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_xy2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_x2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_y2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_xy2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

void ff_put_pixels16_x2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_y2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_xy2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_x2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_y2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_xy2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

void ff_avg_pixels16_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels16_x2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels16_y2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels16_xy2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels8_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels8_x2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels8_y2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels8_xy2_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

void ff_avg_pixels16_x2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels16_y2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels16_xy2_no_rnd_neon(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

av_cold void ff_hpeldsp_init_aarch64(HpelDSPContext *c, int flags)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags)) {
c->put_pixels_tab[0][0] = ff_put_pixels16_neon;
c->put_pixels_tab[0][1] = ff_put_pixels16_x2_neon;
c->put_pixels_tab[0][2] = ff_put_pixels16_y2_neon;
c->put_pixels_tab[0][3] = ff_put_pixels16_xy2_neon;
c->put_pixels_tab[1][0] = ff_put_pixels8_neon;
c->put_pixels_tab[1][1] = ff_put_pixels8_x2_neon;
c->put_pixels_tab[1][2] = ff_put_pixels8_y2_neon;
c->put_pixels_tab[1][3] = ff_put_pixels8_xy2_neon;

c->put_no_rnd_pixels_tab[0][0] = ff_put_pixels16_neon;
c->put_no_rnd_pixels_tab[0][1] = ff_put_pixels16_x2_no_rnd_neon;
c->put_no_rnd_pixels_tab[0][2] = ff_put_pixels16_y2_no_rnd_neon;
c->put_no_rnd_pixels_tab[0][3] = ff_put_pixels16_xy2_no_rnd_neon;
c->put_no_rnd_pixels_tab[1][0] = ff_put_pixels8_neon;
c->put_no_rnd_pixels_tab[1][1] = ff_put_pixels8_x2_no_rnd_neon;
c->put_no_rnd_pixels_tab[1][2] = ff_put_pixels8_y2_no_rnd_neon;
c->put_no_rnd_pixels_tab[1][3] = ff_put_pixels8_xy2_no_rnd_neon;

c->avg_pixels_tab[0][0] = ff_avg_pixels16_neon;
c->avg_pixels_tab[0][1] = ff_avg_pixels16_x2_neon;
c->avg_pixels_tab[0][2] = ff_avg_pixels16_y2_neon;
c->avg_pixels_tab[0][3] = ff_avg_pixels16_xy2_neon;
c->avg_pixels_tab[1][0] = ff_avg_pixels8_neon;
c->avg_pixels_tab[1][1] = ff_avg_pixels8_x2_neon;
c->avg_pixels_tab[1][2] = ff_avg_pixels8_y2_neon;
c->avg_pixels_tab[1][3] = ff_avg_pixels8_xy2_neon;

c->avg_no_rnd_pixels_tab[0] = ff_avg_pixels16_neon;
c->avg_no_rnd_pixels_tab[1] = ff_avg_pixels16_x2_no_rnd_neon;
c->avg_no_rnd_pixels_tab[2] = ff_avg_pixels16_y2_no_rnd_neon;
c->avg_no_rnd_pixels_tab[3] = ff_avg_pixels16_xy2_no_rnd_neon;
}
}
