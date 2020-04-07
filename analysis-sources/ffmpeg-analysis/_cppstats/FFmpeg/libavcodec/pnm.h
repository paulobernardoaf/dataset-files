#include "avcodec.h"
typedef struct PNMContext {
uint8_t *bytestream;
uint8_t *bytestream_start;
uint8_t *bytestream_end;
int maxval; 
int type;
} PNMContext;
int ff_pnm_decode_header(AVCodecContext *avctx, PNMContext * const s);
