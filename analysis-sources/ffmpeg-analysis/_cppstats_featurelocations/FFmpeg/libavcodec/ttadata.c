



















#include "ttadata.h"

const uint32_t ff_tta_shift_1[] = {
0x00000001, 0x00000002, 0x00000004, 0x00000008,
0x00000010, 0x00000020, 0x00000040, 0x00000080,
0x00000100, 0x00000200, 0x00000400, 0x00000800,
0x00001000, 0x00002000, 0x00004000, 0x00008000,
0x00010000, 0x00020000, 0x00040000, 0x00080000,
0x00100000, 0x00200000, 0x00400000, 0x00800000,
0x01000000, 0x02000000, 0x04000000, 0x08000000,
0x10000000, 0x20000000, 0x40000000, 0x80000000,
0x80000000, 0x80000000, 0x80000000, 0x80000000,
0x80000000, 0x80000000, 0x80000000, 0x80000000
};

const uint32_t * const ff_tta_shift_16 = ff_tta_shift_1 + 4;

const uint8_t ff_tta_filter_configs[] = { 10, 9, 10, 12 };

void ff_tta_rice_init(TTARice *c, uint32_t k0, uint32_t k1)
{
c->k0 = k0;
c->k1 = k1;
c->sum0 = ff_tta_shift_16[k0];
c->sum1 = ff_tta_shift_16[k1];
}

void ff_tta_filter_init(TTAFilter *c, int32_t shift) {
memset(c, 0, sizeof(TTAFilter));
c->shift = shift;
c->round = ff_tta_shift_1[shift-1];
}
