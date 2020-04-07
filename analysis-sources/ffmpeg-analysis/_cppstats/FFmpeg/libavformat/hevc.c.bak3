



















#include "libavcodec/avcodec.h"
#include "libavcodec/get_bits.h"
#include "libavcodec/golomb.h"
#include "libavcodec/hevc.h"
#include "libavutil/intreadwrite.h"
#include "avc.h"
#include "avio.h"
#include "avio_internal.h"
#include "hevc.h"

#define MAX_SPATIAL_SEGMENTATION 4096 

typedef struct HVCCNALUnitArray {
uint8_t array_completeness;
uint8_t NAL_unit_type;
uint16_t numNalus;
uint16_t *nalUnitLength;
uint8_t **nalUnit;
} HVCCNALUnitArray;

typedef struct HEVCDecoderConfigurationRecord {
uint8_t configurationVersion;
uint8_t general_profile_space;
uint8_t general_tier_flag;
uint8_t general_profile_idc;
uint32_t general_profile_compatibility_flags;
uint64_t general_constraint_indicator_flags;
uint8_t general_level_idc;
uint16_t min_spatial_segmentation_idc;
uint8_t parallelismType;
uint8_t chromaFormat;
uint8_t bitDepthLumaMinus8;
uint8_t bitDepthChromaMinus8;
uint16_t avgFrameRate;
uint8_t constantFrameRate;
uint8_t numTemporalLayers;
uint8_t temporalIdNested;
uint8_t lengthSizeMinusOne;
uint8_t numOfArrays;
HVCCNALUnitArray *array;
} HEVCDecoderConfigurationRecord;

typedef struct HVCCProfileTierLevel {
uint8_t profile_space;
uint8_t tier_flag;
uint8_t profile_idc;
uint32_t profile_compatibility_flags;
uint64_t constraint_indicator_flags;
uint8_t level_idc;
} HVCCProfileTierLevel;

static void hvcc_update_ptl(HEVCDecoderConfigurationRecord *hvcc,
HVCCProfileTierLevel *ptl)
{




hvcc->general_profile_space = ptl->profile_space;






if (hvcc->general_tier_flag < ptl->tier_flag)
hvcc->general_level_idc = ptl->level_idc;
else
hvcc->general_level_idc = FFMAX(hvcc->general_level_idc, ptl->level_idc);





hvcc->general_tier_flag = FFMAX(hvcc->general_tier_flag, ptl->tier_flag);















hvcc->general_profile_idc = FFMAX(hvcc->general_profile_idc, ptl->profile_idc);





hvcc->general_profile_compatibility_flags &= ptl->profile_compatibility_flags;





hvcc->general_constraint_indicator_flags &= ptl->constraint_indicator_flags;
}

static void hvcc_parse_ptl(GetBitContext *gb,
HEVCDecoderConfigurationRecord *hvcc,
unsigned int max_sub_layers_minus1)
{
unsigned int i;
HVCCProfileTierLevel general_ptl;
uint8_t sub_layer_profile_present_flag[HEVC_MAX_SUB_LAYERS];
uint8_t sub_layer_level_present_flag[HEVC_MAX_SUB_LAYERS];

general_ptl.profile_space = get_bits(gb, 2);
general_ptl.tier_flag = get_bits1(gb);
general_ptl.profile_idc = get_bits(gb, 5);
general_ptl.profile_compatibility_flags = get_bits_long(gb, 32);
general_ptl.constraint_indicator_flags = get_bits64(gb, 48);
general_ptl.level_idc = get_bits(gb, 8);
hvcc_update_ptl(hvcc, &general_ptl);

for (i = 0; i < max_sub_layers_minus1; i++) {
sub_layer_profile_present_flag[i] = get_bits1(gb);
sub_layer_level_present_flag[i] = get_bits1(gb);
}

if (max_sub_layers_minus1 > 0)
for (i = max_sub_layers_minus1; i < 8; i++)
skip_bits(gb, 2); 

for (i = 0; i < max_sub_layers_minus1; i++) {
if (sub_layer_profile_present_flag[i]) {











skip_bits_long(gb, 32);
skip_bits_long(gb, 32);
skip_bits (gb, 24);
}

if (sub_layer_level_present_flag[i])
skip_bits(gb, 8);
}
}

static void skip_sub_layer_hrd_parameters(GetBitContext *gb,
unsigned int cpb_cnt_minus1,
uint8_t sub_pic_hrd_params_present_flag)
{
unsigned int i;

for (i = 0; i <= cpb_cnt_minus1; i++) {
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 

if (sub_pic_hrd_params_present_flag) {
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
}

skip_bits1(gb); 
}
}

static int skip_hrd_parameters(GetBitContext *gb, uint8_t cprms_present_flag,
unsigned int max_sub_layers_minus1)
{
unsigned int i;
uint8_t sub_pic_hrd_params_present_flag = 0;
uint8_t nal_hrd_parameters_present_flag = 0;
uint8_t vcl_hrd_parameters_present_flag = 0;

if (cprms_present_flag) {
nal_hrd_parameters_present_flag = get_bits1(gb);
vcl_hrd_parameters_present_flag = get_bits1(gb);

if (nal_hrd_parameters_present_flag ||
vcl_hrd_parameters_present_flag) {
sub_pic_hrd_params_present_flag = get_bits1(gb);

if (sub_pic_hrd_params_present_flag)






skip_bits(gb, 19);





skip_bits(gb, 8);

if (sub_pic_hrd_params_present_flag)
skip_bits(gb, 4); 






skip_bits(gb, 15);
}
}

for (i = 0; i <= max_sub_layers_minus1; i++) {
unsigned int cpb_cnt_minus1 = 0;
uint8_t low_delay_hrd_flag = 0;
uint8_t fixed_pic_rate_within_cvs_flag = 0;
uint8_t fixed_pic_rate_general_flag = get_bits1(gb);

if (!fixed_pic_rate_general_flag)
fixed_pic_rate_within_cvs_flag = get_bits1(gb);

if (fixed_pic_rate_within_cvs_flag)
get_ue_golomb_long(gb); 
else
low_delay_hrd_flag = get_bits1(gb);

if (!low_delay_hrd_flag) {
cpb_cnt_minus1 = get_ue_golomb_long(gb);
if (cpb_cnt_minus1 > 31)
return AVERROR_INVALIDDATA;
}

if (nal_hrd_parameters_present_flag)
skip_sub_layer_hrd_parameters(gb, cpb_cnt_minus1,
sub_pic_hrd_params_present_flag);

if (vcl_hrd_parameters_present_flag)
skip_sub_layer_hrd_parameters(gb, cpb_cnt_minus1,
sub_pic_hrd_params_present_flag);
}

return 0;
}

static void skip_timing_info(GetBitContext *gb)
{
skip_bits_long(gb, 32); 
skip_bits_long(gb, 32); 

if (get_bits1(gb)) 
get_ue_golomb_long(gb); 
}

static void hvcc_parse_vui(GetBitContext *gb,
HEVCDecoderConfigurationRecord *hvcc,
unsigned int max_sub_layers_minus1)
{
unsigned int min_spatial_segmentation_idc;

if (get_bits1(gb)) 
if (get_bits(gb, 8) == 255) 
skip_bits_long(gb, 32); 

if (get_bits1(gb)) 
skip_bits1(gb); 

if (get_bits1(gb)) { 
skip_bits(gb, 4); 

if (get_bits1(gb)) 





skip_bits(gb, 24);
}

if (get_bits1(gb)) { 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
}






skip_bits(gb, 3);

if (get_bits1(gb)) { 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
}

if (get_bits1(gb)) { 
skip_timing_info(gb);

if (get_bits1(gb)) 
skip_hrd_parameters(gb, 1, max_sub_layers_minus1);
}

if (get_bits1(gb)) { 





skip_bits(gb, 3);

min_spatial_segmentation_idc = get_ue_golomb_long(gb);








hvcc->min_spatial_segmentation_idc = FFMIN(hvcc->min_spatial_segmentation_idc,
min_spatial_segmentation_idc);

get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
}
}

static void skip_sub_layer_ordering_info(GetBitContext *gb)
{
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
}

static int hvcc_parse_vps(GetBitContext *gb,
HEVCDecoderConfigurationRecord *hvcc)
{
unsigned int vps_max_sub_layers_minus1;






skip_bits(gb, 12);

vps_max_sub_layers_minus1 = get_bits(gb, 3);









hvcc->numTemporalLayers = FFMAX(hvcc->numTemporalLayers,
vps_max_sub_layers_minus1 + 1);





skip_bits(gb, 17);

hvcc_parse_ptl(gb, hvcc, vps_max_sub_layers_minus1);


return 0;
}

static void skip_scaling_list_data(GetBitContext *gb)
{
int i, j, k, num_coeffs;

for (i = 0; i < 4; i++)
for (j = 0; j < (i == 3 ? 2 : 6); j++)
if (!get_bits1(gb)) 
get_ue_golomb_long(gb); 
else {
num_coeffs = FFMIN(64, 1 << (4 + (i << 1)));

if (i > 1)
get_se_golomb_long(gb); 

for (k = 0; k < num_coeffs; k++)
get_se_golomb_long(gb); 
}
}

static int parse_rps(GetBitContext *gb, unsigned int rps_idx,
unsigned int num_rps,
unsigned int num_delta_pocs[HEVC_MAX_SHORT_TERM_REF_PIC_SETS])
{
unsigned int i;

if (rps_idx && get_bits1(gb)) { 

if (rps_idx >= num_rps)
return AVERROR_INVALIDDATA;

skip_bits1 (gb); 
get_ue_golomb_long(gb); 

num_delta_pocs[rps_idx] = 0;




















for (i = 0; i <= num_delta_pocs[rps_idx - 1]; i++) {
uint8_t use_delta_flag = 0;
uint8_t used_by_curr_pic_flag = get_bits1(gb);
if (!used_by_curr_pic_flag)
use_delta_flag = get_bits1(gb);

if (used_by_curr_pic_flag || use_delta_flag)
num_delta_pocs[rps_idx]++;
}
} else {
unsigned int num_negative_pics = get_ue_golomb_long(gb);
unsigned int num_positive_pics = get_ue_golomb_long(gb);

if ((num_positive_pics + (uint64_t)num_negative_pics) * 2 > get_bits_left(gb))
return AVERROR_INVALIDDATA;

num_delta_pocs[rps_idx] = num_negative_pics + num_positive_pics;

for (i = 0; i < num_negative_pics; i++) {
get_ue_golomb_long(gb); 
skip_bits1 (gb); 
}

for (i = 0; i < num_positive_pics; i++) {
get_ue_golomb_long(gb); 
skip_bits1 (gb); 
}
}

return 0;
}

static int hvcc_parse_sps(GetBitContext *gb,
HEVCDecoderConfigurationRecord *hvcc)
{
unsigned int i, sps_max_sub_layers_minus1, log2_max_pic_order_cnt_lsb_minus4;
unsigned int num_short_term_ref_pic_sets, num_delta_pocs[HEVC_MAX_SHORT_TERM_REF_PIC_SETS];

skip_bits(gb, 4); 

sps_max_sub_layers_minus1 = get_bits (gb, 3);









hvcc->numTemporalLayers = FFMAX(hvcc->numTemporalLayers,
sps_max_sub_layers_minus1 + 1);

hvcc->temporalIdNested = get_bits1(gb);

hvcc_parse_ptl(gb, hvcc, sps_max_sub_layers_minus1);

get_ue_golomb_long(gb); 

hvcc->chromaFormat = get_ue_golomb_long(gb);

if (hvcc->chromaFormat == 3)
skip_bits1(gb); 

get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 

if (get_bits1(gb)) { 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
}

hvcc->bitDepthLumaMinus8 = get_ue_golomb_long(gb);
hvcc->bitDepthChromaMinus8 = get_ue_golomb_long(gb);
log2_max_pic_order_cnt_lsb_minus4 = get_ue_golomb_long(gb);


i = get_bits1(gb) ? 0 : sps_max_sub_layers_minus1;
for (; i <= sps_max_sub_layers_minus1; i++)
skip_sub_layer_ordering_info(gb);

get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 

if (get_bits1(gb) && 
get_bits1(gb)) 
skip_scaling_list_data(gb);

skip_bits1(gb); 
skip_bits1(gb); 

if (get_bits1(gb)) { 
skip_bits (gb, 4); 
skip_bits (gb, 4); 
get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
skip_bits1 (gb); 
}

num_short_term_ref_pic_sets = get_ue_golomb_long(gb);
if (num_short_term_ref_pic_sets > HEVC_MAX_SHORT_TERM_REF_PIC_SETS)
return AVERROR_INVALIDDATA;

for (i = 0; i < num_short_term_ref_pic_sets; i++) {
int ret = parse_rps(gb, i, num_short_term_ref_pic_sets, num_delta_pocs);
if (ret < 0)
return ret;
}

if (get_bits1(gb)) { 
unsigned num_long_term_ref_pics_sps = get_ue_golomb_long(gb);
if (num_long_term_ref_pics_sps > 31U)
return AVERROR_INVALIDDATA;
for (i = 0; i < num_long_term_ref_pics_sps; i++) { 
int len = FFMIN(log2_max_pic_order_cnt_lsb_minus4 + 4, 16);
skip_bits (gb, len); 
skip_bits1(gb); 
}
}

skip_bits1(gb); 
skip_bits1(gb); 

if (get_bits1(gb)) 
hvcc_parse_vui(gb, hvcc, sps_max_sub_layers_minus1);


return 0;
}

static int hvcc_parse_pps(GetBitContext *gb,
HEVCDecoderConfigurationRecord *hvcc)
{
uint8_t tiles_enabled_flag, entropy_coding_sync_enabled_flag;

get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 








skip_bits(gb, 7);

get_ue_golomb_long(gb); 
get_ue_golomb_long(gb); 
get_se_golomb_long(gb); 





skip_bits(gb, 2);

if (get_bits1(gb)) 
get_ue_golomb_long(gb); 

get_se_golomb_long(gb); 
get_se_golomb_long(gb); 







skip_bits(gb, 4);

tiles_enabled_flag = get_bits1(gb);
entropy_coding_sync_enabled_flag = get_bits1(gb);

if (entropy_coding_sync_enabled_flag && tiles_enabled_flag)
hvcc->parallelismType = 0; 
else if (entropy_coding_sync_enabled_flag)
hvcc->parallelismType = 3; 
else if (tiles_enabled_flag)
hvcc->parallelismType = 2; 
else
hvcc->parallelismType = 1; 


return 0;
}

static void nal_unit_parse_header(GetBitContext *gb, uint8_t *nal_type)
{
skip_bits1(gb); 

*nal_type = get_bits(gb, 6);





skip_bits(gb, 9);
}

static int hvcc_array_add_nal_unit(uint8_t *nal_buf, uint32_t nal_size,
uint8_t nal_type, int ps_array_completeness,
HEVCDecoderConfigurationRecord *hvcc)
{
int ret;
uint8_t index;
uint16_t numNalus;
HVCCNALUnitArray *array;

for (index = 0; index < hvcc->numOfArrays; index++)
if (hvcc->array[index].NAL_unit_type == nal_type)
break;

if (index >= hvcc->numOfArrays) {
uint8_t i;

ret = av_reallocp_array(&hvcc->array, index + 1, sizeof(HVCCNALUnitArray));
if (ret < 0)
return ret;

for (i = hvcc->numOfArrays; i <= index; i++)
memset(&hvcc->array[i], 0, sizeof(HVCCNALUnitArray));
hvcc->numOfArrays = index + 1;
}

array = &hvcc->array[index];
numNalus = array->numNalus;

ret = av_reallocp_array(&array->nalUnit, numNalus + 1, sizeof(uint8_t*));
if (ret < 0)
return ret;

ret = av_reallocp_array(&array->nalUnitLength, numNalus + 1, sizeof(uint16_t));
if (ret < 0)
return ret;

array->nalUnit [numNalus] = nal_buf;
array->nalUnitLength[numNalus] = nal_size;
array->NAL_unit_type = nal_type;
array->numNalus++;







if (nal_type == HEVC_NAL_VPS || nal_type == HEVC_NAL_SPS || nal_type == HEVC_NAL_PPS)
array->array_completeness = ps_array_completeness;

return 0;
}

static int hvcc_add_nal_unit(uint8_t *nal_buf, uint32_t nal_size,
int ps_array_completeness,
HEVCDecoderConfigurationRecord *hvcc)
{
int ret = 0;
GetBitContext gbc;
uint8_t nal_type;
uint8_t *rbsp_buf;
uint32_t rbsp_size;

rbsp_buf = ff_nal_unit_extract_rbsp(nal_buf, nal_size, &rbsp_size, 2);
if (!rbsp_buf) {
ret = AVERROR(ENOMEM);
goto end;
}

ret = init_get_bits8(&gbc, rbsp_buf, rbsp_size);
if (ret < 0)
goto end;

nal_unit_parse_header(&gbc, &nal_type);






switch (nal_type) {
case HEVC_NAL_VPS:
case HEVC_NAL_SPS:
case HEVC_NAL_PPS:
case HEVC_NAL_SEI_PREFIX:
case HEVC_NAL_SEI_SUFFIX:
ret = hvcc_array_add_nal_unit(nal_buf, nal_size, nal_type,
ps_array_completeness, hvcc);
if (ret < 0)
goto end;
else if (nal_type == HEVC_NAL_VPS)
ret = hvcc_parse_vps(&gbc, hvcc);
else if (nal_type == HEVC_NAL_SPS)
ret = hvcc_parse_sps(&gbc, hvcc);
else if (nal_type == HEVC_NAL_PPS)
ret = hvcc_parse_pps(&gbc, hvcc);
if (ret < 0)
goto end;
break;
default:
ret = AVERROR_INVALIDDATA;
goto end;
}

end:
av_free(rbsp_buf);
return ret;
}

static void hvcc_init(HEVCDecoderConfigurationRecord *hvcc)
{
memset(hvcc, 0, sizeof(HEVCDecoderConfigurationRecord));
hvcc->configurationVersion = 1;
hvcc->lengthSizeMinusOne = 3; 





hvcc->general_profile_compatibility_flags = 0xffffffff;
hvcc->general_constraint_indicator_flags = 0xffffffffffff;





hvcc->min_spatial_segmentation_idc = MAX_SPATIAL_SEGMENTATION + 1;
}

static void hvcc_close(HEVCDecoderConfigurationRecord *hvcc)
{
uint8_t i;

for (i = 0; i < hvcc->numOfArrays; i++) {
hvcc->array[i].numNalus = 0;
av_freep(&hvcc->array[i].nalUnit);
av_freep(&hvcc->array[i].nalUnitLength);
}

hvcc->numOfArrays = 0;
av_freep(&hvcc->array);
}

static int hvcc_write(AVIOContext *pb, HEVCDecoderConfigurationRecord *hvcc)
{
uint8_t i;
uint16_t j, vps_count = 0, sps_count = 0, pps_count = 0;




hvcc->configurationVersion = 1;




if (hvcc->min_spatial_segmentation_idc > MAX_SPATIAL_SEGMENTATION)
hvcc->min_spatial_segmentation_idc = 0;






if (!hvcc->min_spatial_segmentation_idc)
hvcc->parallelismType = 0;





hvcc->avgFrameRate = 0;
hvcc->constantFrameRate = 0;

av_log(NULL, AV_LOG_TRACE, "configurationVersion: %"PRIu8"\n",
hvcc->configurationVersion);
av_log(NULL, AV_LOG_TRACE, "general_profile_space: %"PRIu8"\n",
hvcc->general_profile_space);
av_log(NULL, AV_LOG_TRACE, "general_tier_flag: %"PRIu8"\n",
hvcc->general_tier_flag);
av_log(NULL, AV_LOG_TRACE, "general_profile_idc: %"PRIu8"\n",
hvcc->general_profile_idc);
av_log(NULL, AV_LOG_TRACE, "general_profile_compatibility_flags: 0x%08"PRIx32"\n",
hvcc->general_profile_compatibility_flags);
av_log(NULL, AV_LOG_TRACE, "general_constraint_indicator_flags: 0x%012"PRIx64"\n",
hvcc->general_constraint_indicator_flags);
av_log(NULL, AV_LOG_TRACE, "general_level_idc: %"PRIu8"\n",
hvcc->general_level_idc);
av_log(NULL, AV_LOG_TRACE, "min_spatial_segmentation_idc: %"PRIu16"\n",
hvcc->min_spatial_segmentation_idc);
av_log(NULL, AV_LOG_TRACE, "parallelismType: %"PRIu8"\n",
hvcc->parallelismType);
av_log(NULL, AV_LOG_TRACE, "chromaFormat: %"PRIu8"\n",
hvcc->chromaFormat);
av_log(NULL, AV_LOG_TRACE, "bitDepthLumaMinus8: %"PRIu8"\n",
hvcc->bitDepthLumaMinus8);
av_log(NULL, AV_LOG_TRACE, "bitDepthChromaMinus8: %"PRIu8"\n",
hvcc->bitDepthChromaMinus8);
av_log(NULL, AV_LOG_TRACE, "avgFrameRate: %"PRIu16"\n",
hvcc->avgFrameRate);
av_log(NULL, AV_LOG_TRACE, "constantFrameRate: %"PRIu8"\n",
hvcc->constantFrameRate);
av_log(NULL, AV_LOG_TRACE, "numTemporalLayers: %"PRIu8"\n",
hvcc->numTemporalLayers);
av_log(NULL, AV_LOG_TRACE, "temporalIdNested: %"PRIu8"\n",
hvcc->temporalIdNested);
av_log(NULL, AV_LOG_TRACE, "lengthSizeMinusOne: %"PRIu8"\n",
hvcc->lengthSizeMinusOne);
av_log(NULL, AV_LOG_TRACE, "numOfArrays: %"PRIu8"\n",
hvcc->numOfArrays);
for (i = 0; i < hvcc->numOfArrays; i++) {
av_log(NULL, AV_LOG_TRACE, "array_completeness[%"PRIu8"]: %"PRIu8"\n",
i, hvcc->array[i].array_completeness);
av_log(NULL, AV_LOG_TRACE, "NAL_unit_type[%"PRIu8"]: %"PRIu8"\n",
i, hvcc->array[i].NAL_unit_type);
av_log(NULL, AV_LOG_TRACE, "numNalus[%"PRIu8"]: %"PRIu16"\n",
i, hvcc->array[i].numNalus);
for (j = 0; j < hvcc->array[i].numNalus; j++)
av_log(NULL, AV_LOG_TRACE,
"nalUnitLength[%"PRIu8"][%"PRIu16"]: %"PRIu16"\n",
i, j, hvcc->array[i].nalUnitLength[j]);
}




for (i = 0; i < hvcc->numOfArrays; i++)
switch (hvcc->array[i].NAL_unit_type) {
case HEVC_NAL_VPS:
vps_count += hvcc->array[i].numNalus;
break;
case HEVC_NAL_SPS:
sps_count += hvcc->array[i].numNalus;
break;
case HEVC_NAL_PPS:
pps_count += hvcc->array[i].numNalus;
break;
default:
break;
}
if (!vps_count || vps_count > HEVC_MAX_VPS_COUNT ||
!sps_count || sps_count > HEVC_MAX_SPS_COUNT ||
!pps_count || pps_count > HEVC_MAX_PPS_COUNT)
return AVERROR_INVALIDDATA;


avio_w8(pb, hvcc->configurationVersion);






avio_w8(pb, hvcc->general_profile_space << 6 |
hvcc->general_tier_flag << 5 |
hvcc->general_profile_idc);


avio_wb32(pb, hvcc->general_profile_compatibility_flags);


avio_wb32(pb, hvcc->general_constraint_indicator_flags >> 16);
avio_wb16(pb, hvcc->general_constraint_indicator_flags);


avio_w8(pb, hvcc->general_level_idc);





avio_wb16(pb, hvcc->min_spatial_segmentation_idc | 0xf000);





avio_w8(pb, hvcc->parallelismType | 0xfc);





avio_w8(pb, hvcc->chromaFormat | 0xfc);





avio_w8(pb, hvcc->bitDepthLumaMinus8 | 0xf8);





avio_w8(pb, hvcc->bitDepthChromaMinus8 | 0xf8);


avio_wb16(pb, hvcc->avgFrameRate);







avio_w8(pb, hvcc->constantFrameRate << 6 |
hvcc->numTemporalLayers << 3 |
hvcc->temporalIdNested << 2 |
hvcc->lengthSizeMinusOne);


avio_w8(pb, hvcc->numOfArrays);

for (i = 0; i < hvcc->numOfArrays; i++) {





avio_w8(pb, hvcc->array[i].array_completeness << 7 |
hvcc->array[i].NAL_unit_type & 0x3f);


avio_wb16(pb, hvcc->array[i].numNalus);

for (j = 0; j < hvcc->array[i].numNalus; j++) {

avio_wb16(pb, hvcc->array[i].nalUnitLength[j]);


avio_write(pb, hvcc->array[i].nalUnit[j],
hvcc->array[i].nalUnitLength[j]);
}
}

return 0;
}

int ff_hevc_annexb2mp4(AVIOContext *pb, const uint8_t *buf_in,
int size, int filter_ps, int *ps_count)
{
int num_ps = 0, ret = 0;
uint8_t *buf, *end, *start = NULL;

if (!filter_ps) {
ret = ff_avc_parse_nal_units(pb, buf_in, size);
goto end;
}

ret = ff_avc_parse_nal_units_buf(buf_in, &start, &size);
if (ret < 0)
goto end;

ret = 0;
buf = start;
end = start + size;

while (end - buf > 4) {
uint32_t len = FFMIN(AV_RB32(buf), end - buf - 4);
uint8_t type = (buf[4] >> 1) & 0x3f;

buf += 4;

switch (type) {
case HEVC_NAL_VPS:
case HEVC_NAL_SPS:
case HEVC_NAL_PPS:
num_ps++;
break;
default:
ret += 4 + len;
avio_wb32(pb, len);
avio_write(pb, buf, len);
break;
}

buf += len;
}

end:
av_free(start);
if (ps_count)
*ps_count = num_ps;
return ret;
}

int ff_hevc_annexb2mp4_buf(const uint8_t *buf_in, uint8_t **buf_out,
int *size, int filter_ps, int *ps_count)
{
AVIOContext *pb;
int ret;

ret = avio_open_dyn_buf(&pb);
if (ret < 0)
return ret;

ret = ff_hevc_annexb2mp4(pb, buf_in, *size, filter_ps, ps_count);
if (ret < 0) {
ffio_free_dyn_buf(&pb);
return ret;
}

*size = avio_close_dyn_buf(pb, buf_out);

return 0;
}

int ff_isom_write_hvcc(AVIOContext *pb, const uint8_t *data,
int size, int ps_array_completeness)
{
int ret = 0;
uint8_t *buf, *end, *start = NULL;
HEVCDecoderConfigurationRecord hvcc;

hvcc_init(&hvcc);

if (size < 6) {

ret = AVERROR_INVALIDDATA;
goto end;
} else if (*data == 1) {

avio_write(pb, data, size);
goto end;
} else if (!(AV_RB24(data) == 1 || AV_RB32(data) == 1)) {

ret = AVERROR_INVALIDDATA;
goto end;
}

ret = ff_avc_parse_nal_units_buf(data, &start, &size);
if (ret < 0)
goto end;

buf = start;
end = start + size;

while (end - buf > 4) {
uint32_t len = FFMIN(AV_RB32(buf), end - buf - 4);
uint8_t type = (buf[4] >> 1) & 0x3f;

buf += 4;

switch (type) {
case HEVC_NAL_VPS:
case HEVC_NAL_SPS:
case HEVC_NAL_PPS:
case HEVC_NAL_SEI_PREFIX:
case HEVC_NAL_SEI_SUFFIX:
ret = hvcc_add_nal_unit(buf, len, ps_array_completeness, &hvcc);
if (ret < 0)
goto end;
break;
default:
break;
}

buf += len;
}

ret = hvcc_write(pb, &hvcc);

end:
hvcc_close(&hvcc);
av_free(start);
return ret;
}
