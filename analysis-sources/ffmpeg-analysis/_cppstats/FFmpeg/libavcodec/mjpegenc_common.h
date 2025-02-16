#include <stdint.h>
#include "avcodec.h"
#include "idctdsp.h"
#include "mpegvideo.h"
#include "put_bits.h"
void ff_mjpeg_encode_picture_header(AVCodecContext *avctx, PutBitContext *pb,
ScanTable *intra_scantable, int pred,
uint16_t luma_intra_matrix[64],
uint16_t chroma_intra_matrix[64]);
void ff_mjpeg_encode_picture_frame(MpegEncContext *s);
void ff_mjpeg_encode_picture_trailer(PutBitContext *pb, int header_bits);
void ff_mjpeg_escape_FF(PutBitContext *pb, int start);
int ff_mjpeg_encode_stuffing(MpegEncContext *s);
void ff_mjpeg_init_hvsample(AVCodecContext *avctx, int hsample[4], int vsample[4]);
void ff_mjpeg_encode_dc(PutBitContext *pb, int val,
uint8_t *huff_size, uint16_t *huff_code);
av_cold void ff_init_uni_ac_vlc(const uint8_t huff_size_ac[256], uint8_t *uni_ac_vlc_len);
