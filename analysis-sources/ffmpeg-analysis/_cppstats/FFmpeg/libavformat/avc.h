#include <stdint.h>
#include "avio.h"
int ff_avc_parse_nal_units(AVIOContext *s, const uint8_t *buf, int size);
int ff_avc_parse_nal_units_buf(const uint8_t *buf_in, uint8_t **buf, int *size);
int ff_isom_write_avcc(AVIOContext *pb, const uint8_t *data, int len);
const uint8_t *ff_avc_find_startcode(const uint8_t *p, const uint8_t *end);
int ff_avc_write_annexb_extradata(const uint8_t *in, uint8_t **buf, int *size);
const uint8_t *ff_avc_mp4_find_startcode(const uint8_t *start,
const uint8_t *end,
int nal_length_size);
uint8_t *ff_nal_unit_extract_rbsp(const uint8_t *src, uint32_t src_len,
uint32_t *dst_len, int header_len);
typedef struct {
uint8_t id;
uint8_t profile_idc;
uint8_t level_idc;
uint8_t constraint_set_flags;
uint8_t chroma_format_idc;
uint8_t bit_depth_luma;
uint8_t bit_depth_chroma;
uint8_t frame_mbs_only_flag;
AVRational sar;
} H264SequenceParameterSet;
H264SequenceParameterSet *ff_avc_decode_sps(const uint8_t *src, int src_len);
