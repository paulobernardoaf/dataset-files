





















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/idctdsp.h"
#include "idct.h"

av_cold void ff_idctdsp_init_aarch64(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags) && !avctx->lowres && !high_bit_depth) {
if (avctx->idct_algo == FF_IDCT_AUTO ||
avctx->idct_algo == FF_IDCT_SIMPLEAUTO ||
avctx->idct_algo == FF_IDCT_SIMPLENEON) {
c->idct_put = ff_simple_idct_put_neon;
c->idct_add = ff_simple_idct_add_neon;
c->idct = ff_simple_idct_neon;
c->perm_type = FF_IDCT_PERM_PARTTRANS;
}
}
}
