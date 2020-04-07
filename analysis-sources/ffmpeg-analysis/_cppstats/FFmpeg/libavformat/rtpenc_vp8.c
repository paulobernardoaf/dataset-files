#include "rtpenc.h"
void ff_rtp_send_vp8(AVFormatContext *s1, const uint8_t *buf, int size)
{
RTPMuxContext *s = s1->priv_data;
int len, max_packet_size, header_size;
s->buf_ptr = s->buf;
s->timestamp = s->cur_timestamp;
*s->buf_ptr++ = 0x90;
*s->buf_ptr++ = 0x80; 
*s->buf_ptr++ = s->frame_count++ & 0x7f;
header_size = s->buf_ptr - s->buf;
max_packet_size = s->max_payload_size - header_size;
while (size > 0) {
len = FFMIN(size, max_packet_size);
memcpy(s->buf_ptr, buf, len);
ff_rtp_send_data(s1, s->buf, len + header_size, size == len);
size -= len;
buf += len;
s->buf[0] &= ~0x10;
}
}
