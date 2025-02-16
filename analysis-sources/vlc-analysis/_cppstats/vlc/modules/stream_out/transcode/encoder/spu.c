#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_modules.h>
#include <vlc_codec.h>
#include <vlc_sout.h>
#include "encoder.h"
#include "encoder_priv.h"
int transcode_encoder_spu_open( transcode_encoder_t *p_enc,
const transcode_encoder_config_t *p_cfg )
{
p_enc->p_encoder->p_cfg = p_cfg->p_config_chain;
p_enc->p_encoder->fmt_out.i_codec = p_cfg->i_codec;
p_enc->p_encoder->p_module = module_need( p_enc->p_encoder, "encoder",
p_cfg->psz_name, true );
return ( p_enc->p_encoder->p_module ) ? VLC_SUCCESS: VLC_EGENERIC;
}
block_t * transcode_encoder_spu_encode( transcode_encoder_t *p_enc, subpicture_t *p_spu )
{
return p_enc->p_encoder->pf_encode_sub( p_enc->p_encoder, p_spu );
}
