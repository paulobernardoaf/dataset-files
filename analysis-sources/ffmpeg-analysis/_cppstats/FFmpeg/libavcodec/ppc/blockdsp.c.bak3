





















#include "config.h"

#include <string.h>

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"

#include "libavcodec/blockdsp.h"




















static void clear_blocks_dcbz32_ppc(int16_t *blocks)
{
register int misal = (unsigned long) blocks & 0x00000010, i = 0;

if (misal) {
((unsigned long *) blocks)[0] = 0L;
((unsigned long *) blocks)[1] = 0L;
((unsigned long *) blocks)[2] = 0L;
((unsigned long *) blocks)[3] = 0L;
i += 16;
}
for (; i < sizeof(int16_t) * 6 * 64 - 31; i += 32)
__asm__ volatile ("dcbz %0,%1" :: "b" (blocks), "r" (i) : "memory");
if (misal) {
((unsigned long *) blocks)[188] = 0L;
((unsigned long *) blocks)[189] = 0L;
((unsigned long *) blocks)[190] = 0L;
((unsigned long *) blocks)[191] = 0L;
i += 16;
}
}



static void clear_blocks_dcbz128_ppc(int16_t *blocks)
{
#if HAVE_DCBZL
register int misal = (unsigned long) blocks & 0x0000007f, i = 0;

if (misal) {



memset(blocks, 0, sizeof(int16_t) * 6 * 64);
} else {
for (; i < sizeof(int16_t) * 6 * 64; i += 128)
__asm__ volatile ("dcbzl %0,%1" :: "b" (blocks), "r" (i) : "memory");
}
#else
memset(blocks, 0, sizeof(int16_t) * 6 * 64);
#endif
}





static long check_dcbzl_effect(void)
{
long count = 0;
#if HAVE_DCBZL
register char *fakedata = av_malloc(1024);
register char *fakedata_middle;
register long zero = 0, i = 0;

if (!fakedata)
return 0L;

fakedata_middle = fakedata + 512;

memset(fakedata, 0xFF, 1024);



__asm__ volatile ("dcbzl %0, %1" :: "b" (fakedata_middle), "r" (zero));

for (i = 0; i < 1024; i++)
if (fakedata[i] == (char) 0)
count++;

av_free(fakedata);
#endif

return count;
}

#if HAVE_ALTIVEC
static void clear_block_altivec(int16_t *block)
{
LOAD_ZERO;
vec_st(zero_s16v, 0, block);
vec_st(zero_s16v, 16, block);
vec_st(zero_s16v, 32, block);
vec_st(zero_s16v, 48, block);
vec_st(zero_s16v, 64, block);
vec_st(zero_s16v, 80, block);
vec_st(zero_s16v, 96, block);
vec_st(zero_s16v, 112, block);
}
#endif 

av_cold void ff_blockdsp_init_ppc(BlockDSPContext *c)
{

switch (check_dcbzl_effect()) {
case 32:
c->clear_blocks = clear_blocks_dcbz32_ppc;
break;
case 128:
c->clear_blocks = clear_blocks_dcbz128_ppc;
break;
default:
break;
}

#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

c->clear_block = clear_block_altivec;
#endif 
}
