#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_aout.h>
#include <vlc_codec.h>
#include <vlc_avcodec.h>
#include "avcodec.h"
#include <libavcodec/avcodec.h>
#include <libavutil/mem.h>
#include <libavutil/channel_layout.h>
typedef struct
{
AVCodecContext *p_context;
const AVCodec *p_codec;
audio_sample_format_t aout_format;
date_t end_date;
int i_reject_count;
bool b_extract;
int pi_extraction[AOUT_CHAN_MAX];
int i_previous_channels;
uint64_t i_previous_layout;
} decoder_sys_t;
#define BLOCK_FLAG_PRIVATE_REALLOCATED (1 << BLOCK_FLAG_PRIVATE_SHIFT)
static void SetupOutputFormat( decoder_t *p_dec, bool b_trust );
static block_t * ConvertAVFrame( decoder_t *p_dec, AVFrame *frame );
static int DecodeAudio( decoder_t *, block_t * );
static void Flush( decoder_t * );
static void InitDecoderConfig( decoder_t *p_dec, AVCodecContext *p_context )
{
if( p_dec->fmt_in.i_extra > 0 )
{
const uint8_t * const p_src = p_dec->fmt_in.p_extra;
int i_offset = 0;
int i_size = p_dec->fmt_in.i_extra;
if( p_dec->fmt_in.i_codec == VLC_CODEC_ALAC )
{
static const uint8_t p_pattern[] = { 0, 0, 0, 36, 'a', 'l', 'a', 'c' };
for( i_offset = 0; i_offset < i_size - (int)sizeof(p_pattern); i_offset++ )
{
if( !memcmp( &p_src[i_offset], p_pattern, sizeof(p_pattern) ) )
break;
}
i_size = __MIN( p_dec->fmt_in.i_extra - i_offset, 36 );
if( i_size < 36 )
i_size = 0;
}
if( i_size > 0 )
{
p_context->extradata =
av_malloc( i_size + FF_INPUT_BUFFER_PADDING_SIZE );
if( p_context->extradata )
{
uint8_t *p_dst = p_context->extradata;
p_context->extradata_size = i_size;
memcpy( &p_dst[0], &p_src[i_offset], i_size );
memset( &p_dst[i_size], 0, FF_INPUT_BUFFER_PADDING_SIZE );
}
}
}
else
{
p_context->extradata_size = 0;
p_context->extradata = NULL;
}
}
static int OpenAudioCodec( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *ctx = p_sys->p_context;
const AVCodec *codec = p_sys->p_codec;
if( ctx->extradata_size <= 0 )
{
if( codec->id == AV_CODEC_ID_VORBIS ||
( codec->id == AV_CODEC_ID_AAC &&
!p_dec->fmt_in.b_packetized ) )
{
msg_Warn( p_dec, "waiting for extra data for codec %s",
codec->name );
return 1;
}
}
ctx->sample_rate = p_dec->fmt_in.audio.i_rate;
ctx->channels = p_dec->fmt_in.audio.i_channels;
ctx->block_align = p_dec->fmt_in.audio.i_blockalign;
ctx->bit_rate = p_dec->fmt_in.i_bitrate;
ctx->bits_per_coded_sample = p_dec->fmt_in.audio.i_bitspersample;
if( codec->id == AV_CODEC_ID_ADPCM_G726 &&
ctx->bit_rate > 0 &&
ctx->sample_rate > 0)
ctx->bits_per_coded_sample = ctx->bit_rate / ctx->sample_rate;
return ffmpeg_OpenCodec( p_dec, ctx, codec );
}
typedef struct
{
block_t self;
AVFrame *frame;
} vlc_av_frame_t;
static void vlc_av_frame_Release(block_t *block)
{
vlc_av_frame_t *b = (void *)block;
av_frame_free(&b->frame);
free(b);
}
static const struct vlc_block_callbacks vlc_av_frame_cbs =
{
vlc_av_frame_Release,
};
static block_t *vlc_av_frame_Wrap(AVFrame *frame)
{
for (unsigned i = 1; i < AV_NUM_DATA_POINTERS; i++)
assert(frame->linesize[i] == 0); 
if (av_frame_make_writable(frame)) 
return NULL;
vlc_av_frame_t *b = malloc(sizeof (*b));
if (unlikely(b == NULL))
return NULL;
block_t *block = &b->self;
block_Init(block, &vlc_av_frame_cbs,
frame->extended_data[0], frame->linesize[0]);
block->i_nb_samples = frame->nb_samples;
b->frame = frame;
return block;
}
void EndAudioDec( vlc_object_t *obj )
{
decoder_t *p_dec = (decoder_t *)obj;
decoder_sys_t *sys = p_dec->p_sys;
AVCodecContext *ctx = sys->p_context;
avcodec_free_context( &ctx );
free( sys );
}
int InitAudioDec( vlc_object_t *obj )
{
decoder_t *p_dec = (decoder_t *)obj;
const AVCodec *codec;
AVCodecContext *avctx = ffmpeg_AllocContext( p_dec, &codec );
if( avctx == NULL )
return VLC_EGENERIC;
decoder_sys_t *p_sys = malloc(sizeof(*p_sys));
if( unlikely(p_sys == NULL) )
{
avcodec_free_context( &avctx );
return VLC_ENOMEM;
}
p_dec->p_sys = p_sys;
p_sys->p_context = avctx;
p_sys->p_codec = codec;
InitDecoderConfig( p_dec, avctx );
if( OpenAudioCodec( p_dec ) < 0 )
{
free( p_sys );
avcodec_free_context( &avctx );
return VLC_EGENERIC;
}
p_sys->i_reject_count = 0;
p_sys->b_extract = false;
p_sys->i_previous_channels = 0;
p_sys->i_previous_layout = 0;
SetupOutputFormat( p_dec, false );
if( !p_dec->fmt_out.audio.i_rate )
p_dec->fmt_out.audio.i_rate = p_dec->fmt_in.audio.i_rate;
if( p_dec->fmt_out.audio.i_rate )
date_Init( &p_sys->end_date, p_dec->fmt_out.audio.i_rate, 1 );
p_dec->fmt_out.audio.i_chan_mode = p_dec->fmt_in.audio.i_chan_mode;
p_dec->pf_decode = DecodeAudio;
p_dec->pf_flush = Flush;
if( avctx->profile != FF_PROFILE_UNKNOWN )
p_dec->fmt_in.i_profile = avctx->profile;
if( avctx->level != FF_LEVEL_UNKNOWN )
p_dec->fmt_in.i_level = avctx->level;
return VLC_SUCCESS;
}
static void Flush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *ctx = p_sys->p_context;
if( avcodec_is_open( ctx ) )
avcodec_flush_buffers( ctx );
date_Set( &p_sys->end_date, VLC_TICK_INVALID );
if( ctx->codec_id == AV_CODEC_ID_MP2 ||
ctx->codec_id == AV_CODEC_ID_MP3 )
p_sys->i_reject_count = 3;
}
static int DecodeBlock( decoder_t *p_dec, block_t **pp_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *ctx = p_sys->p_context;
AVFrame *frame = NULL;
block_t *p_block = NULL;
bool b_error = false;
if( !ctx->extradata_size && p_dec->fmt_in.i_extra
&& !avcodec_is_open( ctx ) )
{
InitDecoderConfig( p_dec, ctx );
OpenAudioCodec( p_dec );
}
if( !avcodec_is_open( ctx ) )
{
if( pp_block )
p_block = *pp_block;
goto drop;
}
if( pp_block == NULL ) 
{
(void) avcodec_send_packet( ctx, NULL );
}
else
{
p_block = *pp_block;
}
if( p_block )
{
if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
Flush( p_dec );
goto drop;
}
if( p_block->i_flags & BLOCK_FLAG_DISCONTINUITY )
{
date_Set( &p_sys->end_date, VLC_TICK_INVALID );
}
if( p_block->i_pts == VLC_TICK_INVALID &&
date_Get( &p_sys->end_date ) == VLC_TICK_INVALID )
goto drop;
if( p_block->i_buffer <= 0 )
goto drop;
if( (p_block->i_flags & BLOCK_FLAG_PRIVATE_REALLOCATED) == 0 )
{
*pp_block = p_block = block_Realloc( p_block, 0, p_block->i_buffer + FF_INPUT_BUFFER_PADDING_SIZE );
if( !p_block )
goto end;
p_block->i_buffer -= FF_INPUT_BUFFER_PADDING_SIZE;
memset( &p_block->p_buffer[p_block->i_buffer], 0, FF_INPUT_BUFFER_PADDING_SIZE );
p_block->i_flags |= BLOCK_FLAG_PRIVATE_REALLOCATED;
}
}
frame = av_frame_alloc();
if (unlikely(frame == NULL))
goto end;
for( int ret = 0; ret == 0; )
{
if( p_block )
{
AVPacket pkt;
av_init_packet( &pkt );
pkt.data = p_block->p_buffer;
pkt.size = p_block->i_buffer;
ret = avcodec_send_packet( ctx, &pkt );
if( ret == 0 ) 
{
if( p_block->i_pts > date_Get( &p_sys->end_date ) )
{
date_Set( &p_sys->end_date, p_block->i_pts );
}
block_Release( p_block );
*pp_block = p_block = NULL;
}
else if ( ret != AVERROR(EAGAIN) ) 
{
if( ret == AVERROR(ENOMEM) || ret == AVERROR(EINVAL) )
goto end;
else
{
char errorstring[AV_ERROR_MAX_STRING_SIZE];
if( !av_strerror( ret, errorstring, AV_ERROR_MAX_STRING_SIZE ) )
msg_Err( p_dec, "%s", errorstring );
goto drop;
}
}
}
ret = avcodec_receive_frame( ctx, frame );
if( ret == 0 )
{
if( ctx->channels <= 0 || ctx->channels > INPUT_CHAN_MAX
|| ctx->sample_rate <= 0 )
{
msg_Warn( p_dec, "invalid audio properties channels count %d, sample rate %d",
ctx->channels, ctx->sample_rate );
goto drop;
}
else if( p_dec->fmt_out.audio.i_rate != (unsigned int)ctx->sample_rate )
{
date_Init( &p_sys->end_date, ctx->sample_rate, 1 );
}
SetupOutputFormat( p_dec, true );
if( decoder_UpdateAudioFormat( p_dec ) )
goto drop;
block_t *p_converted = ConvertAVFrame( p_dec, frame ); 
if( p_converted )
{
if( p_sys->i_reject_count > 0 )
{
memset( p_converted->p_buffer, 0, p_converted->i_buffer );
p_sys->i_reject_count--;
}
p_converted->i_buffer = p_converted->i_nb_samples
* p_dec->fmt_out.audio.i_bytes_per_frame;
p_converted->i_pts = date_Get( &p_sys->end_date );
p_converted->i_length = date_Increment( &p_sys->end_date,
p_converted->i_nb_samples ) - p_converted->i_pts;
decoder_QueueAudio( p_dec, p_converted );
}
frame = av_frame_alloc();
if (unlikely(frame == NULL))
break;
}
else
{
if( ret == AVERROR_EOF )
avcodec_flush_buffers( p_sys->p_context );
av_frame_free( &frame );
}
};
return VLCDEC_SUCCESS;
end:
b_error = true;
drop:
if( pp_block )
{
assert( *pp_block == p_block );
*pp_block = NULL;
}
if( p_block != NULL )
block_Release(p_block);
if( frame != NULL )
av_frame_free( &frame );
return (b_error) ? VLCDEC_ECRITICAL : VLCDEC_SUCCESS;
}
static int DecodeAudio( decoder_t *p_dec, block_t *p_block )
{
block_t **pp_block = p_block ? &p_block : NULL;
int i_ret;
do
{
i_ret = DecodeBlock( p_dec, pp_block );
}
while( i_ret == VLCDEC_SUCCESS && pp_block && *pp_block );
return i_ret;
}
static block_t * ConvertAVFrame( decoder_t *p_dec, AVFrame *frame )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *ctx = p_sys->p_context;
block_t *p_block;
if( av_sample_fmt_is_planar( ctx->sample_fmt ) )
{
p_block = block_Alloc(frame->linesize[0] * ctx->channels);
if ( likely(p_block) )
{
const void *planes[ctx->channels];
for (int i = 0; i < ctx->channels; i++)
planes[i] = frame->extended_data[i];
aout_Interleave(p_block->p_buffer, planes, frame->nb_samples,
ctx->channels, p_dec->fmt_out.audio.i_format);
p_block->i_nb_samples = frame->nb_samples;
}
av_frame_free(&frame);
}
else
{
p_block = vlc_av_frame_Wrap(frame);
frame = NULL;
}
if (p_sys->b_extract && p_block)
{ 
block_t *p_buffer = block_Alloc( p_dec->fmt_out.audio.i_bytes_per_frame
* p_block->i_nb_samples );
if( likely(p_buffer) )
{
aout_ChannelExtract( p_buffer->p_buffer,
p_dec->fmt_out.audio.i_channels,
p_block->p_buffer, ctx->channels,
p_block->i_nb_samples, p_sys->pi_extraction,
p_dec->fmt_out.audio.i_bitspersample );
p_buffer->i_nb_samples = p_block->i_nb_samples;
}
block_Release( p_block );
p_block = p_buffer;
}
return p_block;
}
vlc_fourcc_t GetVlcAudioFormat( int fmt )
{
static const vlc_fourcc_t fcc[] = {
[AV_SAMPLE_FMT_U8] = VLC_CODEC_U8,
[AV_SAMPLE_FMT_S16] = VLC_CODEC_S16N,
[AV_SAMPLE_FMT_S32] = VLC_CODEC_S32N,
[AV_SAMPLE_FMT_FLT] = VLC_CODEC_FL32,
[AV_SAMPLE_FMT_DBL] = VLC_CODEC_FL64,
[AV_SAMPLE_FMT_U8P] = VLC_CODEC_U8,
[AV_SAMPLE_FMT_S16P] = VLC_CODEC_S16N,
[AV_SAMPLE_FMT_S32P] = VLC_CODEC_S32N,
[AV_SAMPLE_FMT_FLTP] = VLC_CODEC_FL32,
[AV_SAMPLE_FMT_DBLP] = VLC_CODEC_FL64,
};
if( (sizeof(fcc) / sizeof(fcc[0])) > (unsigned)fmt )
return fcc[fmt];
return VLC_CODEC_S16N;
}
static const uint64_t pi_channels_map[][2] =
{
{ AV_CH_FRONT_LEFT, AOUT_CHAN_LEFT },
{ AV_CH_FRONT_RIGHT, AOUT_CHAN_RIGHT },
{ AV_CH_FRONT_CENTER, AOUT_CHAN_CENTER },
{ AV_CH_LOW_FREQUENCY, AOUT_CHAN_LFE },
{ AV_CH_BACK_LEFT, AOUT_CHAN_REARLEFT },
{ AV_CH_BACK_RIGHT, AOUT_CHAN_REARRIGHT },
{ AV_CH_FRONT_LEFT_OF_CENTER, 0 },
{ AV_CH_FRONT_RIGHT_OF_CENTER, 0 },
{ AV_CH_BACK_CENTER, AOUT_CHAN_REARCENTER },
{ AV_CH_SIDE_LEFT, AOUT_CHAN_MIDDLELEFT },
{ AV_CH_SIDE_RIGHT, AOUT_CHAN_MIDDLERIGHT },
{ AV_CH_TOP_CENTER, 0 },
{ AV_CH_TOP_FRONT_LEFT, 0 },
{ AV_CH_TOP_FRONT_CENTER, 0 },
{ AV_CH_TOP_FRONT_RIGHT, 0 },
{ AV_CH_TOP_BACK_LEFT, 0 },
{ AV_CH_TOP_BACK_CENTER, 0 },
{ AV_CH_TOP_BACK_RIGHT, 0 },
{ AV_CH_STEREO_LEFT, 0 },
{ AV_CH_STEREO_RIGHT, 0 },
};
static void SetupOutputFormat( decoder_t *p_dec, bool b_trust )
{
decoder_sys_t *p_sys = p_dec->p_sys;
p_dec->fmt_out.i_codec = GetVlcAudioFormat( p_sys->p_context->sample_fmt );
p_dec->fmt_out.audio.channel_type = p_dec->fmt_in.audio.channel_type;
p_dec->fmt_out.audio.i_format = p_dec->fmt_out.i_codec;
p_dec->fmt_out.audio.i_rate = p_sys->p_context->sample_rate;
if( p_sys->i_previous_channels == p_sys->p_context->channels &&
p_sys->i_previous_layout == p_sys->p_context->channel_layout )
return;
if( b_trust )
{
p_sys->i_previous_channels = p_sys->p_context->channels;
p_sys->i_previous_layout = p_sys->p_context->channel_layout;
}
const unsigned i_order_max = sizeof(pi_channels_map)/sizeof(*pi_channels_map);
uint32_t pi_order_src[i_order_max];
int i_channels_src = 0;
uint64_t channel_layout =
p_sys->p_context->channel_layout ? p_sys->p_context->channel_layout :
(uint64_t)av_get_default_channel_layout( p_sys->p_context->channels );
if( channel_layout )
{
for( unsigned i = 0; i < i_order_max
&& i_channels_src < p_sys->p_context->channels; i++ )
{
if( channel_layout & pi_channels_map[i][0] )
pi_order_src[i_channels_src++] = pi_channels_map[i][1];
}
if( i_channels_src != p_sys->p_context->channels && b_trust )
msg_Err( p_dec, "Channel layout not understood" );
if( i_channels_src == 2 && pi_order_src[0] == AOUT_CHAN_CENTER
&& pi_order_src[1] == AOUT_CHAN_CENTER )
{
p_dec->fmt_out.audio.i_chan_mode |= AOUT_CHANMODE_DUALMONO;
pi_order_src[0] = AOUT_CHAN_LEFT;
pi_order_src[1] = AOUT_CHAN_RIGHT;
}
uint32_t i_layout_dst;
int i_channels_dst;
p_sys->b_extract = aout_CheckChannelExtraction( p_sys->pi_extraction,
&i_layout_dst, &i_channels_dst,
NULL, pi_order_src, i_channels_src );
if( i_channels_dst != i_channels_src && b_trust )
msg_Warn( p_dec, "%d channels are dropped", i_channels_src - i_channels_dst );
if (p_dec->fmt_out.audio.channel_type == AUDIO_CHANNEL_TYPE_AMBISONICS
&& p_dec->fmt_in.i_codec == VLC_CODEC_MP4A
&& i_channels_src == 4)
p_sys->b_extract = false;
p_dec->fmt_out.audio.i_physical_channels = i_layout_dst;
}
else
{
msg_Warn( p_dec, "no channel layout found");
p_dec->fmt_out.audio.i_physical_channels = 0;
p_dec->fmt_out.audio.i_channels = p_sys->p_context->channels;
}
aout_FormatPrepare( &p_dec->fmt_out.audio );
}
