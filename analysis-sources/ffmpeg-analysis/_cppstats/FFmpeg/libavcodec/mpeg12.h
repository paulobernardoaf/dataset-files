#include "mpeg12vlc.h"
#include "mpegvideo.h"
extern uint8_t ff_mpeg12_static_rl_table_store[2][2][2*MAX_RUN + MAX_LEVEL + 3];
void ff_mpeg12_common_init(MpegEncContext *s);
#define INIT_2D_VLC_RL(rl, static_size, flags){static RL_VLC_ELEM rl_vlc_table[static_size];rl.rl_vlc[0] = rl_vlc_table;ff_init_2d_vlc_rl(&rl, static_size, flags);}
void ff_init_2d_vlc_rl(RLTable *rl, unsigned static_size, int flags);
static inline int decode_dc(GetBitContext *gb, int component)
{
int code, diff;
if (component == 0) {
code = get_vlc2(gb, ff_dc_lum_vlc.table, DC_VLC_BITS, 2);
} else {
code = get_vlc2(gb, ff_dc_chroma_vlc.table, DC_VLC_BITS, 2);
}
if (code < 0){
av_log(NULL, AV_LOG_ERROR, "invalid dc code at\n");
return 0xffff;
}
if (code == 0) {
diff = 0;
} else {
diff = get_xbits(gb, code);
}
return diff;
}
int ff_mpeg1_decode_block_intra(GetBitContext *gb,
const uint16_t *quant_matrix,
uint8_t *const scantable, int last_dc[3],
int16_t *block, int index, int qscale);
void ff_mpeg1_clean_buffers(MpegEncContext *s);
int ff_mpeg1_find_frame_end(ParseContext *pc, const uint8_t *buf, int buf_size, AVCodecParserContext *s);
void ff_mpeg1_encode_picture_header(MpegEncContext *s, int picture_number);
void ff_mpeg1_encode_mb(MpegEncContext *s, int16_t block[8][64],
int motion_x, int motion_y);
void ff_mpeg1_encode_init(MpegEncContext *s);
void ff_mpeg1_encode_slice_header(MpegEncContext *s);
void ff_mpeg12_find_best_frame_rate(AVRational frame_rate,
int *code, int *ext_n, int *ext_d,
int nonstandard);
