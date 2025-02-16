#include "mms.h"
#include "asf.h"
#include "libavutil/intreadwrite.h"
#define MMS_MAX_STREAMS 256 
int ff_mms_read_header(MMSContext *mms, uint8_t *buf, const int size)
{
char *pos;
int size_to_copy;
int remaining_size = mms->asf_header_size - mms->asf_header_read_size;
size_to_copy = FFMIN(size, remaining_size);
pos = mms->asf_header + mms->asf_header_read_size;
memcpy(buf, pos, size_to_copy);
if (mms->asf_header_read_size == mms->asf_header_size) {
av_freep(&mms->asf_header); 
}
mms->asf_header_read_size += size_to_copy;
return size_to_copy;
}
int ff_mms_read_data(MMSContext *mms, uint8_t *buf, const int size)
{
int read_size;
read_size = FFMIN(size, mms->remaining_in_len);
memcpy(buf, mms->read_in_ptr, read_size);
mms->remaining_in_len -= read_size;
mms->read_in_ptr += read_size;
return read_size;
}
int ff_mms_asf_header_parser(MMSContext *mms)
{
uint8_t *p = mms->asf_header;
uint8_t *end;
int flags, stream_id;
mms->stream_num = 0;
if (mms->asf_header_size < sizeof(ff_asf_guid) * 2 + 22 ||
memcmp(p, ff_asf_header, sizeof(ff_asf_guid))) {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (invalid ASF header, size=%d)\n",
mms->asf_header_size);
return AVERROR_INVALIDDATA;
}
end = mms->asf_header + mms->asf_header_size;
p += sizeof(ff_asf_guid) + 14;
while(end - p >= sizeof(ff_asf_guid) + 8) {
uint64_t chunksize;
if (!memcmp(p, ff_asf_data_header, sizeof(ff_asf_guid))) {
chunksize = 50; 
} else {
chunksize = AV_RL64(p + sizeof(ff_asf_guid));
}
if (!chunksize || chunksize > end - p) {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (header chunksize %"PRId64" is invalid)\n",
chunksize);
return AVERROR_INVALIDDATA;
}
if (!memcmp(p, ff_asf_file_header, sizeof(ff_asf_guid))) {
if (end - p > sizeof(ff_asf_guid) * 2 + 68) {
mms->asf_packet_len = AV_RL32(p + sizeof(ff_asf_guid) * 2 + 64);
if (mms->asf_packet_len <= 0 || mms->asf_packet_len > sizeof(mms->in_buffer)) {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (too large pkt_len %d)\n",
mms->asf_packet_len);
return AVERROR_INVALIDDATA;
}
}
} else if (!memcmp(p, ff_asf_stream_header, sizeof(ff_asf_guid))) {
if (end - p >= (sizeof(ff_asf_guid) * 3 + 26)) {
flags = AV_RL16(p + sizeof(ff_asf_guid)*3 + 24);
stream_id = flags & 0x7F;
if (mms->stream_num < MMS_MAX_STREAMS &&
46 + mms->stream_num * 6 < sizeof(mms->out_buffer)) {
mms->streams = av_fast_realloc(mms->streams,
&mms->nb_streams_allocated,
(mms->stream_num + 1) * sizeof(MMSStream));
if (!mms->streams)
return AVERROR(ENOMEM);
mms->streams[mms->stream_num].id = stream_id;
mms->stream_num++;
} else {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (too many A/V streams)\n");
return AVERROR_INVALIDDATA;
}
}
} else if (!memcmp(p, ff_asf_ext_stream_header, sizeof(ff_asf_guid))) {
if (end - p >= 88) {
int stream_count = AV_RL16(p + 84), ext_len_count = AV_RL16(p + 86);
uint64_t skip_bytes = 88;
while (stream_count--) {
if (end - p < skip_bytes + 4) {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (next stream name length is not in the buffer)\n");
return AVERROR_INVALIDDATA;
}
skip_bytes += 4 + AV_RL16(p + skip_bytes + 2);
}
while (ext_len_count--) {
if (end - p < skip_bytes + 22) {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (next extension system info length is not in the buffer)\n");
return AVERROR_INVALIDDATA;
}
skip_bytes += 22 + AV_RL32(p + skip_bytes + 18);
}
if (end - p < skip_bytes) {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (the last extension system info length is invalid)\n");
return AVERROR_INVALIDDATA;
}
if (chunksize - skip_bytes > 24)
chunksize = skip_bytes;
}
} else if (!memcmp(p, ff_asf_head1_guid, sizeof(ff_asf_guid))) {
chunksize = 46; 
if (chunksize > end - p) {
av_log(mms->mms_hd, AV_LOG_ERROR,
"Corrupt stream (header chunksize %"PRId64" is invalid)\n",
chunksize);
return AVERROR_INVALIDDATA;
}
}
p += chunksize;
}
return 0;
}
