



















#include "libavutil/intreadwrite.h"

#include "avformat.h"
#include "rtpenc.h"


void ff_rtp_send_aac(AVFormatContext *s1, const uint8_t *buff, int size)
{
RTPMuxContext *s = s1->priv_data;
AVStream *st = s1->streams[0];
const int max_au_headers_size = 2 + 2 * s->max_frames_per_packet;
int len, max_packet_size = s->max_payload_size - max_au_headers_size;
uint8_t *p;


if ((s1->streams[0]->codecpar->extradata_size) == 0) {
size -= 7;
buff += 7;
}


len = (s->buf_ptr - s->buf);
if (s->num_frames &&
(s->num_frames == s->max_frames_per_packet ||
(len + size) > s->max_payload_size ||
av_compare_ts(s->cur_timestamp - s->timestamp, st->time_base,
s1->max_delay, AV_TIME_BASE_Q) >= 0)) {
int au_size = s->num_frames * 2;

p = s->buf + max_au_headers_size - au_size - 2;
if (p != s->buf) {
memmove(p + 2, s->buf + 2, au_size);
}

AV_WB16(p, au_size * 8);

ff_rtp_send_data(s1, p, s->buf_ptr - p, 1);

s->num_frames = 0;
}
if (s->num_frames == 0) {
s->buf_ptr = s->buf + max_au_headers_size;
s->timestamp = s->cur_timestamp;
}

if (size <= max_packet_size) {
p = s->buf + s->num_frames++ * 2 + 2;
AV_WB16(p, size * 8);
memcpy(s->buf_ptr, buff, size);
s->buf_ptr += size;
} else {
int au_size = size;

max_packet_size = s->max_payload_size - 4;
p = s->buf;
AV_WB16(p, 2 * 8);
while (size > 0) {
len = FFMIN(size, max_packet_size);
AV_WB16(&p[2], au_size * 8);
memcpy(p + 4, buff, len);
ff_rtp_send_data(s1, p, len + 4, len == size);
size -= len;
buff += len;
}
}
}
