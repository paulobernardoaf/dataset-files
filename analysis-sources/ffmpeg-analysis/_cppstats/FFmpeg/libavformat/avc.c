#include "libavutil/intreadwrite.h"
#include "libavcodec/h264.h"
#include "libavcodec/get_bits.h"
#include "avformat.h"
#include "avio.h"
#include "avc.h"
#include "avio_internal.h"
static const uint8_t *ff_avc_find_startcode_internal(const uint8_t *p, const uint8_t *end)
{
const uint8_t *a = p + 4 - ((intptr_t)p & 3);
for (end -= 3; p < a && p < end; p++) {
if (p[0] == 0 && p[1] == 0 && p[2] == 1)
return p;
}
for (end -= 3; p < end; p += 4) {
uint32_t x = *(const uint32_t*)p;
if ((x - 0x01010101) & (~x) & 0x80808080) { 
if (p[1] == 0) {
if (p[0] == 0 && p[2] == 1)
return p;
if (p[2] == 0 && p[3] == 1)
return p+1;
}
if (p[3] == 0) {
if (p[2] == 0 && p[4] == 1)
return p+2;
if (p[4] == 0 && p[5] == 1)
return p+3;
}
}
}
for (end += 3; p < end; p++) {
if (p[0] == 0 && p[1] == 0 && p[2] == 1)
return p;
}
return end + 3;
}
const uint8_t *ff_avc_find_startcode(const uint8_t *p, const uint8_t *end){
const uint8_t *out= ff_avc_find_startcode_internal(p, end);
if(p<out && out<end && !out[-1]) out--;
return out;
}
int ff_avc_parse_nal_units(AVIOContext *pb, const uint8_t *buf_in, int size)
{
const uint8_t *p = buf_in;
const uint8_t *end = p + size;
const uint8_t *nal_start, *nal_end;
size = 0;
nal_start = ff_avc_find_startcode(p, end);
for (;;) {
while (nal_start < end && !*(nal_start++));
if (nal_start == end)
break;
nal_end = ff_avc_find_startcode(nal_start, end);
avio_wb32(pb, nal_end - nal_start);
avio_write(pb, nal_start, nal_end - nal_start);
size += 4 + nal_end - nal_start;
nal_start = nal_end;
}
return size;
}
int ff_avc_parse_nal_units_buf(const uint8_t *buf_in, uint8_t **buf, int *size)
{
AVIOContext *pb;
int ret = avio_open_dyn_buf(&pb);
if(ret < 0)
return ret;
ff_avc_parse_nal_units(pb, buf_in, *size);
*size = avio_close_dyn_buf(pb, buf);
return 0;
}
int ff_isom_write_avcc(AVIOContext *pb, const uint8_t *data, int len)
{
AVIOContext *sps_pb = NULL, *pps_pb = NULL, *sps_ext_pb = NULL;
uint8_t *buf, *end, *start;
uint8_t *sps, *pps, *sps_ext;
uint32_t sps_size = 0, pps_size = 0, sps_ext_size = 0;
int ret, nb_sps = 0, nb_pps = 0, nb_sps_ext = 0;
if (len <= 6)
return AVERROR_INVALIDDATA;
if (AV_RB32(data) != 0x00000001 &&
AV_RB24(data) != 0x000001) {
avio_write(pb, data, len);
return 0;
}
ret = ff_avc_parse_nal_units_buf(data, &buf, &len);
if (ret < 0)
return ret;
start = buf;
end = buf + len;
ret = avio_open_dyn_buf(&sps_pb);
if (ret < 0)
goto fail;
ret = avio_open_dyn_buf(&pps_pb);
if (ret < 0)
goto fail;
ret = avio_open_dyn_buf(&sps_ext_pb);
if (ret < 0)
goto fail;
while (end - buf > 4) {
uint32_t size;
uint8_t nal_type;
size = FFMIN(AV_RB32(buf), end - buf - 4);
buf += 4;
nal_type = buf[0] & 0x1f;
if (nal_type == 7) { 
nb_sps++;
if (size > UINT16_MAX || nb_sps >= H264_MAX_SPS_COUNT) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
avio_wb16(sps_pb, size);
avio_write(sps_pb, buf, size);
} else if (nal_type == 8) { 
nb_pps++;
if (size > UINT16_MAX || nb_pps >= H264_MAX_PPS_COUNT) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
avio_wb16(pps_pb, size);
avio_write(pps_pb, buf, size);
} else if (nal_type == 13) { 
nb_sps_ext++;
if (size > UINT16_MAX || nb_sps_ext >= 256) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
avio_wb16(sps_ext_pb, size);
avio_write(sps_ext_pb, buf, size);
}
buf += size;
}
sps_size = avio_get_dyn_buf(sps_pb, &sps);
pps_size = avio_get_dyn_buf(pps_pb, &pps);
sps_ext_size = avio_get_dyn_buf(sps_ext_pb, &sps_ext);
if (sps_size < 6 || !pps_size) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
avio_w8(pb, 1); 
avio_w8(pb, sps[3]); 
avio_w8(pb, sps[4]); 
avio_w8(pb, sps[5]); 
avio_w8(pb, 0xff); 
avio_w8(pb, 0xe0 | nb_sps); 
avio_write(pb, sps, sps_size);
avio_w8(pb, nb_pps); 
avio_write(pb, pps, pps_size);
if (sps[3] != 66 && sps[3] != 77 && sps[3] != 88) {
H264SequenceParameterSet *seq = ff_avc_decode_sps(sps + 3, sps_size - 3);
if (!seq) {
ret = AVERROR(ENOMEM);
goto fail;
}
avio_w8(pb, 0xfc | seq->chroma_format_idc); 
avio_w8(pb, 0xf8 | (seq->bit_depth_luma - 8)); 
avio_w8(pb, 0xf8 | (seq->bit_depth_chroma - 8)); 
avio_w8(pb, nb_sps_ext); 
if (nb_sps_ext)
avio_write(pb, sps_ext, sps_ext_size);
av_free(seq);
}
fail:
ffio_free_dyn_buf(&sps_pb);
ffio_free_dyn_buf(&pps_pb);
ffio_free_dyn_buf(&sps_ext_pb);
av_free(start);
return ret;
}
int ff_avc_write_annexb_extradata(const uint8_t *in, uint8_t **buf, int *size)
{
uint16_t sps_size, pps_size;
uint8_t *out;
int out_size;
*buf = NULL;
if (*size >= 4 && (AV_RB32(in) == 0x00000001 || AV_RB24(in) == 0x000001))
return 0;
if (*size < 11 || in[0] != 1)
return AVERROR_INVALIDDATA;
sps_size = AV_RB16(&in[6]);
if (11 + sps_size > *size)
return AVERROR_INVALIDDATA;
pps_size = AV_RB16(&in[9 + sps_size]);
if (11 + sps_size + pps_size > *size)
return AVERROR_INVALIDDATA;
out_size = 8 + sps_size + pps_size;
out = av_mallocz(out_size + AV_INPUT_BUFFER_PADDING_SIZE);
if (!out)
return AVERROR(ENOMEM);
AV_WB32(&out[0], 0x00000001);
memcpy(out + 4, &in[8], sps_size);
AV_WB32(&out[4 + sps_size], 0x00000001);
memcpy(out + 8 + sps_size, &in[11 + sps_size], pps_size);
*buf = out;
*size = out_size;
return 0;
}
const uint8_t *ff_avc_mp4_find_startcode(const uint8_t *start,
const uint8_t *end,
int nal_length_size)
{
unsigned int res = 0;
if (end - start < nal_length_size)
return NULL;
while (nal_length_size--)
res = (res << 8) | *start++;
if (res > end - start)
return NULL;
return start + res;
}
uint8_t *ff_nal_unit_extract_rbsp(const uint8_t *src, uint32_t src_len,
uint32_t *dst_len, int header_len)
{
uint8_t *dst;
uint32_t i, len;
dst = av_malloc(src_len + AV_INPUT_BUFFER_PADDING_SIZE);
if (!dst)
return NULL;
i = len = 0;
while (i < header_len && i < src_len)
dst[len++] = src[i++];
while (i + 2 < src_len)
if (!src[i] && !src[i + 1] && src[i + 2] == 3) {
dst[len++] = src[i++];
dst[len++] = src[i++];
i++; 
} else
dst[len++] = src[i++];
while (i < src_len)
dst[len++] = src[i++];
memset(dst + len, 0, AV_INPUT_BUFFER_PADDING_SIZE);
*dst_len = len;
return dst;
}
static const AVRational avc_sample_aspect_ratio[17] = {
{ 0, 1 },
{ 1, 1 },
{ 12, 11 },
{ 10, 11 },
{ 16, 11 },
{ 40, 33 },
{ 24, 11 },
{ 20, 11 },
{ 32, 11 },
{ 80, 33 },
{ 18, 11 },
{ 15, 11 },
{ 64, 33 },
{ 160, 99 },
{ 4, 3 },
{ 3, 2 },
{ 2, 1 },
};
static inline int get_ue_golomb(GetBitContext *gb) {
int i;
for (i = 0; i < 32 && !get_bits1(gb); i++)
;
return get_bitsz(gb, i) + (1 << i) - 1;
}
static inline int get_se_golomb(GetBitContext *gb) {
int v = get_ue_golomb(gb) + 1;
int sign = -(v & 1);
return ((v >> 1) ^ sign) - sign;
}
H264SequenceParameterSet *ff_avc_decode_sps(const uint8_t *buf, int buf_size)
{
int i, j, ret, rbsp_size, aspect_ratio_idc, pic_order_cnt_type;
int num_ref_frames_in_pic_order_cnt_cycle;
int delta_scale, lastScale = 8, nextScale = 8;
int sizeOfScalingList;
H264SequenceParameterSet *sps = NULL;
GetBitContext gb;
uint8_t *rbsp_buf;
rbsp_buf = ff_nal_unit_extract_rbsp(buf, buf_size, &rbsp_size, 0);
if (!rbsp_buf)
return NULL;
ret = init_get_bits8(&gb, rbsp_buf, rbsp_size);
if (ret < 0)
goto end;
sps = av_mallocz(sizeof(*sps));
if (!sps)
goto end;
sps->profile_idc = get_bits(&gb, 8);
sps->constraint_set_flags |= get_bits1(&gb) << 0; 
sps->constraint_set_flags |= get_bits1(&gb) << 1; 
sps->constraint_set_flags |= get_bits1(&gb) << 2; 
sps->constraint_set_flags |= get_bits1(&gb) << 3; 
sps->constraint_set_flags |= get_bits1(&gb) << 4; 
sps->constraint_set_flags |= get_bits1(&gb) << 5; 
skip_bits(&gb, 2); 
sps->level_idc = get_bits(&gb, 8);
sps->id = get_ue_golomb(&gb);
if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
sps->profile_idc == 122 || sps->profile_idc == 244 || sps->profile_idc == 44 ||
sps->profile_idc == 83 || sps->profile_idc == 86 || sps->profile_idc == 118 ||
sps->profile_idc == 128 || sps->profile_idc == 138 || sps->profile_idc == 139 ||
sps->profile_idc == 134) {
sps->chroma_format_idc = get_ue_golomb(&gb); 
if (sps->chroma_format_idc == 3) {
skip_bits1(&gb); 
}
sps->bit_depth_luma = get_ue_golomb(&gb) + 8;
sps->bit_depth_chroma = get_ue_golomb(&gb) + 8;
skip_bits1(&gb); 
if (get_bits1(&gb)) { 
for (i = 0; i < ((sps->chroma_format_idc != 3) ? 8 : 12); i++) {
if (!get_bits1(&gb)) 
continue;
lastScale = 8;
nextScale = 8;
sizeOfScalingList = i < 6 ? 16 : 64;
for (j = 0; j < sizeOfScalingList; j++) {
if (nextScale != 0) {
delta_scale = get_se_golomb(&gb);
nextScale = (lastScale + delta_scale) & 0xff;
}
lastScale = nextScale == 0 ? lastScale : nextScale;
}
}
}
} else {
sps->chroma_format_idc = 1;
sps->bit_depth_luma = 8;
sps->bit_depth_chroma = 8;
}
get_ue_golomb(&gb); 
pic_order_cnt_type = get_ue_golomb(&gb);
if (pic_order_cnt_type == 0) {
get_ue_golomb(&gb); 
} else if (pic_order_cnt_type == 1) {
skip_bits1(&gb); 
get_se_golomb(&gb); 
get_se_golomb(&gb); 
num_ref_frames_in_pic_order_cnt_cycle = get_ue_golomb(&gb);
for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
get_se_golomb(&gb); 
}
get_ue_golomb(&gb); 
skip_bits1(&gb); 
get_ue_golomb(&gb); 
get_ue_golomb(&gb); 
sps->frame_mbs_only_flag = get_bits1(&gb);
if (!sps->frame_mbs_only_flag)
skip_bits1(&gb); 
skip_bits1(&gb); 
if (get_bits1(&gb)) { 
get_ue_golomb(&gb); 
get_ue_golomb(&gb); 
get_ue_golomb(&gb); 
get_ue_golomb(&gb); 
}
if (get_bits1(&gb)) { 
if (get_bits1(&gb)) { 
aspect_ratio_idc = get_bits(&gb, 8);
if (aspect_ratio_idc == 0xff) {
sps->sar.num = get_bits(&gb, 16);
sps->sar.den = get_bits(&gb, 16);
} else if (aspect_ratio_idc < FF_ARRAY_ELEMS(avc_sample_aspect_ratio)) {
sps->sar = avc_sample_aspect_ratio[aspect_ratio_idc];
}
}
}
if (!sps->sar.den) {
sps->sar.num = 1;
sps->sar.den = 1;
}
end:
av_free(rbsp_buf);
return sps;
}
