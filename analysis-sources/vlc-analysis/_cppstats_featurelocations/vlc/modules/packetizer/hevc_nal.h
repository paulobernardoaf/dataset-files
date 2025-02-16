


















#if !defined(HEVC_NAL_H)
#define HEVC_NAL_H

#include <vlc_es.h>
#include <vlc_bits.h>

#define HEVC_VPS_ID_MAX 15
#define HEVC_SPS_ID_MAX 15
#define HEVC_PPS_ID_MAX 63

enum hevc_general_profile_idc_e
{
HEVC_PROFILE_IDC_NONE = 0,
HEVC_PROFILE_IDC_MAIN = 1,
HEVC_PROFILE_IDC_MAIN_10 = 2,
HEVC_PROFILE_IDC_MAIN_STILL_PICTURE = 3,
HEVC_PROFILE_IDC_REXT = 4, 
HEVC_PROFILE_IDC_HIGH_THROUGHPUT = 5,
HEVC_PROFILE_IDC_MULTIVIEW_MAIN = 6,
HEVC_PROFILE_IDC_SCALABLE_MAIN = 7,
HEVC_PROFILE_IDC_3D_MAIN = 8,
HEVC_PROFILE_IDC_SCREEN_EXTENDED = 9,
HEVC_PROFILE_IDC_SCALABLE_REXT = 10,
};

enum hevc_level_idc_e
{
HEVC_LEVEL_IDC_1 = 30, 
HEVC_LEVEL_IDC_2 = 60,
HEVC_LEVEL_IDC_2_1 = 63,
HEVC_LEVEL_IDC_3 = 90,
HEVC_LEVEL_IDC_3_1 = 93,
HEVC_LEVEL_IDC_4 = 120,
HEVC_LEVEL_IDC_4_1 = 123,
HEVC_LEVEL_IDC_5 = 150,
HEVC_LEVEL_IDC_5_1 = 153,
HEVC_LEVEL_IDC_5_2 = 156,
HEVC_LEVEL_IDC_6 = 180,
HEVC_LEVEL_IDC_6_1 = 183,
HEVC_LEVEL_IDC_6_2 = 186,
HEVC_LEVEL_IDC_8_5 = 255,
};


enum hevc_rext_indication_e
{
HEVC_REXT_INDICATION_MONOCHROME = 0x1F9,
HEVC_REXT_INDICATION_MONOCHROME_10 = 0x1B9,
HEVC_REXT_INDICATION_MONOCHROME_12 = 0x139,
HEVC_REXT_INDICATION_MONOCHROME_16 = 0x039,
HEVC_REXT_INDICATION_MAIN_12 = 0x131,
HEVC_REXT_INDICATION_MAIN_422_10 = 0x1A1,
HEVC_REXT_INDICATION_MAIN_422_12 = 0x121,
HEVC_REXT_INDICATION_MAIN_444 = 0x1C1,
HEVC_REXT_INDICATION_MAIN_444_10 = 0x181,
HEVC_REXT_INDICATION_MAIN_444_12 = 0x101,
HEVC_REXT_INDICATION_MAIN_INTRA = 0x1F4, 
HEVC_REXT_INDICATION_MAIN_10_INTRA = 0x1B4,
HEVC_REXT_INDICATION_MAIN_12_INTRA = 0x134,
HEVC_REXT_INDICATION_MAIN_422_10_INTRA = 0x1A4,
HEVC_REXT_INDICATION_MAIN_422_12_INTRA = 0x124,
HEVC_REXT_INDICATION_MAIN_444_INTRA = 0x1C4,
HEVC_REXT_INDICATION_MAIN_444_10_INTRA = 0x184,
HEVC_REXT_INDICATION_MAIN_444_12_INTRA = 0x104,
HEVC_REXT_INDICATION_MAIN_444_16_INTRA = 0x004,
HEVC_REXT_INDICATION_MAIN_444_STILL_PICTURE = 0x1C6,
HEVC_REXT_INDICATION_MAIN_444_16_STILL_PICTURE = 0x006,
};

enum hevc_high_throughput_indication_e
{
HEVC_HIGH_THROUGHPUT_INDICATION_444 = 0x3C1,
HEVC_HIGH_THROUGHPUT_INDICATION_444_10 = 0x381,
HEVC_HIGH_THROUGHPUT_INDICATION_444_14 = 0x201,
HEVC_HIGH_THROUGHPUT_INDICATION_444_16_INTRA = 0x002, 
};

enum hevc_screen_extended_indication_e
{
HEVC_SCREEN_EXTENDED_INDICATION_MAIN = 0x3F1,
HEVC_SCREEN_EXTENDED_INDICATION_MAIN_10 = 0x3B1,
HEVC_SCREEN_EXTENDED_INDICATION_MAIN_444 = 0x3C1,
HEVC_SCREEN_EXTENDED_INDICATION_MAIN_444_10 = 0x381,

HEVC_SCREEN_EXTENDED_INDICATION_HIGH_THROUGHPUT_444_14 = 0x201,
};



#define HEVC_INDICATION_SHIFT 8
#define MKPROF(a, b) ((HEVC_##a##_INDICATION_##b << HEVC_INDICATION_SHIFT) | HEVC_PROFILE_IDC_##a)
enum vlc_hevc_profile_e
{
VLC_HEVC_PROFILE_NONE = HEVC_PROFILE_IDC_NONE,
VLC_HEVC_PROFILE_MAIN = HEVC_PROFILE_IDC_MAIN,
VLC_HEVC_PROFILE_MAIN_10 = HEVC_PROFILE_IDC_MAIN_10,
VLC_HEVC_PROFILE_MAIN_STILL_PICTURE = HEVC_PROFILE_IDC_MAIN_STILL_PICTURE,

VLC_HEVC_PROFILE_MONOCHROME = MKPROF(REXT,MONOCHROME),
VLC_HEVC_PROFILE_MONOCHROME_10 = MKPROF(REXT,MONOCHROME_10),
VLC_HEVC_PROFILE_MONOCHROME_12 = MKPROF(REXT,MONOCHROME_12),
VLC_HEVC_PROFILE_MONOCHROME_16 = MKPROF(REXT,MONOCHROME_16),
VLC_HEVC_PROFILE_MAIN_12 = MKPROF(REXT,MAIN_12),
VLC_HEVC_PROFILE_MAIN_422_10 = MKPROF(REXT,MAIN_422_10),
VLC_HEVC_PROFILE_MAIN_422_12 = MKPROF(REXT,MAIN_422_12),
VLC_HEVC_PROFILE_MAIN_444 = MKPROF(REXT,MAIN_444),
VLC_HEVC_PROFILE_MAIN_444_10 = MKPROF(REXT,MAIN_444_10),
VLC_HEVC_PROFILE_MAIN_444_12 = MKPROF(REXT,MAIN_444_12),
VLC_HEVC_PROFILE_MAIN_INTRA = MKPROF(REXT,MAIN_INTRA),
VLC_HEVC_PROFILE_MAIN_10_INTRA = MKPROF(REXT,MAIN_10_INTRA),
VLC_HEVC_PROFILE_MAIN_12_INTRA = MKPROF(REXT,MAIN_12_INTRA),
VLC_HEVC_PROFILE_MAIN_422_10_INTRA = MKPROF(REXT,MAIN_422_10_INTRA),
VLC_HEVC_PROFILE_MAIN_422_12_INTRA = MKPROF(REXT,MAIN_422_12_INTRA),
VLC_HEVC_PROFILE_MAIN_444_INTRA = MKPROF(REXT,MAIN_444_INTRA),
VLC_HEVC_PROFILE_MAIN_444_10_INTRA = MKPROF(REXT,MAIN_444_10_INTRA),
VLC_HEVC_PROFILE_MAIN_444_12_INTRA = MKPROF(REXT,MAIN_444_12_INTRA),
VLC_HEVC_PROFILE_MAIN_444_16_INTRA = MKPROF(REXT,MAIN_444_16_INTRA),
VLC_HEVC_PROFILE_MAIN_444_STILL_PICTURE = MKPROF(REXT,MAIN_444_STILL_PICTURE),
VLC_HEVC_PROFILE_MAIN_444_16_STILL_PICTURE = MKPROF(REXT,MAIN_444_16_STILL_PICTURE),

VLC_HEVC_PROFILE_HIGH_THROUGHPUT_444 = MKPROF(HIGH_THROUGHPUT,444),
VLC_HEVC_PROFILE_HIGH_THROUGHPUT_444_10 = MKPROF(HIGH_THROUGHPUT,444_10),
VLC_HEVC_PROFILE_HIGH_THROUGHPUT_444_14 = MKPROF(HIGH_THROUGHPUT,444_14),
VLC_HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA = MKPROF(HIGH_THROUGHPUT,444_16_INTRA),

VLC_HEVC_PROFILE_MULTIVIEW_MAIN = HEVC_PROFILE_IDC_MULTIVIEW_MAIN,
VLC_HEVC_PROFILE_SCALABLE_MAIN = HEVC_PROFILE_IDC_SCALABLE_MAIN,
VLC_HEVC_PROFILE_3D_MAIN = HEVC_PROFILE_IDC_3D_MAIN,

VLC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN = MKPROF(SCREEN_EXTENDED,MAIN),
VLC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN_10 = MKPROF(SCREEN_EXTENDED,MAIN_10),
VLC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444 = MKPROF(SCREEN_EXTENDED,MAIN_444),
VLC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444_10 = MKPROF(SCREEN_EXTENDED,MAIN_444_10),
VLC_HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444_14 = MKPROF(SCREEN_EXTENDED,HIGH_THROUGHPUT_444_14),

VLC_HEVC_PROFILE_SCALABLE_REXT = 10,
};
#undef MKPROF

#define vlc_hevc_profile_is_Intra(p) (p && (1 << (2+HEVC_INDICATION_SHIFT)))


enum hevc_nal_unit_type_e
{
HEVC_NAL_TRAIL_N = 0, 
HEVC_NAL_TRAIL_R = 1, 
HEVC_NAL_TSA_N = 2, 
HEVC_NAL_TSA_R = 3, 
HEVC_NAL_STSA_N = 4, 
HEVC_NAL_STSA_R = 5, 
HEVC_NAL_RADL_N = 6, 
HEVC_NAL_RADL_R = 7, 
HEVC_NAL_RASL_N = 8, 
HEVC_NAL_RASL_R = 9, 

HEVC_NAL_RSV_VCL_N10= 10,
HEVC_NAL_RSV_VCL_N12= 12,
HEVC_NAL_RSV_VCL_N14= 14,

HEVC_NAL_BLA_W_LP = 16, 
HEVC_NAL_BLA_W_RADL = 17, 
HEVC_NAL_BLA_N_LP = 18, 
HEVC_NAL_IDR_W_RADL = 19, 
HEVC_NAL_IDR_N_LP = 20, 
HEVC_NAL_CRA = 21, 

HEVC_NAL_IRAP_VCL22 = 22, 
HEVC_NAL_IRAP_VCL23 = 23,

HEVC_NAL_VPS = 32,
HEVC_NAL_SPS = 33,
HEVC_NAL_PPS = 34,
HEVC_NAL_AUD = 35, 
HEVC_NAL_EOS = 36, 
HEVC_NAL_EOB = 37, 
HEVC_NAL_FD = 38, 
HEVC_NAL_PREF_SEI = 39, 
HEVC_NAL_SUFF_SEI = 40, 

HEVC_NAL_RSV_NVCL41 = 41, 
HEVC_NAL_RSV_NVCL44 = 44,
HEVC_NAL_RSV_NVCL45 = 45,
HEVC_NAL_RSV_NVCL47 = 47,
HEVC_NAL_UNSPEC48 = 48, 
HEVC_NAL_UNSPEC55 = 55,
HEVC_NAL_UNSPEC56 = 56,
HEVC_NAL_UNSPEC63 = 63,
HEVC_NAL_UNKNOWN
};

enum hevc_slice_type_e
{
HEVC_SLICE_TYPE_B = 0,
HEVC_SLICE_TYPE_P,
HEVC_SLICE_TYPE_I
};

#define HEVC_MIN_HVCC_SIZE 23


static inline bool hevc_ishvcC( const uint8_t *p_buf, size_t i_buf )
{
return ( i_buf >= HEVC_MIN_HVCC_SIZE &&
p_buf[0] != 0x00







);
}

static inline uint8_t hevc_getNALLengthSize( const uint8_t *p_hvcC )
{
return (p_hvcC[21] & 0x03) + 1;
}

static inline uint8_t hevc_getNALType( const uint8_t *p_buf )
{
return ((p_buf[0] & 0x7E) >> 1);
}

static inline uint8_t hevc_getNALLayer( const uint8_t *p_buf )
{
return ((p_buf[0] & 0x01) << 6) | (p_buf[1] >> 3);
}


typedef struct hevc_video_parameter_set_t hevc_video_parameter_set_t;
typedef struct hevc_sequence_parameter_set_t hevc_sequence_parameter_set_t;
typedef struct hevc_picture_parameter_set_t hevc_picture_parameter_set_t;
typedef struct hevc_slice_segment_header_t hevc_slice_segment_header_t;


hevc_video_parameter_set_t * hevc_decode_vps( const uint8_t *, size_t, bool );
hevc_sequence_parameter_set_t * hevc_decode_sps( const uint8_t *, size_t, bool );
hevc_picture_parameter_set_t * hevc_decode_pps( const uint8_t *, size_t, bool );

typedef void(*pf_get_matchedxps)(uint8_t i_pps_id, void *priv,
hevc_picture_parameter_set_t **,
hevc_sequence_parameter_set_t **,
hevc_video_parameter_set_t **);
hevc_slice_segment_header_t * hevc_decode_slice_header( const uint8_t *, size_t, bool,
pf_get_matchedxps, void *priv );

void hevc_rbsp_release_vps( hevc_video_parameter_set_t * );
void hevc_rbsp_release_sps( hevc_sequence_parameter_set_t * );
void hevc_rbsp_release_pps( hevc_picture_parameter_set_t * );
void hevc_rbsp_release_slice_header( hevc_slice_segment_header_t * );


uint8_t hevc_get_sps_vps_id( const hevc_sequence_parameter_set_t * );
uint8_t hevc_get_pps_sps_id( const hevc_picture_parameter_set_t * );
uint8_t hevc_get_slice_pps_id( const hevc_slice_segment_header_t * );

bool hevc_get_xps_id(const uint8_t *p_nalbuf, size_t i_nalbuf, uint8_t *pi_id);
bool hevc_get_sps_profile_tier_level( const hevc_sequence_parameter_set_t *,
uint8_t *pi_profile, uint8_t *pi_level );
enum vlc_hevc_profile_e
hevc_get_vlc_profile( const hevc_sequence_parameter_set_t * );
bool hevc_get_picture_size( const hevc_sequence_parameter_set_t *, unsigned *p_w, unsigned *p_h,
unsigned *p_vw, unsigned *p_vh );
bool hevc_get_frame_rate( const hevc_sequence_parameter_set_t *,
const hevc_video_parameter_set_t * ,
unsigned *pi_num, unsigned *pi_den );
bool hevc_get_aspect_ratio( const hevc_sequence_parameter_set_t *,
unsigned *pi_num, unsigned *pi_den );
bool hevc_get_chroma_luma( const hevc_sequence_parameter_set_t *, uint8_t *pi_chroma_format,
uint8_t *pi_depth_luma, uint8_t *pi_depth_chroma );
bool hevc_get_colorimetry( const hevc_sequence_parameter_set_t *p_sps,
video_color_primaries_t *p_primaries,
video_transfer_func_t *p_transfer,
video_color_space_t *p_colorspace,
video_color_range_t *p_full_range );
uint8_t hevc_get_max_num_reorder( const hevc_video_parameter_set_t *p_vps );
bool hevc_get_slice_type( const hevc_slice_segment_header_t *, enum hevc_slice_type_e * );


bool hevc_get_profile_level(const es_format_t *p_fmt, uint8_t *pi_profile,
uint8_t *pi_level, uint8_t *pi_nal_length_size);




struct hevc_dcr_values
{
uint8_t general_configuration[12];
uint8_t i_numTemporalLayer;
uint8_t i_chroma_idc;
uint8_t i_bit_depth_luma_minus8;
uint8_t i_bit_depth_chroma_minus8;
bool b_temporalIdNested;
};

#define HEVC_DCR_VPS_COUNT (HEVC_VPS_ID_MAX + 1)
#define HEVC_DCR_SPS_COUNT (HEVC_SPS_ID_MAX + 1)
#define HEVC_DCR_PPS_COUNT (HEVC_PPS_ID_MAX + 1)
#define HEVC_DCR_SEI_COUNT (16)

struct hevc_dcr_params
{
const uint8_t *p_vps[HEVC_DCR_VPS_COUNT],
*p_sps[HEVC_DCR_SPS_COUNT],
*p_pps[HEVC_DCR_VPS_COUNT],
*p_seipref[HEVC_DCR_SEI_COUNT],
*p_seisuff[HEVC_DCR_SEI_COUNT];
uint8_t rgi_vps[HEVC_DCR_VPS_COUNT],
rgi_sps[HEVC_DCR_SPS_COUNT],
rgi_pps[HEVC_DCR_PPS_COUNT],
rgi_seipref[HEVC_DCR_SEI_COUNT],
rgi_seisuff[HEVC_DCR_SEI_COUNT];
uint8_t i_vps_count, i_sps_count, i_pps_count;
uint8_t i_seipref_count, i_seisuff_count;
struct hevc_dcr_values *p_values;
};

uint8_t * hevc_create_dcr( const struct hevc_dcr_params *p_params,
uint8_t i_nal_length_size,
bool b_completeness, size_t *pi_size );


uint8_t * hevc_hvcC_to_AnnexB_NAL( const uint8_t *p_buf, size_t i_buf,
size_t *pi_res, uint8_t *pi_nal_length_size );




typedef struct
{
struct
{
int lsb;
int msb;
} prevTid0PicOrderCnt;

bool HandleCraAsBlaFlag;
bool first_picture; 
} hevc_poc_ctx_t;

static inline void hevc_poc_cxt_init( hevc_poc_ctx_t *p_ctx )
{
p_ctx->prevTid0PicOrderCnt.lsb = 0;
p_ctx->prevTid0PicOrderCnt.msb = 0;
p_ctx->first_picture = true;
}

int hevc_compute_picture_order_count( const hevc_sequence_parameter_set_t *p_sps,
const hevc_slice_segment_header_t *slice,
hevc_poc_ctx_t *ctx );

typedef struct hevc_sei_pic_timing_t hevc_sei_pic_timing_t;

hevc_sei_pic_timing_t * hevc_decode_sei_pic_timing( bs_t *,
const hevc_sequence_parameter_set_t * );
void hevc_release_sei_pic_timing( hevc_sei_pic_timing_t * );

uint8_t hevc_get_num_clock_ts( const hevc_sequence_parameter_set_t *,
const hevc_sei_pic_timing_t * );
bool hevc_frame_is_progressive( const hevc_sequence_parameter_set_t *,
const hevc_sei_pic_timing_t * );

#endif 
