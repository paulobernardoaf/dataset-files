#include "get_bits.h"
#include "mpegvideo.h"
#include "put_bits.h"
void ff_flv_encode_picture_header(MpegEncContext *s, int picture_number);
void ff_flv2_encode_ac_esc(PutBitContext *pb, int slevel, int level, int run,
int last);
int ff_flv_decode_picture_header(MpegEncContext *s);
