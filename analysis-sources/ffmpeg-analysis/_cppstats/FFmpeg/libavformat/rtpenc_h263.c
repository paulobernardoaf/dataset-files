#include "avformat.h"
#include "rtpenc.h"
const uint8_t *ff_h263_find_resync_marker_reverse(const uint8_t *av_restrict start,
const uint8_t *av_restrict end)
{
const uint8_t *p = end - 1;
start += 1; 
for (; p > start; p -= 2) {
if (!*p) {
if (!p[ 1] && p[2]) return p;
else if (!p[-1] && p[1]) return p - 1;
}
}
return end;
}
void ff_rtp_send_h263(AVFormatContext *s1, const uint8_t *buf1, int size)
{
RTPMuxContext *s = s1->priv_data;
int len, max_packet_size;
uint8_t *q;
max_packet_size = s->max_payload_size;
while (size > 0) {
q = s->buf;
if (size >= 2 && (buf1[0] == 0) && (buf1[1] == 0)) {
*q++ = 0x04;
buf1 += 2;
size -= 2;
} else {
*q++ = 0;
}
*q++ = 0;
len = FFMIN(max_packet_size - 2, size);
if (len < size) {
const uint8_t *end = ff_h263_find_resync_marker_reverse(buf1,
buf1 + len);
len = end - buf1;
}
memcpy(q, buf1, len);
q += len;
s->timestamp = s->cur_timestamp;
ff_rtp_send_data(s1, s->buf, q - s->buf, (len == size));
buf1 += len;
size -= len;
}
}
