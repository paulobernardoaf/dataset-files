#include "internal.h"
#include "libavutil/avstring.h"
#include <stdlib.h>
void ff_dvdsub_parse_palette(uint32_t *palette, const char *p)
{
for (int i = 0; i < 16; i++) {
palette[i] = strtoul(p, (char **)&p, 16);
while (*p == ',' || av_isspace(*p))
p++;
}
}
