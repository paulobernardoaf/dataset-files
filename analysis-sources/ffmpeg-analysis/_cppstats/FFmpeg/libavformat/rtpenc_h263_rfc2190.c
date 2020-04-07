#include "avformat.h"
#include "rtpenc.h"
#include "libavcodec/put_bits.h"
#include "libavcodec/get_bits.h"
struct H263Info {
int src;
int i;
int u;
int s;
int a;
int pb;
int tr;
};
struct H263State {
int gobn;
int mba;
int hmv1, vmv1, hmv2, vmv2;
int quant;
};
static void send_mode_a(AVFormatContext *s1, const struct H263Info *info,
const uint8_t *buf, int len, int ebits, int m)
{
RTPMuxContext *s = s1->priv_data;
PutBitContext pb;
init_put_bits(&pb, s->buf, 32);
put_bits(&pb, 1, 0); 
put_bits(&pb, 1, 0); 
put_bits(&pb, 3, 0); 
put_bits(&pb, 3, ebits); 
put_bits(&pb, 3, info->src); 
put_bits(&pb, 1, info->i); 
put_bits(&pb, 1, info->u); 
put_bits(&pb, 1, info->s); 
put_bits(&pb, 1, info->a); 
put_bits(&pb, 4, 0); 
put_bits(&pb, 2, 0); 
put_bits(&pb, 3, 0); 
put_bits(&pb, 8, info->tr); 
flush_put_bits(&pb);
memcpy(s->buf + 4, buf, len);
ff_rtp_send_data(s1, s->buf, len + 4, m);
}
static void send_mode_b(AVFormatContext *s1, const struct H263Info *info,
const struct H263State *state, const uint8_t *buf,
int len, int sbits, int ebits, int m)
{
RTPMuxContext *s = s1->priv_data;
PutBitContext pb;
init_put_bits(&pb, s->buf, 64);
put_bits(&pb, 1, 1); 
put_bits(&pb, 1, 0); 
put_bits(&pb, 3, sbits); 
put_bits(&pb, 3, ebits); 
put_bits(&pb, 3, info->src); 
put_bits(&pb, 5, state->quant); 
put_bits(&pb, 5, state->gobn); 
put_bits(&pb, 9, state->mba); 
put_bits(&pb, 2, 0); 
put_bits(&pb, 1, info->i); 
put_bits(&pb, 1, info->u); 
put_bits(&pb, 1, info->s); 
put_bits(&pb, 1, info->a); 
put_bits(&pb, 7, state->hmv1); 
put_bits(&pb, 7, state->vmv1); 
put_bits(&pb, 7, state->hmv2); 
put_bits(&pb, 7, state->vmv2); 
flush_put_bits(&pb);
memcpy(s->buf + 8, buf, len);
ff_rtp_send_data(s1, s->buf, len + 8, m);
}
void ff_rtp_send_h263_rfc2190(AVFormatContext *s1, const uint8_t *buf, int size,
const uint8_t *mb_info, int mb_info_size)
{
RTPMuxContext *s = s1->priv_data;
int len, sbits = 0, ebits = 0;
GetBitContext gb;
struct H263Info info = { 0 };
struct H263State state = { 0 };
int mb_info_pos = 0, mb_info_count = mb_info_size / 12;
const uint8_t *buf_base = buf;
s->timestamp = s->cur_timestamp;
init_get_bits(&gb, buf, size*8);
if (get_bits(&gb, 22) == 0x20) { 
info.tr = get_bits(&gb, 8);
skip_bits(&gb, 2); 
skip_bits(&gb, 3); 
info.src = get_bits(&gb, 3);
info.i = get_bits(&gb, 1);
info.u = get_bits(&gb, 1);
info.s = get_bits(&gb, 1);
info.a = get_bits(&gb, 1);
info.pb = get_bits(&gb, 1);
}
while (size > 0) {
struct H263State packet_start_state = state;
len = FFMIN(s->max_payload_size - 8, size);
if (len < size) {
const uint8_t *end = ff_h263_find_resync_marker_reverse(buf,
buf + len);
len = end - buf;
if (len == s->max_payload_size - 8) {
while (mb_info_pos < mb_info_count) {
uint32_t pos = AV_RL32(&mb_info[12*mb_info_pos])/8;
if (pos >= buf - buf_base)
break;
mb_info_pos++;
}
while (mb_info_pos + 1 < mb_info_count) {
uint32_t pos = AV_RL32(&mb_info[12*(mb_info_pos + 1)])/8;
if (pos >= end - buf_base)
break;
mb_info_pos++;
}
if (mb_info_pos < mb_info_count) {
const uint8_t *ptr = &mb_info[12*mb_info_pos];
uint32_t bit_pos = AV_RL32(ptr);
uint32_t pos_next_mb_info = (bit_pos + 7)/8;
if (pos_next_mb_info <= end - buf_base) {
state.quant = ptr[4];
state.gobn = ptr[5];
state.mba = AV_RL16(&ptr[6]);
state.hmv1 = (int8_t) ptr[8];
state.vmv1 = (int8_t) ptr[9];
state.hmv2 = (int8_t) ptr[10];
state.vmv2 = (int8_t) ptr[11];
ebits = 8 * pos_next_mb_info - bit_pos;
len = pos_next_mb_info - (buf - buf_base);
mb_info_pos++;
}
} else {
av_log(s1, AV_LOG_ERROR, "Unable to split H.263 packet, "
"use -mb_info %d or -ps 1.\n",
s->max_payload_size - 8);
}
}
}
if (size > 2 && !buf[0] && !buf[1])
send_mode_a(s1, &info, buf, len, ebits, len == size);
else
send_mode_b(s1, &info, &packet_start_state, buf, len, sbits,
ebits, len == size);
if (ebits) {
sbits = 8 - ebits;
len--;
} else {
sbits = 0;
}
buf += len;
size -= len;
ebits = 0;
}
}
