#include "libavcodec/wmv2dsp.h"
void ff_wmv2_idct_add_mmi(uint8_t *dest, int line_size, int16_t *block);
void ff_wmv2_idct_put_mmi(uint8_t *dest, int line_size, int16_t *block);
