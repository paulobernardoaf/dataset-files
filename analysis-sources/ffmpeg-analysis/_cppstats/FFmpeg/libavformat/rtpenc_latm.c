#include "avformat.h"
#include "rtpenc.h"
void ff_rtp_send_latm(AVFormatContext *s1, const uint8_t *buff, int size)
{
RTPMuxContext *s = s1->priv_data;
int header_size;
int offset = 0;
int len = 0;
if ((s1->streams[0]->codecpar->extradata_size) == 0) {
size -= 7;
buff += 7;
}
header_size = size/0xFF + 1;
memset(s->buf, 0xFF, header_size - 1);
s->buf[header_size - 1] = size % 0xFF;
s->timestamp = s->cur_timestamp;
while (size > 0) {
len = FFMIN(size, s->max_payload_size - (!offset ? header_size : 0));
size -= len;
if (!offset) {
memcpy(s->buf + header_size, buff, len);
ff_rtp_send_data(s1, s->buf, header_size + len, !size);
} else {
ff_rtp_send_data(s1, buff + offset, len, !size);
}
offset += len;
}
}
