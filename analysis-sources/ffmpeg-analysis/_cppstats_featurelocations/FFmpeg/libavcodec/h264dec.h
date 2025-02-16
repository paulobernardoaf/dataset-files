


























#if !defined(AVCODEC_H264DEC_H)
#define AVCODEC_H264DEC_H

#include "libavutil/buffer.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/thread.h"

#include "cabac.h"
#include "error_resilience.h"
#include "h264_parse.h"
#include "h264_ps.h"
#include "h264_sei.h"
#include "h2645_parse.h"
#include "h264chroma.h"
#include "h264dsp.h"
#include "h264pred.h"
#include "h264qpel.h"
#include "internal.h"
#include "mpegutils.h"
#include "parser.h"
#include "qpeldsp.h"
#include "rectangle.h"
#include "videodsp.h"

#define H264_MAX_PICTURE_COUNT 36

#define MAX_MMCO_COUNT 66

#define MAX_DELAYED_PIC_COUNT 16



#define ALLOW_INTERLACE

#define FMO 0





#define MAX_SLICES 32

#if defined(ALLOW_INTERLACE)
#define MB_MBAFF(h) (h)->mb_mbaff
#define MB_FIELD(sl) (sl)->mb_field_decoding_flag
#define FRAME_MBAFF(h) (h)->mb_aff_frame
#define FIELD_PICTURE(h) ((h)->picture_structure != PICT_FRAME)
#define LEFT_MBS 2
#define LTOP 0
#define LBOT 1
#define LEFT(i) (i)
#else
#define MB_MBAFF(h) 0
#define MB_FIELD(sl) 0
#define FRAME_MBAFF(h) 0
#define FIELD_PICTURE(h) 0
#undef IS_INTERLACED
#define IS_INTERLACED(mb_type) 0
#define LEFT_MBS 1
#define LTOP 0
#define LBOT 0
#define LEFT(i) 0
#endif
#define FIELD_OR_MBAFF_PICTURE(h) (FRAME_MBAFF(h) || FIELD_PICTURE(h))

#if !defined(CABAC)
#define CABAC(h) (h)->ps.pps->cabac
#endif

#define CHROMA(h) ((h)->ps.sps->chroma_format_idc)
#define CHROMA422(h) ((h)->ps.sps->chroma_format_idc == 2)
#define CHROMA444(h) ((h)->ps.sps->chroma_format_idc == 3)

#define MB_TYPE_REF0 MB_TYPE_ACPRED 
#define MB_TYPE_8x8DCT 0x01000000
#define IS_REF0(a) ((a) & MB_TYPE_REF0)
#define IS_8x8DCT(a) ((a) & MB_TYPE_8x8DCT)




typedef enum MMCOOpcode {
MMCO_END = 0,
MMCO_SHORT2UNUSED,
MMCO_LONG2UNUSED,
MMCO_SHORT2LONG,
MMCO_SET_MAX_LONG,
MMCO_RESET,
MMCO_LONG,
} MMCOOpcode;




typedef struct MMCO {
MMCOOpcode opcode;
int short_pic_num; 
int long_arg; 
} MMCO;

typedef struct H264Picture {
AVFrame *f;
ThreadFrame tf;

AVBufferRef *qscale_table_buf;
int8_t *qscale_table;

AVBufferRef *motion_val_buf[2];
int16_t (*motion_val[2])[2];

AVBufferRef *mb_type_buf;
uint32_t *mb_type;

AVBufferRef *hwaccel_priv_buf;
void *hwaccel_picture_private; 

AVBufferRef *ref_index_buf[2];
int8_t *ref_index[2];

int field_poc[2]; 
int poc; 
int frame_num; 
int mmco_reset; 

int pic_id; 

int long_ref; 
int ref_poc[2][2][32]; 
int ref_count[2][2]; 
int mbaff; 
int field_picture; 

int reference;
int recovered; 
int invalid_gap;
int sei_recovery_frame_cnt;
} H264Picture;

typedef struct H264Ref {
uint8_t *data[3];
int linesize[3];

int reference;
int poc;
int pic_id;

H264Picture *parent;
} H264Ref;

typedef struct H264SliceContext {
struct H264Context *h264;
GetBitContext gb;
ERContext er;

int slice_num;
int slice_type;
int slice_type_nos; 
int slice_type_fixed;

int qscale;
int chroma_qp[2]; 
int qp_thresh; 
int last_qscale_diff;


int deblocking_filter; 
int slice_alpha_c0_offset;
int slice_beta_offset;

H264PredWeightTable pwt;

int prev_mb_skipped;
int next_mb_skipped;

int chroma_pred_mode;
int intra16x16_pred_mode;

int8_t intra4x4_pred_mode_cache[5 * 8];
int8_t(*intra4x4_pred_mode);

int topleft_mb_xy;
int top_mb_xy;
int topright_mb_xy;
int left_mb_xy[LEFT_MBS];

int topleft_type;
int top_type;
int topright_type;
int left_type[LEFT_MBS];

const uint8_t *left_block;
int topleft_partition;

unsigned int topleft_samples_available;
unsigned int top_samples_available;
unsigned int topright_samples_available;
unsigned int left_samples_available;

ptrdiff_t linesize, uvlinesize;
ptrdiff_t mb_linesize; 
ptrdiff_t mb_uvlinesize;

int mb_x, mb_y;
int mb_xy;
int resync_mb_x;
int resync_mb_y;
unsigned int first_mb_addr;

int next_slice_idx;
int mb_skip_run;
int is_complex;

int picture_structure;
int mb_field_decoding_flag;
int mb_mbaff; 

int redundant_pic_count;




int neighbor_transform_size;

int direct_spatial_mv_pred;
int col_parity;
int col_fieldoff;

int cbp;
int top_cbp;
int left_cbp;

int dist_scale_factor[32];
int dist_scale_factor_field[2][32];
int map_col_to_list0[2][16 + 32];
int map_col_to_list0_field[2][2][16 + 32];




unsigned int ref_count[2]; 
unsigned int list_count;
H264Ref ref_list[2][48]; 


struct {
uint8_t op;
uint32_t val;
} ref_modifications[2][32];
int nb_ref_modifications[2];

unsigned int pps_id;

const uint8_t *intra_pcm_ptr;
int16_t *dc_val_base;

uint8_t *bipred_scratchpad;
uint8_t *edge_emu_buffer;
uint8_t (*top_borders[2])[(16 * 3) * 2];
int bipred_scratchpad_allocated;
int edge_emu_buffer_allocated;
int top_borders_allocated[2];





DECLARE_ALIGNED(8, uint8_t, non_zero_count_cache)[15 * 8];




DECLARE_ALIGNED(16, int16_t, mv_cache)[2][5 * 8][2];
DECLARE_ALIGNED(8, int8_t, ref_cache)[2][5 * 8];
DECLARE_ALIGNED(16, uint8_t, mvd_cache)[2][5 * 8][2];
uint8_t direct_cache[5 * 8];

DECLARE_ALIGNED(8, uint16_t, sub_mb_type)[4];


DECLARE_ALIGNED(16, int16_t, mb)[16 * 48 * 2];
DECLARE_ALIGNED(16, int16_t, mb_luma_dc)[3][16 * 2];


int16_t mb_padding[256 * 2];

uint8_t (*mvd_table[2])[2];




CABACContext cabac;
uint8_t cabac_state[1024];
int cabac_init_idc;

MMCO mmco[MAX_MMCO_COUNT];
int nb_mmco;
int explicit_ref_marking;

int frame_num;
int poc_lsb;
int delta_poc_bottom;
int delta_poc[2];
int curr_pic_num;
int max_pic_num;
} H264SliceContext;




typedef struct H264Context {
const AVClass *class;
AVCodecContext *avctx;
VideoDSPContext vdsp;
H264DSPContext h264dsp;
H264ChromaContext h264chroma;
H264QpelContext h264qpel;

H264Picture DPB[H264_MAX_PICTURE_COUNT];
H264Picture *cur_pic_ptr;
H264Picture cur_pic;
H264Picture last_pic_for_ec;

H264SliceContext *slice_ctx;
int nb_slice_ctx;
int nb_slice_ctx_queued;

H2645Packet pkt;

int pixel_shift; 


int width, height;
int chroma_x_shift, chroma_y_shift;

int droppable;
int coded_picture_number;

int context_initialized;
int flags;
int workaround_bugs;
int x264_build;




int postpone_filter;




int picture_idr;

int crop_left;
int crop_right;
int crop_top;
int crop_bottom;

int8_t(*intra4x4_pred_mode);
H264PredContext hpc;

uint8_t (*non_zero_count)[48];

#define LIST_NOT_USED -1 
#define PART_NOT_AVAILABLE -2





int block_offset[2 * (16 * 3)];

uint32_t *mb2b_xy; 
uint32_t *mb2br_xy;
int b_stride; 

uint16_t *slice_table; 


int mb_aff_frame;
int picture_structure;
int first_field;

uint8_t *list_counts; 


uint16_t *cbp_table;


uint8_t *chroma_pred_mode_table;
uint8_t (*mvd_table[2])[2];
uint8_t *direct_table;

uint8_t scan_padding[16];
uint8_t zigzag_scan[16];
uint8_t zigzag_scan8x8[64];
uint8_t zigzag_scan8x8_cavlc[64];
uint8_t field_scan[16];
uint8_t field_scan8x8[64];
uint8_t field_scan8x8_cavlc[64];
uint8_t zigzag_scan_q0[16];
uint8_t zigzag_scan8x8_q0[64];
uint8_t zigzag_scan8x8_cavlc_q0[64];
uint8_t field_scan_q0[16];
uint8_t field_scan8x8_q0[64];
uint8_t field_scan8x8_cavlc_q0[64];

int mb_y;
int mb_height, mb_width;
int mb_stride;
int mb_num;




int nal_ref_idc;
int nal_unit_type;

int has_slice; 




int is_avc; 
int nal_length_size; 

int bit_depth_luma; 
int chroma_format_idc; 

H264ParamSets ps;

uint16_t *slice_table_base;

H264POCContext poc;

H264Ref default_ref[2];
H264Picture *short_ref[32];
H264Picture *long_ref[32];
H264Picture *delayed_pic[MAX_DELAYED_PIC_COUNT + 2]; 
int last_pocs[MAX_DELAYED_PIC_COUNT];
H264Picture *next_output_pic;
int next_outputed_poc;




MMCO mmco[MAX_MMCO_COUNT];
int nb_mmco;
int mmco_reset;
int explicit_ref_marking;

int long_ref_count; 
int short_ref_count; 








int current_slice;









int prev_interlaced_frame;




int valid_recovery_point;







int recovery_frame;





#define FRAME_RECOVERED_IDR (1 << 0)




#define FRAME_RECOVERED_SEI (1 << 1)

int frame_recovered; 

int has_recovery_point;

int missing_fields;





int setup_finished;

int cur_chroma_format_idc;
int cur_bit_depth_luma;
int16_t slice_row[MAX_SLICES]; 



int width_from_caller;
int height_from_caller;

int enable_er;

H264SEIContext sei;

AVBufferPool *qscale_table_pool;
AVBufferPool *mb_type_pool;
AVBufferPool *motion_val_pool;
AVBufferPool *ref_index_pool;
int ref2frm[MAX_SLICES][2][64]; 
} H264Context;

extern const uint16_t ff_h264_mb_sizes[4];




int ff_h264_get_slice_type(const H264SliceContext *sl);





int ff_h264_alloc_tables(H264Context *h);

int ff_h264_decode_ref_pic_list_reordering(H264SliceContext *sl, void *logctx);
int ff_h264_build_ref_list(H264Context *h, H264SliceContext *sl);
void ff_h264_remove_all_refs(H264Context *h);




int ff_h264_execute_ref_pic_marking(H264Context *h);

int ff_h264_decode_ref_pic_marking(H264SliceContext *sl, GetBitContext *gb,
const H2645NAL *nal, void *logctx);

void ff_h264_hl_decode_mb(const H264Context *h, H264SliceContext *sl);
void ff_h264_decode_init_vlc(void);





int ff_h264_decode_mb_cavlc(const H264Context *h, H264SliceContext *sl);





int ff_h264_decode_mb_cabac(const H264Context *h, H264SliceContext *sl);

void ff_h264_init_cabac_states(const H264Context *h, H264SliceContext *sl);

void ff_h264_direct_dist_scale_factor(const H264Context *const h, H264SliceContext *sl);
void ff_h264_direct_ref_list_init(const H264Context *const h, H264SliceContext *sl);
void ff_h264_pred_direct_motion(const H264Context *const h, H264SliceContext *sl,
int *mb_type);

void ff_h264_filter_mb_fast(const H264Context *h, H264SliceContext *sl, int mb_x, int mb_y,
uint8_t *img_y, uint8_t *img_cb, uint8_t *img_cr,
unsigned int linesize, unsigned int uvlinesize);
void ff_h264_filter_mb(const H264Context *h, H264SliceContext *sl, int mb_x, int mb_y,
uint8_t *img_y, uint8_t *img_cb, uint8_t *img_cr,
unsigned int linesize, unsigned int uvlinesize);































#define LUMA_DC_BLOCK_INDEX 48
#define CHROMA_DC_BLOCK_INDEX 49


static const uint8_t scan8[16 * 3 + 3] = {
4 + 1 * 8, 5 + 1 * 8, 4 + 2 * 8, 5 + 2 * 8,
6 + 1 * 8, 7 + 1 * 8, 6 + 2 * 8, 7 + 2 * 8,
4 + 3 * 8, 5 + 3 * 8, 4 + 4 * 8, 5 + 4 * 8,
6 + 3 * 8, 7 + 3 * 8, 6 + 4 * 8, 7 + 4 * 8,
4 + 6 * 8, 5 + 6 * 8, 4 + 7 * 8, 5 + 7 * 8,
6 + 6 * 8, 7 + 6 * 8, 6 + 7 * 8, 7 + 7 * 8,
4 + 8 * 8, 5 + 8 * 8, 4 + 9 * 8, 5 + 9 * 8,
6 + 8 * 8, 7 + 8 * 8, 6 + 9 * 8, 7 + 9 * 8,
4 + 11 * 8, 5 + 11 * 8, 4 + 12 * 8, 5 + 12 * 8,
6 + 11 * 8, 7 + 11 * 8, 6 + 12 * 8, 7 + 12 * 8,
4 + 13 * 8, 5 + 13 * 8, 4 + 14 * 8, 5 + 14 * 8,
6 + 13 * 8, 7 + 13 * 8, 6 + 14 * 8, 7 + 14 * 8,
0 + 0 * 8, 0 + 5 * 8, 0 + 10 * 8
};

static av_always_inline uint32_t pack16to32(unsigned a, unsigned b)
{
#if HAVE_BIGENDIAN
return (b & 0xFFFF) + (a << 16);
#else
return (a & 0xFFFF) + (b << 16);
#endif
}

static av_always_inline uint16_t pack8to16(unsigned a, unsigned b)
{
#if HAVE_BIGENDIAN
return (b & 0xFF) + (a << 8);
#else
return (a & 0xFF) + (b << 8);
#endif
}




static av_always_inline int get_chroma_qp(const PPS *pps, int t, int qscale)
{
return pps->chroma_qp_table[t][qscale];
}




static av_always_inline int pred_intra_mode(const H264Context *h,
H264SliceContext *sl, int n)
{
const int index8 = scan8[n];
const int left = sl->intra4x4_pred_mode_cache[index8 - 1];
const int top = sl->intra4x4_pred_mode_cache[index8 - 8];
const int min = FFMIN(left, top);

ff_tlog(h->avctx, "mode:%d %d min:%d\n", left, top, min);

if (min < 0)
return DC_PRED;
else
return min;
}

static av_always_inline void write_back_intra_pred_mode(const H264Context *h,
H264SliceContext *sl)
{
int8_t *i4x4 = sl->intra4x4_pred_mode + h->mb2br_xy[sl->mb_xy];
int8_t *i4x4_cache = sl->intra4x4_pred_mode_cache;

AV_COPY32(i4x4, i4x4_cache + 4 + 8 * 4);
i4x4[4] = i4x4_cache[7 + 8 * 3];
i4x4[5] = i4x4_cache[7 + 8 * 2];
i4x4[6] = i4x4_cache[7 + 8 * 1];
}

static av_always_inline void write_back_non_zero_count(const H264Context *h,
H264SliceContext *sl)
{
const int mb_xy = sl->mb_xy;
uint8_t *nnz = h->non_zero_count[mb_xy];
uint8_t *nnz_cache = sl->non_zero_count_cache;

AV_COPY32(&nnz[ 0], &nnz_cache[4 + 8 * 1]);
AV_COPY32(&nnz[ 4], &nnz_cache[4 + 8 * 2]);
AV_COPY32(&nnz[ 8], &nnz_cache[4 + 8 * 3]);
AV_COPY32(&nnz[12], &nnz_cache[4 + 8 * 4]);
AV_COPY32(&nnz[16], &nnz_cache[4 + 8 * 6]);
AV_COPY32(&nnz[20], &nnz_cache[4 + 8 * 7]);
AV_COPY32(&nnz[32], &nnz_cache[4 + 8 * 11]);
AV_COPY32(&nnz[36], &nnz_cache[4 + 8 * 12]);

if (!h->chroma_y_shift) {
AV_COPY32(&nnz[24], &nnz_cache[4 + 8 * 8]);
AV_COPY32(&nnz[28], &nnz_cache[4 + 8 * 9]);
AV_COPY32(&nnz[40], &nnz_cache[4 + 8 * 13]);
AV_COPY32(&nnz[44], &nnz_cache[4 + 8 * 14]);
}
}

static av_always_inline void write_back_motion_list(const H264Context *h,
H264SliceContext *sl,
int b_stride,
int b_xy, int b8_xy,
int mb_type, int list)
{
int16_t(*mv_dst)[2] = &h->cur_pic.motion_val[list][b_xy];
int16_t(*mv_src)[2] = &sl->mv_cache[list][scan8[0]];
AV_COPY128(mv_dst + 0 * b_stride, mv_src + 8 * 0);
AV_COPY128(mv_dst + 1 * b_stride, mv_src + 8 * 1);
AV_COPY128(mv_dst + 2 * b_stride, mv_src + 8 * 2);
AV_COPY128(mv_dst + 3 * b_stride, mv_src + 8 * 3);
if (CABAC(h)) {
uint8_t (*mvd_dst)[2] = &sl->mvd_table[list][FMO ? 8 * sl->mb_xy
: h->mb2br_xy[sl->mb_xy]];
uint8_t(*mvd_src)[2] = &sl->mvd_cache[list][scan8[0]];
if (IS_SKIP(mb_type)) {
AV_ZERO128(mvd_dst);
} else {
AV_COPY64(mvd_dst, mvd_src + 8 * 3);
AV_COPY16(mvd_dst + 3 + 3, mvd_src + 3 + 8 * 0);
AV_COPY16(mvd_dst + 3 + 2, mvd_src + 3 + 8 * 1);
AV_COPY16(mvd_dst + 3 + 1, mvd_src + 3 + 8 * 2);
}
}

{
int8_t *ref_index = &h->cur_pic.ref_index[list][b8_xy];
int8_t *ref_cache = sl->ref_cache[list];
ref_index[0 + 0 * 2] = ref_cache[scan8[0]];
ref_index[1 + 0 * 2] = ref_cache[scan8[4]];
ref_index[0 + 1 * 2] = ref_cache[scan8[8]];
ref_index[1 + 1 * 2] = ref_cache[scan8[12]];
}
}

static av_always_inline void write_back_motion(const H264Context *h,
H264SliceContext *sl,
int mb_type)
{
const int b_stride = h->b_stride;
const int b_xy = 4 * sl->mb_x + 4 * sl->mb_y * h->b_stride; 
const int b8_xy = 4 * sl->mb_xy;

if (USES_LIST(mb_type, 0)) {
write_back_motion_list(h, sl, b_stride, b_xy, b8_xy, mb_type, 0);
} else {
fill_rectangle(&h->cur_pic.ref_index[0][b8_xy],
2, 2, 2, (uint8_t)LIST_NOT_USED, 1);
}
if (USES_LIST(mb_type, 1))
write_back_motion_list(h, sl, b_stride, b_xy, b8_xy, mb_type, 1);

if (sl->slice_type_nos == AV_PICTURE_TYPE_B && CABAC(h)) {
if (IS_8X8(mb_type)) {
uint8_t *direct_table = &h->direct_table[4 * sl->mb_xy];
direct_table[1] = sl->sub_mb_type[1] >> 1;
direct_table[2] = sl->sub_mb_type[2] >> 1;
direct_table[3] = sl->sub_mb_type[3] >> 1;
}
}
}

static av_always_inline int get_dct8x8_allowed(const H264Context *h, H264SliceContext *sl)
{
if (h->ps.sps->direct_8x8_inference_flag)
return !(AV_RN64A(sl->sub_mb_type) &
((MB_TYPE_16x8 | MB_TYPE_8x16 | MB_TYPE_8x8) *
0x0001000100010001ULL));
else
return !(AV_RN64A(sl->sub_mb_type) &
((MB_TYPE_16x8 | MB_TYPE_8x16 | MB_TYPE_8x8 | MB_TYPE_DIRECT2) *
0x0001000100010001ULL));
}

static inline int find_start_code(const uint8_t *buf, int buf_size,
int buf_index, int next_avc)
{
uint32_t state = -1;

buf_index = avpriv_find_start_code(buf + buf_index, buf + next_avc + 1, &state) - buf - 1;

return FFMIN(buf_index, buf_size);
}

int ff_h264_field_end(H264Context *h, H264SliceContext *sl, int in_setup);

int ff_h264_ref_picture(H264Context *h, H264Picture *dst, H264Picture *src);
void ff_h264_unref_picture(H264Context *h, H264Picture *pic);

int ff_h264_slice_context_init(H264Context *h, H264SliceContext *sl);

void ff_h264_draw_horiz_band(const H264Context *h, H264SliceContext *sl, int y, int height);







int ff_h264_queue_decode_slice(H264Context *h, const H2645NAL *nal);
int ff_h264_execute_decode_slices(H264Context *h);
int ff_h264_update_thread_context(AVCodecContext *dst,
const AVCodecContext *src);

void ff_h264_flush_change(H264Context *h);

void ff_h264_free_tables(H264Context *h);

void ff_h264_set_erpic(ERPicture *dst, H264Picture *src);

#endif 
