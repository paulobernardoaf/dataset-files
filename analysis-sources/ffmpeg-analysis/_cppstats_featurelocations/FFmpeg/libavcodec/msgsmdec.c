




















#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "gsm.h"
#include "msgsmdec.h"

#include "gsmdec_template.c"

int ff_msgsm_decode_block(AVCodecContext *avctx, int16_t *samples,
const uint8_t *buf, int mode)
{
int res;
GetBitContext gb;
init_get_bits(&gb, buf, GSM_MS_BLOCK_SIZE * 8);
res = gsm_decode_block(avctx, samples, &gb, mode);
if (res < 0)
return res;
return gsm_decode_block(avctx, samples + GSM_FRAME_SIZE, &gb, mode);
}
