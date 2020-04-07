#include "libavutil/avutil.h"
int ff_dirac_golomb_read_16bit(const uint8_t *buf, int bytes,
uint8_t *_dst, int coeffs);
int ff_dirac_golomb_read_32bit(const uint8_t *buf, int bytes,
uint8_t *_dst, int coeffs);
