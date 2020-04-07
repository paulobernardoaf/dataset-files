#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_bits.h>
#include "h264_nal.h"
#include "h264_slice.h"
#include "hxxx_nal.h"
#include "hxxx_ep3b.h"
bool h264_decode_slice( const uint8_t *p_buffer, size_t i_buffer,
void (* get_sps_pps)(uint8_t, void *,
const h264_sequence_parameter_set_t **,
const h264_picture_parameter_set_t ** ),
void *priv, h264_slice_t *p_slice )
{
int i_slice_type;
h264_slice_init( p_slice );
bs_t s;
struct hxxx_bsfw_ep3b_ctx_s bsctx;
hxxx_bsfw_ep3b_ctx_init( &bsctx );
bs_init_custom( &s, p_buffer, i_buffer, &hxxx_bsfw_ep3b_callbacks, &bsctx );
bs_skip( &s, 1 );
const uint8_t i_nal_ref_idc = bs_read( &s, 2 );
const uint8_t i_nal_type = bs_read( &s, 5 );
bs_read_ue( &s );
i_slice_type = bs_read_ue( &s );
p_slice->type = i_slice_type % 5;
p_slice->i_nal_type = i_nal_type;
p_slice->i_nal_ref_idc = i_nal_ref_idc;
p_slice->i_pic_parameter_set_id = bs_read_ue( &s );
if( p_slice->i_pic_parameter_set_id > H264_PPS_ID_MAX )
return false;
const h264_sequence_parameter_set_t *p_sps;
const h264_picture_parameter_set_t *p_pps;
get_sps_pps( p_slice->i_pic_parameter_set_id, priv, &p_sps, &p_pps );
if( !p_sps || !p_pps )
return false;
p_slice->i_frame_num = bs_read( &s, p_sps->i_log2_max_frame_num + 4 );
if( !p_sps->frame_mbs_only_flag )
{
p_slice->i_field_pic_flag = bs_read( &s, 1 );
if( p_slice->i_field_pic_flag )
p_slice->i_bottom_field_flag = bs_read( &s, 1 );
}
if( p_slice->i_nal_type == H264_NAL_SLICE_IDR )
p_slice->i_idr_pic_id = bs_read_ue( &s );
p_slice->i_pic_order_cnt_type = p_sps->i_pic_order_cnt_type;
if( p_sps->i_pic_order_cnt_type == 0 )
{
p_slice->i_pic_order_cnt_lsb = bs_read( &s, p_sps->i_log2_max_pic_order_cnt_lsb + 4 );
if( p_pps->i_pic_order_present_flag && !p_slice->i_field_pic_flag )
p_slice->i_delta_pic_order_cnt_bottom = bs_read_se( &s );
}
else if( (p_sps->i_pic_order_cnt_type == 1) &&
(!p_sps->i_delta_pic_order_always_zero_flag) )
{
p_slice->i_delta_pic_order_cnt0 = bs_read_se( &s );
if( p_pps->i_pic_order_present_flag && !p_slice->i_field_pic_flag )
p_slice->i_delta_pic_order_cnt1 = bs_read_se( &s );
}
if( p_pps->i_redundant_pic_present_flag )
bs_read_ue( &s ); 
unsigned num_ref_idx_l01_active_minus1[2] = {0 , 0};
if( i_slice_type == 1 || i_slice_type == 6 ) 
bs_read1( &s ); 
if( i_slice_type == 0 || i_slice_type == 5 ||
i_slice_type == 3 || i_slice_type == 8 ||
i_slice_type == 1 || i_slice_type == 6 ) 
{
if( bs_read1( &s ) ) 
{
num_ref_idx_l01_active_minus1[0] = bs_read_ue( &s );
if( i_slice_type == 1 || i_slice_type == 6 ) 
num_ref_idx_l01_active_minus1[1] = bs_read_ue( &s );
}
}
if( p_slice->i_nal_type == 5 || p_slice->i_nal_ref_idc == 0 )
{
p_slice->has_mmco5 = false;
return true;
}
const bool b_mvc = (p_slice->i_nal_type == 20 || p_slice->i_nal_type == 21 );
unsigned i = 0;
if( i_slice_type % 5 != 2 && i_slice_type % 5 != 4 )
i++;
if( i_slice_type % 5 == 1 )
i++;
for( ; i>0; i-- )
{
if( bs_read1( &s ) ) 
{
uint32_t mod;
do
{
mod = bs_read_ue( &s );
if( mod < 3 || ( b_mvc && (mod == 4 || mod == 5) ) )
bs_read_ue( &s ); 
}
while( mod != 3 && bs_remain( &s ) );
}
}
if( ( p_pps->weighted_pred_flag && ( i_slice_type == 0 || i_slice_type == 5 || 
i_slice_type == 3 || i_slice_type == 8 ) ) ||
( p_pps->weighted_bipred_idc == 1 && ( i_slice_type == 1 || i_slice_type == 6 ) ) )
{
bs_read_ue( &s ); 
if( !p_sps->b_separate_colour_planes_flag ) 
bs_read_ue( &s ); 
const unsigned i_num_layers = ( i_slice_type % 5 == 1 ) ? 2 : 1;
for( unsigned j=0; j < i_num_layers; j++ )
{
for( unsigned k=0; k<=num_ref_idx_l01_active_minus1[j]; k++ )
{
if( bs_read1( &s ) ) 
{
bs_read_se( &s );
bs_read_se( &s );
}
if( !p_sps->b_separate_colour_planes_flag ) 
{
if( bs_read1( &s ) ) 
{
bs_read_se( &s );
bs_read_se( &s );
bs_read_se( &s );
bs_read_se( &s );
}
}
}
}
}
if( p_slice->i_nal_type != 5 ) 
{
if( bs_read1( &s ) ) 
{
uint32_t mmco;
do
{
mmco = bs_read_ue( &s );
if( mmco == 1 || mmco == 3 )
bs_read_ue( &s ); 
if( mmco == 2 )
bs_read_ue( &s ); 
if( mmco == 3 || mmco == 6 )
bs_read_ue( &s ); 
if( mmco == 4 )
bs_read_ue( &s ); 
if( mmco == 5 )
{
p_slice->has_mmco5 = true;
break; 
}
}
while( mmco > 0 );
}
}
return true;
}
void h264_compute_poc( const h264_sequence_parameter_set_t *p_sps,
const h264_slice_t *p_slice, h264_poc_context_t *p_ctx,
int *p_PictureOrderCount, int *p_tFOC, int *p_bFOC )
{
*p_tFOC = *p_bFOC = 0;
if( p_sps->i_pic_order_cnt_type == 0 )
{
unsigned maxPocLSB = 1U << (p_sps->i_log2_max_pic_order_cnt_lsb + 4);
if( p_slice->i_nal_type == H264_NAL_SLICE_IDR )
{
p_ctx->prevPicOrderCnt.lsb = 0;
p_ctx->prevPicOrderCnt.msb = 0;
}
else if( p_ctx->prevRefPictureHasMMCO5 )
{
p_ctx->prevPicOrderCnt.msb = 0;
if( !p_ctx->prevRefPictureIsBottomField )
p_ctx->prevPicOrderCnt.lsb = p_ctx->prevRefPictureTFOC;
else
p_ctx->prevPicOrderCnt.lsb = 0;
}
int pocMSB = p_ctx->prevPicOrderCnt.msb;
int64_t orderDiff = p_slice->i_pic_order_cnt_lsb - p_ctx->prevPicOrderCnt.lsb;
if( orderDiff < 0 && -orderDiff >= maxPocLSB / 2 )
pocMSB += maxPocLSB;
else if( orderDiff > maxPocLSB / 2 )
pocMSB -= maxPocLSB;
*p_tFOC = *p_bFOC = pocMSB + p_slice->i_pic_order_cnt_lsb;
if( p_slice->i_field_pic_flag )
*p_bFOC += p_slice->i_delta_pic_order_cnt_bottom;
if( p_slice->i_nal_ref_idc )
{
p_ctx->prevRefPictureIsBottomField = (p_slice->i_field_pic_flag &&
p_slice->i_bottom_field_flag);
p_ctx->prevRefPictureHasMMCO5 = p_slice->has_mmco5;
p_ctx->prevRefPictureTFOC = *p_tFOC;
p_ctx->prevPicOrderCnt.lsb = p_slice->i_pic_order_cnt_lsb;
p_ctx->prevPicOrderCnt.msb = pocMSB;
}
}
else
{
unsigned maxFrameNum = 1 << (p_sps->i_log2_max_frame_num + 4);
unsigned frameNumOffset;
unsigned expectedPicOrderCnt = 0;
if( p_slice->i_nal_type == H264_NAL_SLICE_IDR )
frameNumOffset = 0;
else if( p_ctx->prevFrameNum > p_slice->i_frame_num )
frameNumOffset = p_ctx->prevFrameNumOffset + maxFrameNum;
else
frameNumOffset = p_ctx->prevFrameNumOffset;
if( p_sps->i_pic_order_cnt_type == 1 )
{
unsigned absFrameNum;
if( p_sps->i_num_ref_frames_in_pic_order_cnt_cycle > 0 )
absFrameNum = frameNumOffset + p_slice->i_frame_num;
else
absFrameNum = 0;
if( p_slice->i_nal_ref_idc == 0 && absFrameNum > 0 )
absFrameNum--;
if( absFrameNum > 0 )
{
int32_t expectedDeltaPerPicOrderCntCycle = 0;
for( int i=0; i<p_sps->i_num_ref_frames_in_pic_order_cnt_cycle; i++ )
expectedDeltaPerPicOrderCntCycle += p_sps->offset_for_ref_frame[i];
unsigned picOrderCntCycleCnt = 0;
unsigned frameNumInPicOrderCntCycle = 0;
if( p_sps->i_num_ref_frames_in_pic_order_cnt_cycle )
{
picOrderCntCycleCnt = ( absFrameNum - 1 ) / p_sps->i_num_ref_frames_in_pic_order_cnt_cycle;
frameNumInPicOrderCntCycle = ( absFrameNum - 1 ) % p_sps->i_num_ref_frames_in_pic_order_cnt_cycle;
}
expectedPicOrderCnt = picOrderCntCycleCnt * expectedDeltaPerPicOrderCntCycle;
for( unsigned i=0; i <= frameNumInPicOrderCntCycle; i++ )
expectedPicOrderCnt = expectedPicOrderCnt + p_sps->offset_for_ref_frame[i];
}
if( p_slice->i_nal_ref_idc == 0 )
expectedPicOrderCnt = expectedPicOrderCnt + p_sps->offset_for_non_ref_pic;
*p_tFOC = expectedPicOrderCnt + p_slice->i_delta_pic_order_cnt0;
if( !p_slice->i_field_pic_flag )
*p_bFOC = *p_tFOC + p_sps->offset_for_top_to_bottom_field + p_slice->i_delta_pic_order_cnt1;
else if( p_slice->i_bottom_field_flag )
*p_bFOC = expectedPicOrderCnt + p_sps->offset_for_top_to_bottom_field + p_slice->i_delta_pic_order_cnt0;
}
else if( p_sps->i_pic_order_cnt_type == 2 )
{
unsigned tempPicOrderCnt;
if( p_slice->i_nal_type == H264_NAL_SLICE_IDR )
tempPicOrderCnt = 0;
else if( p_slice->i_nal_ref_idc == 0 )
tempPicOrderCnt = 2 * ( frameNumOffset + p_slice->i_frame_num ) - 1;
else
tempPicOrderCnt = 2 * ( frameNumOffset + p_slice->i_frame_num );
*p_bFOC = *p_tFOC = tempPicOrderCnt;
}
p_ctx->prevFrameNum = p_slice->i_frame_num;
if( p_slice->has_mmco5 )
p_ctx->prevFrameNumOffset = 0;
else
p_ctx->prevFrameNumOffset = frameNumOffset;
}
if( !p_slice->i_field_pic_flag ) 
*p_PictureOrderCount = __MIN( *p_bFOC, *p_tFOC );
else 
if ( p_slice->i_bottom_field_flag )
*p_PictureOrderCount = *p_bFOC;
else
*p_PictureOrderCount = *p_tFOC;
}
static uint8_t h264_infer_pic_struct( const h264_sequence_parameter_set_t *p_sps,
const h264_slice_t *p_slice,
uint8_t i_pic_struct, int tFOC, int bFOC )
{
if( !p_sps->vui.b_pic_struct_present_flag || i_pic_struct >= 9 )
{
if( p_slice->i_field_pic_flag )
i_pic_struct = 1 + p_slice->i_bottom_field_flag;
else if( tFOC == bFOC )
i_pic_struct = 0;
else if( tFOC < bFOC )
i_pic_struct = 3;
else
i_pic_struct = 4;
}
return i_pic_struct;
}
uint8_t h264_get_num_ts( const h264_sequence_parameter_set_t *p_sps,
const h264_slice_t *p_slice, uint8_t i_pic_struct,
int tFOC, int bFOC )
{
i_pic_struct = h264_infer_pic_struct( p_sps, p_slice, i_pic_struct, tFOC, bFOC );
const uint8_t rgi_numclock[9] = { 2, 1, 1, 2, 2, 3, 3, 4, 6 };
return rgi_numclock[ i_pic_struct ];
}
