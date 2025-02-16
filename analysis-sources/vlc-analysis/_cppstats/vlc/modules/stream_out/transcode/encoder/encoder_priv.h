#include <vlc_picture_fifo.h>
struct transcode_encoder_t
{
encoder_t *p_encoder;
vlc_thread_t thread;
vlc_mutex_t lock_out;
bool b_abort;
picture_fifo_t *pp_pics;
vlc_sem_t picture_pool_has_room;
vlc_cond_t cond;
block_t *p_buffers;
bool b_threaded;
};
int transcode_encoder_audio_open( transcode_encoder_t *p_enc,
const transcode_encoder_config_t *p_cfg );
int transcode_encoder_video_open( transcode_encoder_t *p_enc,
const transcode_encoder_config_t *p_cfg );
int transcode_encoder_spu_open( transcode_encoder_t *p_enc,
const transcode_encoder_config_t *p_cfg );
void transcode_encoder_video_close( transcode_encoder_t *p_enc );
block_t * transcode_encoder_video_encode( transcode_encoder_t *p_enc, picture_t *p_pic );
block_t * transcode_encoder_audio_encode( transcode_encoder_t *p_enc, block_t *p_block );
block_t * transcode_encoder_spu_encode( transcode_encoder_t *p_enc, subpicture_t *p_spu );
int transcode_encoder_audio_drain( transcode_encoder_t *p_enc, block_t **out );
int transcode_encoder_video_drain( transcode_encoder_t *p_enc, block_t **out );
int transcode_encoder_video_test( encoder_t *p_encoder,
const transcode_encoder_config_t *p_cfg,
const es_format_t *p_dec_fmtin,
vlc_fourcc_t i_codec_in,
es_format_t *p_enc_wanted_in );
int transcode_encoder_audio_test( encoder_t *p_encoder,
const transcode_encoder_config_t *p_cfg,
const es_format_t *p_dec_out,
vlc_fourcc_t i_codec_in,
es_format_t *p_enc_wanted_in );
