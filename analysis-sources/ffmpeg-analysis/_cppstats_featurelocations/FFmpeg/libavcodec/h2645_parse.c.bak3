



















#include <string.h>

#include "config.h"

#include "libavutil/intmath.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"

#include "bytestream.h"
#include "hevc.h"
#include "h264.h"
#include "h2645_parse.h"

int ff_h2645_extract_rbsp(const uint8_t *src, int length,
H2645RBSP *rbsp, H2645NAL *nal, int small_padding)
{
int i, si, di;
uint8_t *dst;

nal->skipped_bytes = 0;
#define STARTCODE_TEST if (i + 2 < length && src[i + 1] == 0 && src[i + 2] <= 3) { if (src[i + 2] != 3 && src[i + 2] != 0) { length = i; } break; }







#if HAVE_FAST_UNALIGNED
#define FIND_FIRST_ZERO if (i > 0 && !src[i]) i--; while (src[i]) i++




#if HAVE_FAST_64BIT
for (i = 0; i + 1 < length; i += 9) {
if (!((~AV_RN64(src + i) &
(AV_RN64(src + i) - 0x0100010001000101ULL)) &
0x8000800080008080ULL))
continue;
FIND_FIRST_ZERO;
STARTCODE_TEST;
i -= 7;
}
#else
for (i = 0; i + 1 < length; i += 5) {
if (!((~AV_RN32(src + i) &
(AV_RN32(src + i) - 0x01000101U)) &
0x80008080U))
continue;
FIND_FIRST_ZERO;
STARTCODE_TEST;
i -= 3;
}
#endif 
#else
for (i = 0; i + 1 < length; i += 2) {
if (src[i])
continue;
if (i > 0 && src[i - 1] == 0)
i--;
STARTCODE_TEST;
}
#endif 

if (i >= length - 1 && small_padding) { 
nal->data =
nal->raw_data = src;
nal->size =
nal->raw_size = length;
return length;
} else if (i > length)
i = length;

nal->rbsp_buffer = &rbsp->rbsp_buffer[rbsp->rbsp_buffer_size];
dst = nal->rbsp_buffer;

memcpy(dst, src, i);
si = di = i;
while (si + 2 < length) {

if (src[si + 2] > 3) {
dst[di++] = src[si++];
dst[di++] = src[si++];
} else if (src[si] == 0 && src[si + 1] == 0 && src[si + 2] != 0) {
if (src[si + 2] == 3) { 
dst[di++] = 0;
dst[di++] = 0;
si += 3;

if (nal->skipped_bytes_pos) {
nal->skipped_bytes++;
if (nal->skipped_bytes_pos_size < nal->skipped_bytes) {
nal->skipped_bytes_pos_size *= 2;
av_assert0(nal->skipped_bytes_pos_size >= nal->skipped_bytes);
av_reallocp_array(&nal->skipped_bytes_pos,
nal->skipped_bytes_pos_size,
sizeof(*nal->skipped_bytes_pos));
if (!nal->skipped_bytes_pos) {
nal->skipped_bytes_pos_size = 0;
return AVERROR(ENOMEM);
}
}
if (nal->skipped_bytes_pos)
nal->skipped_bytes_pos[nal->skipped_bytes-1] = di - 1;
}
continue;
} else 
goto nsc;
}

dst[di++] = src[si++];
}
while (si < length)
dst[di++] = src[si++];

nsc:
memset(dst + di, 0, AV_INPUT_BUFFER_PADDING_SIZE);

nal->data = dst;
nal->size = di;
nal->raw_data = src;
nal->raw_size = si;
rbsp->rbsp_buffer_size += si;

return si;
}

static const char *hevc_nal_type_name[64] = {
"TRAIL_N", 
"TRAIL_R", 
"TSA_N", 
"TSA_R", 
"STSA_N", 
"STSA_R", 
"RADL_N", 
"RADL_R", 
"RASL_N", 
"RASL_R", 
"RSV_VCL_N10", 
"RSV_VCL_R11", 
"RSV_VCL_N12", 
"RSV_VLC_R13", 
"RSV_VCL_N14", 
"RSV_VCL_R15", 
"BLA_W_LP", 
"BLA_W_RADL", 
"BLA_N_LP", 
"IDR_W_RADL", 
"IDR_N_LP", 
"CRA_NUT", 
"RSV_IRAP_VCL22", 
"RSV_IRAP_VCL23", 
"RSV_VCL24", 
"RSV_VCL25", 
"RSV_VCL26", 
"RSV_VCL27", 
"RSV_VCL28", 
"RSV_VCL29", 
"RSV_VCL30", 
"RSV_VCL31", 
"VPS", 
"SPS", 
"PPS", 
"AUD", 
"EOS_NUT", 
"EOB_NUT", 
"FD_NUT", 
"SEI_PREFIX", 
"SEI_SUFFIX", 
"RSV_NVCL41", 
"RSV_NVCL42", 
"RSV_NVCL43", 
"RSV_NVCL44", 
"RSV_NVCL45", 
"RSV_NVCL46", 
"RSV_NVCL47", 
"UNSPEC48", 
"UNSPEC49", 
"UNSPEC50", 
"UNSPEC51", 
"UNSPEC52", 
"UNSPEC53", 
"UNSPEC54", 
"UNSPEC55", 
"UNSPEC56", 
"UNSPEC57", 
"UNSPEC58", 
"UNSPEC59", 
"UNSPEC60", 
"UNSPEC61", 
"UNSPEC62", 
"UNSPEC63", 
};

static const char *hevc_nal_unit_name(int nal_type)
{
av_assert0(nal_type >= 0 && nal_type < 64);
return hevc_nal_type_name[nal_type];
}

static const char *h264_nal_type_name[32] = {
"Unspecified 0", 
"Coded slice of a non-IDR picture", 
"Coded slice data partition A", 
"Coded slice data partition B", 
"Coded slice data partition C", 
"IDR", 
"SEI", 
"SPS", 
"PPS", 
"AUD", 
"End of sequence", 
"End of stream", 
"Filler data", 
"SPS extension", 
"Prefix", 
"Subset SPS", 
"Depth parameter set", 
"Reserved 17", 
"Reserved 18", 
"Auxiliary coded picture without partitioning", 
"Slice extension", 
"Slice extension for a depth view or a 3D-AVC texture view", 
"Reserved 22", 
"Reserved 23", 
"Unspecified 24", 
"Unspecified 25", 
"Unspecified 26", 
"Unspecified 27", 
"Unspecified 28", 
"Unspecified 29", 
"Unspecified 30", 
"Unspecified 31", 
};

static const char *h264_nal_unit_name(int nal_type)
{
av_assert0(nal_type >= 0 && nal_type < 32);
return h264_nal_type_name[nal_type];
}

static int get_bit_length(H2645NAL *nal, int skip_trailing_zeros)
{
int size = nal->size;
int v;

while (skip_trailing_zeros && size > 0 && nal->data[size - 1] == 0)
size--;

if (!size)
return 0;

v = nal->data[size - 1];

if (size > INT_MAX / 8)
return AVERROR(ERANGE);
size *= 8;



if (v)
size -= ff_ctz(v) + 1;

return size;
}





static int hevc_parse_nal_header(H2645NAL *nal, void *logctx)
{
GetBitContext *gb = &nal->gb;

if (get_bits1(gb) != 0)
return AVERROR_INVALIDDATA;

nal->type = get_bits(gb, 6);

nal->nuh_layer_id = get_bits(gb, 6);
nal->temporal_id = get_bits(gb, 3) - 1;
if (nal->temporal_id < 0)
return AVERROR_INVALIDDATA;

av_log(logctx, AV_LOG_DEBUG,
"nal_unit_type: %d(%s), nuh_layer_id: %d, temporal_id: %d\n",
nal->type, hevc_nal_unit_name(nal->type), nal->nuh_layer_id, nal->temporal_id);

return 1;
}

static int h264_parse_nal_header(H2645NAL *nal, void *logctx)
{
GetBitContext *gb = &nal->gb;

if (get_bits1(gb) != 0)
return AVERROR_INVALIDDATA;

nal->ref_idc = get_bits(gb, 2);
nal->type = get_bits(gb, 5);

av_log(logctx, AV_LOG_DEBUG,
"nal_unit_type: %d(%s), nal_ref_idc: %d\n",
nal->type, h264_nal_unit_name(nal->type), nal->ref_idc);

return 1;
}

static int find_next_start_code(const uint8_t *buf, const uint8_t *next_avc)
{
int i = 0;

if (buf + 3 >= next_avc)
return next_avc - buf;

while (buf + i + 3 < next_avc) {
if (buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1)
break;
i++;
}
return i + 3;
}

static void alloc_rbsp_buffer(H2645RBSP *rbsp, unsigned int size, int use_ref)
{
int min_size = size;

if (size > INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
goto fail;
size += AV_INPUT_BUFFER_PADDING_SIZE;

if (rbsp->rbsp_buffer_alloc_size >= size &&
(!rbsp->rbsp_buffer_ref || av_buffer_is_writable(rbsp->rbsp_buffer_ref))) {
av_assert0(rbsp->rbsp_buffer);
memset(rbsp->rbsp_buffer + min_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
return;
}

size = FFMIN(size + size / 16 + 32, INT_MAX);

if (rbsp->rbsp_buffer_ref)
av_buffer_unref(&rbsp->rbsp_buffer_ref);
else
av_free(rbsp->rbsp_buffer);

rbsp->rbsp_buffer = av_mallocz(size);
if (!rbsp->rbsp_buffer)
goto fail;
rbsp->rbsp_buffer_alloc_size = size;

if (use_ref) {
rbsp->rbsp_buffer_ref = av_buffer_create(rbsp->rbsp_buffer, size,
NULL, NULL, 0);
if (!rbsp->rbsp_buffer_ref)
goto fail;
}

return;

fail:
rbsp->rbsp_buffer_alloc_size = 0;
if (rbsp->rbsp_buffer_ref) {
av_buffer_unref(&rbsp->rbsp_buffer_ref);
rbsp->rbsp_buffer = NULL;
} else
av_freep(&rbsp->rbsp_buffer);

return;
}

int ff_h2645_packet_split(H2645Packet *pkt, const uint8_t *buf, int length,
void *logctx, int is_nalff, int nal_length_size,
enum AVCodecID codec_id, int small_padding, int use_ref)
{
GetByteContext bc;
int consumed, ret = 0;
int next_avc = is_nalff ? 0 : length;
int64_t padding = small_padding ? 0 : MAX_MBPAIR_SIZE;

bytestream2_init(&bc, buf, length);
alloc_rbsp_buffer(&pkt->rbsp, length + padding, use_ref);

if (!pkt->rbsp.rbsp_buffer)
return AVERROR(ENOMEM);

pkt->rbsp.rbsp_buffer_size = 0;
pkt->nb_nals = 0;
while (bytestream2_get_bytes_left(&bc) >= 4) {
H2645NAL *nal;
int extract_length = 0;
int skip_trailing_zeros = 1;

if (bytestream2_tell(&bc) == next_avc) {
int i = 0;
extract_length = get_nalsize(nal_length_size,
bc.buffer, bytestream2_get_bytes_left(&bc), &i, logctx);
if (extract_length < 0)
return extract_length;

bytestream2_skip(&bc, nal_length_size);

next_avc = bytestream2_tell(&bc) + extract_length;
} else {
int buf_index;

if (bytestream2_tell(&bc) > next_avc)
av_log(logctx, AV_LOG_WARNING, "Exceeded next NALFF position, re-syncing.\n");


buf_index = find_next_start_code(bc.buffer, buf + next_avc);

bytestream2_skip(&bc, buf_index);

if (!bytestream2_get_bytes_left(&bc)) {
if (pkt->nb_nals > 0) {


return 0;
} else {
av_log(logctx, AV_LOG_ERROR, "No start code is found.\n");
return AVERROR_INVALIDDATA;
}
}

extract_length = FFMIN(bytestream2_get_bytes_left(&bc), next_avc - bytestream2_tell(&bc));

if (bytestream2_tell(&bc) >= next_avc) {

bytestream2_skip(&bc, next_avc - bytestream2_tell(&bc));
continue;
}
}

if (pkt->nals_allocated < pkt->nb_nals + 1) {
int new_size = pkt->nals_allocated + 1;
void *tmp;

if (new_size >= INT_MAX / sizeof(*pkt->nals))
return AVERROR(ENOMEM);

tmp = av_fast_realloc(pkt->nals, &pkt->nal_buffer_size, new_size * sizeof(*pkt->nals));
if (!tmp)
return AVERROR(ENOMEM);

pkt->nals = tmp;
memset(pkt->nals + pkt->nals_allocated, 0, sizeof(*pkt->nals));

nal = &pkt->nals[pkt->nb_nals];
nal->skipped_bytes_pos_size = 1024; 
nal->skipped_bytes_pos = av_malloc_array(nal->skipped_bytes_pos_size, sizeof(*nal->skipped_bytes_pos));
if (!nal->skipped_bytes_pos)
return AVERROR(ENOMEM);

pkt->nals_allocated = new_size;
}
nal = &pkt->nals[pkt->nb_nals];

consumed = ff_h2645_extract_rbsp(bc.buffer, extract_length, &pkt->rbsp, nal, small_padding);
if (consumed < 0)
return consumed;

if (is_nalff && (extract_length != consumed) && extract_length)
av_log(logctx, AV_LOG_DEBUG,
"NALFF: Consumed only %d bytes instead of %d\n",
consumed, extract_length);

pkt->nb_nals++;

bytestream2_skip(&bc, consumed);


if (bytestream2_get_bytes_left(&bc) >= 4 &&
bytestream2_peek_be32(&bc) == 0x000001E0)
skip_trailing_zeros = 0;

nal->size_bits = get_bit_length(nal, skip_trailing_zeros);

ret = init_get_bits(&nal->gb, nal->data, nal->size_bits);
if (ret < 0)
return ret;

if (codec_id == AV_CODEC_ID_HEVC)
ret = hevc_parse_nal_header(nal, logctx);
else
ret = h264_parse_nal_header(nal, logctx);
if (ret <= 0 || nal->size <= 0 || nal->size_bits <= 0) {
if (ret < 0) {
av_log(logctx, AV_LOG_WARNING, "Invalid NAL unit %d, skipping.\n",
nal->type);
}
pkt->nb_nals--;
}
}

return 0;
}

void ff_h2645_packet_uninit(H2645Packet *pkt)
{
int i;
for (i = 0; i < pkt->nals_allocated; i++) {
av_freep(&pkt->nals[i].skipped_bytes_pos);
}
av_freep(&pkt->nals);
pkt->nals_allocated = pkt->nal_buffer_size = 0;
if (pkt->rbsp.rbsp_buffer_ref) {
av_buffer_unref(&pkt->rbsp.rbsp_buffer_ref);
pkt->rbsp.rbsp_buffer = NULL;
} else
av_freep(&pkt->rbsp.rbsp_buffer);
pkt->rbsp.rbsp_buffer_alloc_size = pkt->rbsp.rbsp_buffer_size = 0;
}
