#include <stdio.h>
#include "libavutil/pixdesc.h"
#include "libavfilter/drawutils.h"
int main(void)
{
enum AVPixelFormat f;
const AVPixFmtDescriptor *desc;
FFDrawContext draw;
FFDrawColor color;
int r, i;
for (f = 0; av_pix_fmt_desc_get(f); f++) {
desc = av_pix_fmt_desc_get(f);
if (!desc->name)
continue;
printf("Testing %s...%*s", desc->name,
(int)(16 - strlen(desc->name)), "");
r = ff_draw_init(&draw, f, 0);
if (r < 0) {
char buf[128];
av_strerror(r, buf, sizeof(buf));
printf("no: %s\n", buf);
continue;
}
ff_draw_color(&draw, &color, (uint8_t[]) { 1, 0, 0, 1 });
for (i = 0; i < sizeof(color); i++)
if (((uint8_t *)&color)[i] != 128)
break;
if (i == sizeof(color)) {
printf("fallback color\n");
continue;
}
printf("ok\n");
}
return 0;
}
