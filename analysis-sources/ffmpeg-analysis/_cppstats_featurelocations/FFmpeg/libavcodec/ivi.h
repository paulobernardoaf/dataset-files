



























#if !defined(AVCODEC_IVI_H)
#define AVCODEC_IVI_H

#include "avcodec.h"
#include "get_bits.h"
#include <stdint.h>




enum {
IVI4_FRAMETYPE_INTRA = 0,
IVI4_FRAMETYPE_INTRA1 = 1, 
IVI4_FRAMETYPE_INTER = 2, 
IVI4_FRAMETYPE_BIDIR = 3, 
IVI4_FRAMETYPE_INTER_NOREF = 4, 
IVI4_FRAMETYPE_NULL_FIRST = 5, 
IVI4_FRAMETYPE_NULL_LAST = 6 
};

#define IVI_VLC_BITS 13 
#define IVI5_IS_PROTECTED 0x20




typedef struct IVIHuffDesc {
int32_t num_rows;
uint8_t xbits[16];
} IVIHuffDesc;




typedef struct IVIHuffTab {
int32_t tab_sel; 

VLC *tab; 


IVIHuffDesc cust_desc; 
VLC cust_tab; 
} IVIHuffTab;

enum {
IVI_MB_HUFF = 0, 
IVI_BLK_HUFF = 1 
};





extern const uint8_t ff_ivi_vertical_scan_8x8[64];
extern const uint8_t ff_ivi_horizontal_scan_8x8[64];
extern const uint8_t ff_ivi_direct_scan_4x4[16];





typedef void (InvTransformPtr)(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);
typedef void (DCTransformPtr) (const int32_t *in, int16_t *out, ptrdiff_t pitch, int blk_size);





typedef struct RVMapDesc {
uint8_t eob_sym; 
uint8_t esc_sym; 
uint8_t runtab[256];
int8_t valtab[256];
} RVMapDesc;

extern const RVMapDesc ff_ivi_rvmap_tabs[9];





typedef struct IVIMbInfo {
int16_t xpos;
int16_t ypos;
uint32_t buf_offs; 
uint8_t type; 
uint8_t cbp; 
int8_t q_delta; 
int8_t mv_x; 
int8_t mv_y; 
int8_t b_mv_x; 
int8_t b_mv_y; 
} IVIMbInfo;





typedef struct IVITile {
int xpos;
int ypos;
int width;
int height;
int mb_size;
int is_empty; 
int data_size; 
int num_MBs; 
IVIMbInfo *mbs; 
IVIMbInfo *ref_mbs; 
} IVITile;





typedef struct IVIBandDesc {
int plane; 
int band_num; 
int width;
int height;
int aheight; 
const uint8_t *data_ptr; 
int data_size; 
int16_t *buf; 
int16_t *ref_buf; 
int16_t *b_ref_buf; 
int16_t *bufs[4]; 
ptrdiff_t pitch; 
int is_empty; 
int mb_size; 
int blk_size; 
int is_halfpel; 
int inherit_mv; 
int inherit_qdelta; 
int qdelta_present; 
int quant_mat; 
int glob_quant; 
const uint8_t *scan; 
int scan_size; 

IVIHuffTab blk_vlc; 

int num_corr; 
uint8_t corr[61*2]; 
int rvmap_sel; 
RVMapDesc *rv_map; 
int num_tiles; 
IVITile *tiles; 
InvTransformPtr *inv_transform;
int transform_size;
DCTransformPtr *dc_transform;
int is_2d_trans; 
int32_t checksum; 
int checksum_present;
int bufsize; 
const uint16_t *intra_base; 
const uint16_t *inter_base; 
const uint8_t *intra_scale; 
const uint8_t *inter_scale; 
} IVIBandDesc;





typedef struct IVIPlaneDesc {
uint16_t width;
uint16_t height;
uint8_t num_bands; 
IVIBandDesc *bands; 
} IVIPlaneDesc;


typedef struct IVIPicConfig {
uint16_t pic_width;
uint16_t pic_height;
uint16_t chroma_width;
uint16_t chroma_height;
uint16_t tile_width;
uint16_t tile_height;
uint8_t luma_bands;
uint8_t chroma_bands;
} IVIPicConfig;

typedef struct IVI45DecContext {
GetBitContext gb;
RVMapDesc rvmap_tabs[9]; 

uint32_t frame_num;
int frame_type;
int prev_frame_type; 
uint32_t data_size; 
int is_scalable;
const uint8_t *frame_data; 
int inter_scal; 
uint32_t frame_size; 
uint32_t pic_hdr_size; 
uint8_t frame_flags;
uint16_t checksum; 

IVIPicConfig pic_conf;
IVIPlaneDesc planes[3]; 

int buf_switch; 
int dst_buf; 
int ref_buf; 
int ref2_buf; 
int b_ref_buf; 

IVIHuffTab mb_vlc; 
IVIHuffTab blk_vlc; 

uint8_t rvmap_sel;
uint8_t in_imf;
uint8_t in_q; 
uint8_t pic_glob_quant;
uint8_t unknown1;

uint16_t gop_hdr_size;
uint8_t gop_flags;
uint32_t lock_word;

int show_indeo4_info;
uint8_t has_b_frames;
uint8_t has_transp; 
uint8_t uses_tiling;
uint8_t uses_haar;
uint8_t uses_fullpel;

int (*decode_pic_hdr) (struct IVI45DecContext *ctx, AVCodecContext *avctx);
int (*decode_band_hdr) (struct IVI45DecContext *ctx, IVIBandDesc *band, AVCodecContext *avctx);
int (*decode_mb_info) (struct IVI45DecContext *ctx, IVIBandDesc *band, IVITile *tile, AVCodecContext *avctx);
void (*switch_buffers) (struct IVI45DecContext *ctx);
int (*is_nonnull_frame)(struct IVI45DecContext *ctx);

int gop_invalid;
int buf_invalid[4];

int is_indeo4;

AVFrame *p_frame;
int got_p_frame;
} IVI45DecContext;


static inline int ivi_pic_config_cmp(IVIPicConfig *str1, IVIPicConfig *str2)
{
return str1->pic_width != str2->pic_width || str1->pic_height != str2->pic_height ||
str1->chroma_width != str2->chroma_width || str1->chroma_height != str2->chroma_height ||
str1->tile_width != str2->tile_width || str1->tile_height != str2->tile_height ||
str1->luma_bands != str2->luma_bands || str1->chroma_bands != str2->chroma_bands;
}


#define IVI_NUM_TILES(stride, tile_size) (((stride) + (tile_size) - 1) / (tile_size))


#define IVI_MBs_PER_TILE(tile_width, tile_height, mb_size) ((((tile_width) + (mb_size) - 1) / (mb_size)) * (((tile_height) + (mb_size) - 1) / (mb_size)))



#define IVI_TOSIGNED(val) (-(((val) >> 1) ^ -((val) & 1)))


static inline int ivi_scale_mv(int mv, int mv_scale)
{
return (mv + (mv > 0) + (mv_scale - 1)) >> mv_scale;
}




void ff_ivi_init_static_vlc(void);












int ff_ivi_dec_huff_desc(GetBitContext *gb, int desc_coded, int which_tab,
IVIHuffTab *huff_tab, AVCodecContext *avctx);









int ff_ivi_init_planes(AVCodecContext *avctx, IVIPlaneDesc *planes,
const IVIPicConfig *cfg, int is_indeo4);









int ff_ivi_init_tiles(IVIPlaneDesc *planes, int tile_width, int tile_height);

int ff_ivi_decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
AVPacket *avpkt);
int ff_ivi_decode_close(AVCodecContext *avctx);

#endif 
