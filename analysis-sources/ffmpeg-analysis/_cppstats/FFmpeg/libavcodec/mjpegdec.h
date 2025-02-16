#include "libavutil/log.h"
#include "libavutil/pixdesc.h"
#include "libavutil/stereo3d.h"
#include "avcodec.h"
#include "blockdsp.h"
#include "get_bits.h"
#include "hpeldsp.h"
#include "idctdsp.h"
#undef near 
#define MAX_COMPONENTS 4
typedef struct MJpegDecodeContext {
AVClass *class;
AVCodecContext *avctx;
GetBitContext gb;
int buf_size;
int start_code; 
int buffer_size;
uint8_t *buffer;
uint16_t quant_matrixes[4][64];
VLC vlcs[3][4];
int qscale[4]; 
int org_height; 
int first_picture; 
int interlaced; 
int bottom_field; 
int lossless;
int ls;
int progressive;
int bayer; 
int rgb;
uint8_t upscale_h[4];
uint8_t upscale_v[4];
int rct; 
int pegasus_rct; 
int bits; 
int colr;
int xfrm;
int adobe_transform;
int maxval;
int near; 
int t1,t2,t3;
int reset; 
int width, height;
int mb_width, mb_height;
int nb_components;
int block_stride[MAX_COMPONENTS];
int component_id[MAX_COMPONENTS];
int h_count[MAX_COMPONENTS]; 
int v_count[MAX_COMPONENTS];
int comp_index[MAX_COMPONENTS];
int dc_index[MAX_COMPONENTS];
int ac_index[MAX_COMPONENTS];
int nb_blocks[MAX_COMPONENTS];
int h_scount[MAX_COMPONENTS];
int v_scount[MAX_COMPONENTS];
int quant_sindex[MAX_COMPONENTS];
int h_max, v_max; 
int quant_index[4]; 
int last_dc[MAX_COMPONENTS]; 
AVFrame *picture; 
AVFrame *picture_ptr; 
int got_picture; 
int linesize[MAX_COMPONENTS]; 
int8_t *qscale_table;
DECLARE_ALIGNED(32, int16_t, block)[64];
int16_t (*blocks[MAX_COMPONENTS])[64]; 
uint8_t *last_nnz[MAX_COMPONENTS];
uint64_t coefs_finished[MAX_COMPONENTS]; 
int palette_index;
ScanTable scantable;
BlockDSPContext bdsp;
HpelDSPContext hdsp;
IDCTDSPContext idsp;
int restart_interval;
int restart_count;
int buggy_avid;
int cs_itu601;
int interlace_polarity;
int multiscope;
int mjpb_skiptosod;
int cur_scan; 
int flipped; 
uint16_t (*ljpeg_buffer)[4];
unsigned int ljpeg_buffer_size;
int extern_huff;
AVDictionary *exif_metadata;
AVStereo3D *stereo3d; 
const AVPixFmtDescriptor *pix_desc;
uint8_t **iccdata;
int *iccdatalens;
int iccnum;
int iccread;
const uint8_t *raw_image_buffer;
size_t raw_image_buffer_size;
const uint8_t *raw_scan_buffer;
size_t raw_scan_buffer_size;
uint8_t raw_huffman_lengths[2][4][16];
uint8_t raw_huffman_values[2][4][256];
enum AVPixelFormat hwaccel_sw_pix_fmt;
enum AVPixelFormat hwaccel_pix_fmt;
void *hwaccel_picture_private;
} MJpegDecodeContext;
int ff_mjpeg_decode_init(AVCodecContext *avctx);
int ff_mjpeg_decode_end(AVCodecContext *avctx);
int ff_mjpeg_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt);
int ff_mjpeg_decode_dqt(MJpegDecodeContext *s);
int ff_mjpeg_decode_dht(MJpegDecodeContext *s);
int ff_mjpeg_decode_sof(MJpegDecodeContext *s);
int ff_mjpeg_decode_sos(MJpegDecodeContext *s,
const uint8_t *mb_bitmask,int mb_bitmask_size,
const AVFrame *reference);
int ff_mjpeg_find_marker(MJpegDecodeContext *s,
const uint8_t **buf_ptr, const uint8_t *buf_end,
const uint8_t **unescaped_buf_ptr, int *unescaped_buf_size);
