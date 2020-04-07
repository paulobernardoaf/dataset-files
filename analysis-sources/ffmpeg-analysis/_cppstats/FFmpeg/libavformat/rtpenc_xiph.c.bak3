




















#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"

#include "avformat.h"
#include "rtpenc.h"






void ff_rtp_send_xiph(AVFormatContext *s1, const uint8_t *buff, int size)
{
RTPMuxContext *s = s1->priv_data;
AVStream *st = s1->streams[0];
int max_pkt_size, xdt, frag;
uint8_t *q;

max_pkt_size = s->max_payload_size - 6; 


switch (*buff) {
case 0x01: 
case 0x05: 
case 0x80: 
case 0x82: 
xdt = 1; 
break;
case 0x03: 
case 0x81: 
xdt = 2; 
break;
default:
xdt = 0; 
break;
}




q = s->buf;
*q++ = (RTP_XIPH_IDENT >> 16) & 0xff;
*q++ = (RTP_XIPH_IDENT >> 8) & 0xff;
*q++ = (RTP_XIPH_IDENT ) & 0xff;






frag = size <= max_pkt_size ? 0 : 1;

if (!frag && !xdt) { 
uint8_t *end_ptr = s->buf + 6 + max_pkt_size; 
uint8_t *ptr = s->buf_ptr + 2 + size; 
int remaining = end_ptr - ptr;

av_assert1(s->num_frames <= s->max_frames_per_packet);
if (s->num_frames > 0 &&
(remaining < 0 ||
s->num_frames == s->max_frames_per_packet ||
av_compare_ts(s->cur_timestamp - s->timestamp, st->time_base,
s1->max_delay, AV_TIME_BASE_Q) >= 0)) {

ff_rtp_send_data(s1, s->buf, s->buf_ptr - s->buf, 0);
s->num_frames = 0;
}


if (0 == s->num_frames)
s->timestamp = s->cur_timestamp;
s->num_frames++;



*q++ = s->num_frames;

if (s->num_frames > 1)
q = s->buf_ptr; 
AV_WB16(q, size);
q += 2;
memcpy(q, buff, size);
q += size;
s->buf_ptr = q;

return;
} else if (s->num_frames) {


ff_rtp_send_data(s1, s->buf, s->buf_ptr - s->buf, 0);
}

s->timestamp = s->cur_timestamp;
s->num_frames = 0;
s->buf_ptr = q;
while (size > 0) {
int len = (!frag || frag == 3) ? size : max_pkt_size;
q = s->buf_ptr;


*q++ = (frag << 6) | (xdt << 4); 
AV_WB16(q, len);
q += 2;

memcpy(q, buff, len);
q += len;
buff += len;
size -= len;

ff_rtp_send_data(s1, s->buf, q - s->buf, 0);

frag = size <= max_pkt_size ? 3 : 2;
}
}
