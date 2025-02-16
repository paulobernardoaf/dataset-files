#include <vlc_common.h>
#include "../../video_output/android/utils.h"
typedef struct mc_api mc_api;
typedef struct mc_api_sys mc_api_sys;
typedef struct mc_api_out mc_api_out;
typedef int (*pf_MediaCodecApi_init)(mc_api*);
int MediaCodecJni_Init(mc_api*);
int MediaCodecNdk_Init(mc_api*);
#define MC_API_ERROR (-1)
#define MC_API_INFO_TRYAGAIN (-11)
#define MC_API_INFO_OUTPUT_FORMAT_CHANGED (-12)
#define MC_API_INFO_OUTPUT_BUFFERS_CHANGED (-13)
#define MC_API_NO_QUIRKS 0
#define MC_API_QUIRKS_NEED_CSD 0x1
#define MC_API_VIDEO_QUIRKS_IGNORE_PADDING 0x2
#define MC_API_VIDEO_QUIRKS_SUPPORT_INTERLACED 0x4
#define MC_API_AUDIO_QUIRKS_NEED_CHANNELS 0x8
#define MC_API_VIDEO_QUIRKS_ADAPTIVE 0x1000
#define MC_API_VIDEO_QUIRKS_IGNORE_SIZE 0x2000
struct mc_api_out
{
enum {
MC_OUT_TYPE_BUF,
MC_OUT_TYPE_CONF,
} type;
bool b_eos;
union
{
struct
{
int i_index;
vlc_tick_t i_ts;
const uint8_t *p_ptr;
size_t i_size;
} buf;
union
{
struct
{
unsigned int width, height;
unsigned int stride;
unsigned int slice_height;
int pixel_format;
int crop_left;
int crop_top;
int crop_right;
int crop_bottom;
} video;
struct
{
int channel_count;
int channel_mask;
int sample_rate;
} audio;
} conf;
};
};
union mc_api_args
{
struct
{
void *p_surface;
void *p_jsurface;
int i_width;
int i_height;
int i_angle;
bool b_tunneled_playback;
bool b_adaptive_playback;
} video;
struct
{
int i_sample_rate;
int i_channel_count;
} audio;
};
struct mc_api
{
mc_api_sys *p_sys;
vlc_object_t * p_obj;
const char * psz_mime;
enum es_format_category_e i_cat;
vlc_fourcc_t i_codec;
int i_quirks;
char *psz_name;
bool b_support_rotation;
bool b_started;
bool b_direct_rendering;
void (*clean)(mc_api *);
int (*prepare)(mc_api *, int i_profile);
int (*configure_decoder)(mc_api *, union mc_api_args* p_args);
int (*start)(mc_api *);
int (*stop)(mc_api *);
int (*flush)(mc_api *);
int (*dequeue_in)(mc_api *, vlc_tick_t i_timeout);
int (*dequeue_out)(mc_api *, vlc_tick_t i_timeout);
int (*queue_in)(mc_api *, int i_index, const void *p_buf, size_t i_size,
vlc_tick_t i_ts, bool b_config);
int (*get_out)(mc_api *, int i_index, mc_api_out *p_out);
int (*release_out)(mc_api *, int i_index, bool b_render);
int (*release_out_ts)(mc_api *, int i_index, int64_t i_ts_ns);
int (*set_output_surface)(mc_api*, void *p_surface, void *p_jsurface);
};
