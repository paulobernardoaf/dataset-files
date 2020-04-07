























#include "golomb.h"
#include "hevc_ps.h"
#include "hevc_sei.h"

static int decode_nal_sei_decoded_picture_hash(HEVCSEIPictureHash *s, GetBitContext *gb)
{
int cIdx, i;
uint8_t hash_type;


hash_type = get_bits(gb, 8);

for (cIdx = 0; cIdx < 3; cIdx++) {
if (hash_type == 0) {
s->is_md5 = 1;
for (i = 0; i < 16; i++)
s->md5[cIdx][i] = get_bits(gb, 8);
} else if (hash_type == 1) {

skip_bits(gb, 16);
} else if (hash_type == 2) {

skip_bits(gb, 32);
}
}
return 0;
}

static int decode_nal_sei_mastering_display_info(HEVCSEIMasteringDisplay *s, GetBitContext *gb)
{
int i;

for (i = 0; i < 3; i++) {
s->display_primaries[i][0] = get_bits(gb, 16);
s->display_primaries[i][1] = get_bits(gb, 16);
}

s->white_point[0] = get_bits(gb, 16);
s->white_point[1] = get_bits(gb, 16);


s->max_luminance = get_bits_long(gb, 32);
s->min_luminance = get_bits_long(gb, 32);




s->present = 2;
return 0;
}

static int decode_nal_sei_content_light_info(HEVCSEIContentLight *s, GetBitContext *gb)
{

s->max_content_light_level = get_bits(gb, 16);
s->max_pic_average_light_level = get_bits(gb, 16);



s->present = 2;
return 0;
}

static int decode_nal_sei_frame_packing_arrangement(HEVCSEIFramePacking *s, GetBitContext *gb)
{
get_ue_golomb_long(gb); 
s->present = !get_bits1(gb);

if (s->present) {
s->arrangement_type = get_bits(gb, 7);
s->quincunx_subsampling = get_bits1(gb);
s->content_interpretation_type = get_bits(gb, 6);


skip_bits(gb, 3);
s->current_frame_is_frame0_flag = get_bits1(gb);

skip_bits(gb, 2);

if (!s->quincunx_subsampling && s->arrangement_type != 5)
skip_bits(gb, 16); 
skip_bits(gb, 8); 
skip_bits1(gb); 
}
skip_bits1(gb); 
return 0;
}

static int decode_nal_sei_display_orientation(HEVCSEIDisplayOrientation *s, GetBitContext *gb)
{
s->present = !get_bits1(gb);

if (s->present) {
s->hflip = get_bits1(gb); 
s->vflip = get_bits1(gb); 

s->anticlockwise_rotation = get_bits(gb, 16);
skip_bits1(gb); 
}

return 0;
}

static int decode_nal_sei_pic_timing(HEVCSEI *s, GetBitContext *gb, const HEVCParamSets *ps,
void *logctx, int size)
{
HEVCSEIPictureTiming *h = &s->picture_timing;
HEVCSPS *sps;

if (!ps->sps_list[s->active_seq_parameter_set_id])
return(AVERROR(ENOMEM));
sps = (HEVCSPS*)ps->sps_list[s->active_seq_parameter_set_id]->data;

if (sps->vui.frame_field_info_present_flag) {
int pic_struct = get_bits(gb, 4);
h->picture_struct = AV_PICTURE_STRUCTURE_UNKNOWN;
if (pic_struct == 2 || pic_struct == 10 || pic_struct == 12) {
av_log(logctx, AV_LOG_DEBUG, "BOTTOM Field\n");
h->picture_struct = AV_PICTURE_STRUCTURE_BOTTOM_FIELD;
} else if (pic_struct == 1 || pic_struct == 9 || pic_struct == 11) {
av_log(logctx, AV_LOG_DEBUG, "TOP Field\n");
h->picture_struct = AV_PICTURE_STRUCTURE_TOP_FIELD;
} else if (pic_struct == 7) {
av_log(logctx, AV_LOG_DEBUG, "Frame/Field Doubling\n");
h->picture_struct = HEVC_SEI_PIC_STRUCT_FRAME_DOUBLING;
} else if (pic_struct == 8) {
av_log(logctx, AV_LOG_DEBUG, "Frame/Field Tripling\n");
h->picture_struct = HEVC_SEI_PIC_STRUCT_FRAME_TRIPLING;
}
get_bits(gb, 2); 
get_bits(gb, 1); 
skip_bits1(gb);
size--;
}
skip_bits_long(gb, 8 * size);

return 0;
}

static int decode_registered_user_data_closed_caption(HEVCSEIA53Caption *s, GetBitContext *gb,
int size)
{
int flag;
int user_data_type_code;
int cc_count;

if (size < 3)
return AVERROR(EINVAL);

user_data_type_code = get_bits(gb, 8);
if (user_data_type_code == 0x3) {
skip_bits(gb, 1); 

flag = get_bits(gb, 1); 
if (flag) {
skip_bits(gb, 1);
cc_count = get_bits(gb, 5);
skip_bits(gb, 8); 
size -= 2;

if (cc_count && size >= cc_count * 3) {
int old_size = s->buf_ref ? s->buf_ref->size : 0;
const uint64_t new_size = (old_size + cc_count
* UINT64_C(3));
int i, ret;

if (new_size > INT_MAX)
return AVERROR(EINVAL);


ret = av_buffer_realloc(&s->buf_ref, new_size);
if (ret < 0)
return ret;

for (i = 0; i < cc_count; i++) {
s->buf_ref->data[old_size++] = get_bits(gb, 8);
s->buf_ref->data[old_size++] = get_bits(gb, 8);
s->buf_ref->data[old_size++] = get_bits(gb, 8);
}
skip_bits(gb, 8); 
}
}
} else {
int i;
for (i = 0; i < size - 1; i++)
skip_bits(gb, 8);
}

return 0;
}

static int decode_nal_sei_user_data_registered_itu_t_t35(HEVCSEI *s, GetBitContext *gb,
int size)
{
uint32_t country_code;
uint32_t user_identifier;

if (size < 7)
return AVERROR(EINVAL);
size -= 7;

country_code = get_bits(gb, 8);
if (country_code == 0xFF) {
skip_bits(gb, 8);
size--;
}

skip_bits(gb, 8);
skip_bits(gb, 8);

user_identifier = get_bits_long(gb, 32);

switch (user_identifier) {
case MKBETAG('G', 'A', '9', '4'):
return decode_registered_user_data_closed_caption(&s->a53_caption, gb, size);
default:
skip_bits_long(gb, size * 8);
break;
}
return 0;
}

static int decode_nal_sei_active_parameter_sets(HEVCSEI *s, GetBitContext *gb, void *logctx)
{
int num_sps_ids_minus1;
int i;
unsigned active_seq_parameter_set_id;

get_bits(gb, 4); 
get_bits(gb, 1); 
get_bits(gb, 1); 
num_sps_ids_minus1 = get_ue_golomb_long(gb); 

if (num_sps_ids_minus1 < 0 || num_sps_ids_minus1 > 15) {
av_log(logctx, AV_LOG_ERROR, "num_sps_ids_minus1 %d invalid\n", num_sps_ids_minus1);
return AVERROR_INVALIDDATA;
}

active_seq_parameter_set_id = get_ue_golomb_long(gb);
if (active_seq_parameter_set_id >= HEVC_MAX_SPS_COUNT) {
av_log(logctx, AV_LOG_ERROR, "active_parameter_set_id %d invalid\n", active_seq_parameter_set_id);
return AVERROR_INVALIDDATA;
}
s->active_seq_parameter_set_id = active_seq_parameter_set_id;

for (i = 1; i <= num_sps_ids_minus1; i++)
get_ue_golomb_long(gb); 

return 0;
}

static int decode_nal_sei_alternative_transfer(HEVCSEIAlternativeTransfer *s, GetBitContext *gb)
{
s->present = 1;
s->preferred_transfer_characteristics = get_bits(gb, 8);
return 0;
}

static int decode_nal_sei_prefix(GetBitContext *gb, void *logctx, HEVCSEI *s,
const HEVCParamSets *ps, int type, int size)
{
switch (type) {
case 256: 
return decode_nal_sei_decoded_picture_hash(&s->picture_hash, gb);
case HEVC_SEI_TYPE_FRAME_PACKING:
return decode_nal_sei_frame_packing_arrangement(&s->frame_packing, gb);
case HEVC_SEI_TYPE_DISPLAY_ORIENTATION:
return decode_nal_sei_display_orientation(&s->display_orientation, gb);
case HEVC_SEI_TYPE_PICTURE_TIMING:
return decode_nal_sei_pic_timing(s, gb, ps, logctx, size);
case HEVC_SEI_TYPE_MASTERING_DISPLAY_INFO:
return decode_nal_sei_mastering_display_info(&s->mastering_display, gb);
case HEVC_SEI_TYPE_CONTENT_LIGHT_LEVEL_INFO:
return decode_nal_sei_content_light_info(&s->content_light, gb);
case HEVC_SEI_TYPE_ACTIVE_PARAMETER_SETS:
return decode_nal_sei_active_parameter_sets(s, gb, logctx);
case HEVC_SEI_TYPE_USER_DATA_REGISTERED_ITU_T_T35:
return decode_nal_sei_user_data_registered_itu_t_t35(s, gb, size);
case HEVC_SEI_TYPE_ALTERNATIVE_TRANSFER_CHARACTERISTICS:
return decode_nal_sei_alternative_transfer(&s->alternative_transfer, gb);
default:
av_log(logctx, AV_LOG_DEBUG, "Skipped PREFIX SEI %d\n", type);
skip_bits_long(gb, 8 * size);
return 0;
}
}

static int decode_nal_sei_suffix(GetBitContext *gb, void *logctx, HEVCSEI *s,
int type, int size)
{
switch (type) {
case HEVC_SEI_TYPE_DECODED_PICTURE_HASH:
return decode_nal_sei_decoded_picture_hash(&s->picture_hash, gb);
default:
av_log(logctx, AV_LOG_DEBUG, "Skipped SUFFIX SEI %d\n", type);
skip_bits_long(gb, 8 * size);
return 0;
}
}

static int decode_nal_sei_message(GetBitContext *gb, void *logctx, HEVCSEI *s,
const HEVCParamSets *ps, int nal_unit_type)
{
int payload_type = 0;
int payload_size = 0;
int byte = 0xFF;
av_log(logctx, AV_LOG_DEBUG, "Decoding SEI\n");

while (byte == 0xFF) {
if (get_bits_left(gb) < 16 || payload_type > INT_MAX - 255)
return AVERROR_INVALIDDATA;
byte = get_bits(gb, 8);
payload_type += byte;
}
byte = 0xFF;
while (byte == 0xFF) {
if (get_bits_left(gb) < 8 + 8LL*payload_size)
return AVERROR_INVALIDDATA;
byte = get_bits(gb, 8);
payload_size += byte;
}
if (nal_unit_type == HEVC_NAL_SEI_PREFIX) {
return decode_nal_sei_prefix(gb, logctx, s, ps, payload_type, payload_size);
} else { 
return decode_nal_sei_suffix(gb, logctx, s, payload_type, payload_size);
}
}

static int more_rbsp_data(GetBitContext *gb)
{
return get_bits_left(gb) > 0 && show_bits(gb, 8) != 0x80;
}

int ff_hevc_decode_nal_sei(GetBitContext *gb, void *logctx, HEVCSEI *s,
const HEVCParamSets *ps, int type)
{
int ret;

do {
ret = decode_nal_sei_message(gb, logctx, s, ps, type);
if (ret < 0)
return ret;
} while (more_rbsp_data(gb));
return 1;
}

void ff_hevc_reset_sei(HEVCSEI *s)
{
av_buffer_unref(&s->a53_caption.buf_ref);
}
