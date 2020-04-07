#include "avcodec.h"
#include "dv_profile.h"
#include "me_cmp.h"
#include "vlc.h"
#include "idctdsp.h"
typedef struct DVwork_chunk {
uint16_t buf_offset;
uint16_t mb_coordinates[5];
} DVwork_chunk;
typedef struct DVVideoContext {
AVClass *avclass;
const AVDVProfile *sys;
const AVFrame *frame;
AVCodecContext *avctx;
uint8_t *buf;
uint8_t dv_zigzag[2][64];
void (*get_pixels)(int16_t *block, const uint8_t *pixels, ptrdiff_t linesize);
void (*fdct[2])(int16_t *block);
void (*idct_put[2])(uint8_t *dest, ptrdiff_t stride, int16_t *block);
me_cmp_func ildct_cmp;
DVwork_chunk work_chunks[4 * 12 * 27];
uint32_t idct_factor[2 * 4 * 16 * 64];
IDCTDSPContext idsp;
int quant_deadzone;
} DVVideoContext;
enum dv_section_type {
dv_sect_header = 0x1f,
dv_sect_subcode = 0x3f,
dv_sect_vaux = 0x56,
dv_sect_audio = 0x76,
dv_sect_video = 0x96,
};
enum dv_pack_type {
dv_header525 = 0x3f, 
dv_header625 = 0xbf, 
dv_timecode = 0x13,
dv_audio_source = 0x50,
dv_audio_control = 0x51,
dv_audio_recdate = 0x52,
dv_audio_rectime = 0x53,
dv_video_source = 0x60,
dv_video_control = 0x61,
dv_video_recdate = 0x62,
dv_video_rectime = 0x63,
dv_unknown_pack = 0xff,
};
#define DV_PROFILE_IS_HD(p) ((p)->video_stype & 0x10)
#define DV_PROFILE_IS_1080i50(p) (((p)->video_stype == 0x14) && ((p)->dsf == 1))
#define DV_PROFILE_IS_1080i60(p) (((p)->video_stype == 0x14) && ((p)->dsf == 0))
#define DV_PROFILE_IS_720p50(p) (((p)->video_stype == 0x18) && ((p)->dsf == 1))
#define DV_MAX_FRAME_SIZE 576000
#define DV_MAX_BPM 8
#define TEX_VLC_BITS 10
extern RL_VLC_ELEM ff_dv_rl_vlc[1664];
int ff_dv_init_dynamic_tables(DVVideoContext *s, const AVDVProfile *d);
int ff_dvvideo_init(AVCodecContext *avctx);
static inline int dv_work_pool_size(const AVDVProfile *d)
{
int size = d->n_difchan * d->difseg_size * 27;
if (DV_PROFILE_IS_1080i50(d))
size -= 3 * 27;
if (DV_PROFILE_IS_720p50(d))
size -= 4 * 27;
return size;
}
static inline void dv_calculate_mb_xy(DVVideoContext *s,
DVwork_chunk *work_chunk,
int m, int *mb_x, int *mb_y)
{
*mb_x = work_chunk->mb_coordinates[m] & 0xff;
*mb_y = work_chunk->mb_coordinates[m] >> 8;
if (s->sys->height == 720 && !(s->buf[1] & 0x0C))
*mb_y -= (*mb_y > 17) ? 18 : -72;
}
