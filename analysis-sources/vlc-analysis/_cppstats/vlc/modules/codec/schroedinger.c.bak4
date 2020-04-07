





























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_timestamp_helper.h>

#include <schroedinger/schro.h>




static int OpenDecoder ( vlc_object_t * );
static void CloseDecoder ( vlc_object_t * );
static int OpenEncoder ( vlc_object_t * );
static void CloseEncoder ( vlc_object_t * );

#define ENC_CFG_PREFIX "sout-schro-"

#define ENC_CHROMAFMT "chroma-fmt"
#define ENC_CHROMAFMT_TEXT N_("Chroma format")
#define ENC_CHROMAFMT_LONGTEXT N_("Picking chroma format will force a " "conversion of the video into that format")

static const char *const enc_chromafmt_list[] =
{ "420", "422", "444" };
static const char *const enc_chromafmt_list_text[] =
{ N_("4:2:0"), N_("4:2:2"), N_("4:4:4") };

#define ENC_RATE_CONTROL "rate-control"
#define ENC_RATE_CONTROL_TEXT N_("Rate control method")
#define ENC_RATE_CONTROL_LONGTEXT N_("Method used to encode the video sequence")

static const char *enc_rate_control_list[] = {
"constant_noise_threshold",
"constant_bitrate",
"low_delay",
"lossless",
"constant_lambda",
"constant_error",
"constant_quality"
};

static const char *enc_rate_control_list_text[] = {
N_("Constant noise threshold mode"),
N_("Constant bitrate mode (CBR)"),
N_("Low Delay mode"),
N_("Lossless mode"),
N_("Constant lambda mode"),
N_("Constant error mode"),
N_("Constant quality mode")
};

#define ENC_GOP_STRUCTURE "gop-structure"
#define ENC_GOP_STRUCTURE_TEXT N_("GOP structure")
#define ENC_GOP_STRUCTURE_LONGTEXT N_("GOP structure used to encode the video sequence")

static const char *enc_gop_structure_list[] = {
"adaptive",
"intra_only",
"backref",
"chained_backref",
"biref",
"chained_biref"
};

static const char *enc_gop_structure_list_text[] = {
N_("No fixed gop structure. A picture can be intra or inter and refer to previous or future pictures."),
N_("I-frame only sequence"),
N_("Inter pictures refere to previous pictures only"),
N_("Inter pictures refere to previous pictures only"),
N_("Inter pictures can refer to previous or future pictures"),
N_("Inter pictures can refer to previous or future pictures")
};

#define ENC_QUALITY "quality"
#define ENC_QUALITY_TEXT N_("Constant quality factor")
#define ENC_QUALITY_LONGTEXT N_("Quality factor to use in constant quality mode")

#define ENC_NOISE_THRESHOLD "noise-threshold"
#define ENC_NOISE_THRESHOLD_TEXT N_("Noise Threshold")
#define ENC_NOISE_THRESHOLD_LONGTEXT N_("Noise threshold to use in constant noise threshold mode")

#define ENC_BITRATE "bitrate"
#define ENC_BITRATE_TEXT N_("CBR bitrate (kbps)")
#define ENC_BITRATE_LONGTEXT N_("Target bitrate in kbps when encoding in constant bitrate mode")

#define ENC_MAX_BITRATE "max-bitrate"
#define ENC_MAX_BITRATE_TEXT N_("Maximum bitrate (kbps)")
#define ENC_MAX_BITRATE_LONGTEXT N_("Maximum bitrate in kbps when encoding in constant bitrate mode")

#define ENC_MIN_BITRATE "min-bitrate"
#define ENC_MIN_BITRATE_TEXT N_("Minimum bitrate (kbps)")
#define ENC_MIN_BITRATE_LONGTEXT N_("Minimum bitrate in kbps when encoding in constant bitrate mode")

#define ENC_AU_DISTANCE "gop-length"
#define ENC_AU_DISTANCE_TEXT N_("GOP length")
#define ENC_AU_DISTANCE_LONGTEXT N_("Number of pictures between successive sequence headers i.e. length of the group of pictures")


#define ENC_PREFILTER "filtering"
#define ENC_PREFILTER_TEXT N_("Prefilter")
#define ENC_PREFILTER_LONGTEXT N_("Enable adaptive prefiltering")

static const char *enc_filtering_list[] = {
"none",
"center_weighted_median",
"gaussian",
"add_noise",
"adaptive_gaussian",
"lowpass"
};

static const char *enc_filtering_list_text[] = {
N_("No pre-filtering"),
N_("Centre Weighted Median"),
N_("Gaussian Low Pass Filter"),
N_("Add Noise"),
N_("Gaussian Adaptive Low Pass Filter"),
N_("Low Pass Filter"),
};

#define ENC_PREFILTER_STRENGTH "filter-value"
#define ENC_PREFILTER_STRENGTH_TEXT N_("Amount of prefiltering")
#define ENC_PREFILTER_STRENGTH_LONGTEXT N_("Higher value implies more prefiltering")

#define ENC_CODINGMODE "coding-mode"
#define ENC_CODINGMODE_TEXT N_("Picture coding mode")
#define ENC_CODINGMODE_LONGTEXT N_("Field coding is where interlaced fields are coded" " separately as opposed to a pseudo-progressive frame")

static const char *const enc_codingmode_list[] =
{ "auto", "progressive", "field" };
static const char *const enc_codingmode_list_text[] =
{ N_("auto - let encoder decide based upon input (Best)"),
N_("force coding frame as single picture"),
N_("force coding frame as separate interlaced fields"),
};


#define ENC_MCBLK_SIZE "motion-block-size"
#define ENC_MCBLK_SIZE_TEXT N_("Size of motion compensation blocks")

static const char *enc_block_size_list[] = {
"automatic",
"small",
"medium",
"large"
};
static const char *const enc_block_size_list_text[] =
{ N_("automatic - let encoder decide based upon input (Best)"),
N_("small - use small motion compensation blocks"),
N_("medium - use medium motion compensation blocks"),
N_("large - use large motion compensation blocks"),
};


#define ENC_MCBLK_OVERLAP "motion-block-overlap"
#define ENC_MCBLK_OVERLAP_TEXT N_("Overlap of motion compensation blocks")

static const char *enc_block_overlap_list[] = {
"automatic",
"none",
"partial",
"full"
};
static const char *const enc_block_overlap_list_text[] =
{ N_("automatic - let encoder decide based upon input (Best)"),
N_("none - Motion compensation blocks do not overlap"),
N_("partial - Motion compensation blocks only partially overlap"),
N_("full - Motion compensation blocks fully overlap"),
};


#define ENC_MVPREC "mv-precision"
#define ENC_MVPREC_TEXT N_("Motion Vector precision")
#define ENC_MVPREC_LONGTEXT N_("Motion Vector precision in pels")
static const char *const enc_mvprec_list[] =
{ "1", "1/2", "1/4", "1/8" };


#define ENC_ME_COMBINED "me-combined"
#define ENC_ME_COMBINED_TEXT N_("Three component motion estimation")
#define ENC_ME_COMBINED_LONGTEXT N_("Use chroma as part of the motion estimation process")

#define ENC_DWTINTRA "intra-wavelet"
#define ENC_DWTINTRA_TEXT N_("Intra picture DWT filter")

#define ENC_DWTINTER "inter-wavelet"
#define ENC_DWTINTER_TEXT N_("Inter picture DWT filter")

static const char *enc_wavelet_list[] = {
"desl_dubuc_9_7",
"le_gall_5_3",
"desl_dubuc_13_7",
"haar_0",
"haar_1",
"fidelity",
"daub_9_7"
};

static const char *enc_wavelet_list_text[] = {
"Deslauriers-Dubuc (9,7)",
"LeGall (5,3)",
"Deslauriers-Dubuc (13,7)",
"Haar with no shift",
"Haar with single shift per level",
"Fidelity filter",
"Daubechies (9,7) integer approximation"
};

#define ENC_DWTDEPTH "transform-depth"
#define ENC_DWTDEPTH_TEXT N_("Number of DWT iterations")
#define ENC_DWTDEPTH_LONGTEXT N_("Also known as DWT levels")



#define ENC_MULTIQUANT "enable-multiquant"
#define ENC_MULTIQUANT_TEXT N_("Enable multiple quantizers")
#define ENC_MULTIQUANT_LONGTEXT N_("Enable multiple quantizers per subband (one per codeblock)")


#define ENC_NOAC "enable-noarith"
#define ENC_NOAC_TEXT N_("Disable arithmetic coding")
#define ENC_NOAC_LONGTEXT N_("Use variable length codes instead, useful for very high bitrates")



#define ENC_PWT "perceptual-weighting"
#define ENC_PWT_TEXT N_("perceptual weighting method")

static const char *enc_perceptual_weighting_list[] = {
"none",
"ccir959",
"moo",
"manos_sakrison"
};


#define ENC_PDIST "perceptual-distance"
#define ENC_PDIST_TEXT N_("perceptual distance")
#define ENC_PDIST_LONGTEXT N_("perceptual distance to calculate perceptual weight")


#define ENC_HSLICES "horiz-slices"
#define ENC_HSLICES_TEXT N_("Horizontal slices per frame")
#define ENC_HSLICES_LONGTEXT N_("Number of horizontal slices per frame in low delay mode")


#define ENC_VSLICES "vert-slices"
#define ENC_VSLICES_TEXT N_("Vertical slices per frame")
#define ENC_VSLICES_LONGTEXT N_("Number of vertical slices per frame in low delay mode")


#define ENC_SCBLK_SIZE "codeblock-size"
#define ENC_SCBLK_SIZE_TEXT N_("Size of code blocks in each subband")

static const char *enc_codeblock_size_list[] = {
"automatic",
"small",
"medium",
"large",
"full"
};
static const char *const enc_codeblock_size_list_text[] =
{ N_("automatic - let encoder decide based upon input (Best)"),
N_("small - use small code blocks"),
N_("medium - use medium sized code blocks"),
N_("large - use large code blocks"),
N_("full - One code block per subband"),
};


#define ENC_ME_HIERARCHICAL "enable-hierarchical-me"
#define ENC_ME_HIERARCHICAL_TEXT N_("Enable hierarchical Motion Estimation")


#define ENC_ME_DOWNSAMPLE_LEVELS "downsample-levels"
#define ENC_ME_DOWNSAMPLE_LEVELS_TEXT N_("Number of levels of downsampling")
#define ENC_ME_DOWNSAMPLE_LEVELS_LONGTEXT N_("Number of levels of downsampling in hierarchical motion estimation mode")


#define ENC_ME_GLOBAL_MOTION "enable-global-me"
#define ENC_ME_GLOBAL_MOTION_TEXT N_("Enable Global Motion Estimation")


#define ENC_ME_PHASECORR "enable-phasecorr-me"
#define ENC_ME_PHASECORR_TEXT N_("Enable Phase Correlation Estimation")


#define ENC_SCD "enable-scd"
#define ENC_SCD_TEXT N_("Enable Scene Change Detection")


#define ENC_FORCE_PROFILE "force-profile"
#define ENC_FORCE_PROFILE_TEXT N_("Force Profile")

static const char *enc_profile_list[] = {
"auto",
"vc2_low_delay",
"vc2_simple",
"vc2_main",
"main"
};

static const char *const enc_profile_list_text[] =
{ N_("automatic - let encoder decide based upon input (Best)"),
N_("VC2 Low Delay Profile"),
N_("VC2 Simple Profile"),
N_("VC2 Main Profile"),
N_("Main Profile"),
};

static const char *const ppsz_enc_options[] = {
ENC_RATE_CONTROL, ENC_GOP_STRUCTURE, ENC_QUALITY, ENC_NOISE_THRESHOLD, ENC_BITRATE,
ENC_MIN_BITRATE, ENC_MAX_BITRATE, ENC_AU_DISTANCE, ENC_CHROMAFMT,
ENC_PREFILTER, ENC_PREFILTER_STRENGTH, ENC_CODINGMODE, ENC_MCBLK_SIZE,
ENC_MCBLK_OVERLAP, ENC_MVPREC, ENC_ME_COMBINED, ENC_DWTINTRA, ENC_DWTINTER,
ENC_DWTDEPTH, ENC_MULTIQUANT, ENC_NOAC, ENC_PWT, ENC_PDIST, ENC_HSLICES,
ENC_VSLICES, ENC_SCBLK_SIZE, ENC_ME_HIERARCHICAL, ENC_ME_DOWNSAMPLE_LEVELS,
ENC_ME_GLOBAL_MOTION, ENC_ME_PHASECORR, ENC_SCD, ENC_FORCE_PROFILE,
NULL
};




vlc_module_begin ()
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_VCODEC )
set_shortname( "Schroedinger" )
set_description( N_("Dirac video decoder using libschroedinger") )
set_capability( "video decoder", 200 )
set_callbacks( OpenDecoder, CloseDecoder )
add_shortcut( "schroedinger" )


add_submodule()
set_section( N_("Encoding") , NULL )
set_description( N_("Dirac video encoder using libschroedinger") )
set_capability( "encoder", 110 )
set_callbacks( OpenEncoder, CloseEncoder )
add_shortcut( "schroedinger", "schro" )

add_string( ENC_CFG_PREFIX ENC_RATE_CONTROL, NULL,
ENC_RATE_CONTROL_TEXT, ENC_RATE_CONTROL_LONGTEXT, false )
change_string_list( enc_rate_control_list, enc_rate_control_list_text )

add_float( ENC_CFG_PREFIX ENC_QUALITY, -1.,
ENC_QUALITY_TEXT, ENC_QUALITY_LONGTEXT, false )
change_float_range(-1., 10.);

add_float( ENC_CFG_PREFIX ENC_NOISE_THRESHOLD, -1.,
ENC_NOISE_THRESHOLD_TEXT, ENC_NOISE_THRESHOLD_LONGTEXT, false )
change_float_range(-1., 100.);

add_integer( ENC_CFG_PREFIX ENC_BITRATE, -1,
ENC_BITRATE_TEXT, ENC_BITRATE_LONGTEXT, false )
change_integer_range(-1, INT_MAX);

add_integer( ENC_CFG_PREFIX ENC_MAX_BITRATE, -1,
ENC_MAX_BITRATE_TEXT, ENC_MAX_BITRATE_LONGTEXT, false )
change_integer_range(-1, INT_MAX);

add_integer( ENC_CFG_PREFIX ENC_MIN_BITRATE, -1,
ENC_MIN_BITRATE_TEXT, ENC_MIN_BITRATE_LONGTEXT, false )
change_integer_range(-1, INT_MAX);

add_string( ENC_CFG_PREFIX ENC_GOP_STRUCTURE, NULL,
ENC_GOP_STRUCTURE_TEXT, ENC_GOP_STRUCTURE_LONGTEXT, false )
change_string_list( enc_gop_structure_list, enc_gop_structure_list_text )

add_integer( ENC_CFG_PREFIX ENC_AU_DISTANCE, -1,
ENC_AU_DISTANCE_TEXT, ENC_AU_DISTANCE_LONGTEXT, false )
change_integer_range(-1, INT_MAX);

add_string( ENC_CFG_PREFIX ENC_CHROMAFMT, "420",
ENC_CHROMAFMT_TEXT, ENC_CHROMAFMT_LONGTEXT, false )
change_string_list( enc_chromafmt_list, enc_chromafmt_list_text )

add_string( ENC_CFG_PREFIX ENC_CODINGMODE, "auto",
ENC_CODINGMODE_TEXT, ENC_CODINGMODE_LONGTEXT, false )
change_string_list( enc_codingmode_list, enc_codingmode_list_text )

add_string( ENC_CFG_PREFIX ENC_MVPREC, NULL,
ENC_MVPREC_TEXT, ENC_MVPREC_LONGTEXT, false )
change_string_list( enc_mvprec_list, enc_mvprec_list )


add_string( ENC_CFG_PREFIX ENC_MCBLK_SIZE, NULL,
ENC_MCBLK_SIZE_TEXT, ENC_MCBLK_SIZE_TEXT, true )
change_string_list( enc_block_size_list, enc_block_size_list_text )



add_string( ENC_CFG_PREFIX ENC_MCBLK_OVERLAP, NULL,
ENC_MCBLK_OVERLAP_TEXT, ENC_MCBLK_OVERLAP_TEXT, true )
change_string_list( enc_block_overlap_list, enc_block_overlap_list_text )


add_integer( ENC_CFG_PREFIX ENC_ME_COMBINED, -1,
ENC_ME_COMBINED_TEXT, ENC_ME_COMBINED_LONGTEXT, true )
change_integer_range(-1, 1 );


add_integer( ENC_CFG_PREFIX ENC_ME_HIERARCHICAL, -1,
ENC_ME_HIERARCHICAL_TEXT, ENC_ME_HIERARCHICAL_TEXT, true )
change_integer_range(-1, 1 );


add_integer( ENC_CFG_PREFIX ENC_ME_DOWNSAMPLE_LEVELS, -1,
ENC_ME_DOWNSAMPLE_LEVELS_TEXT, ENC_ME_DOWNSAMPLE_LEVELS_LONGTEXT, true )
change_integer_range(-1, 8 );


add_integer( ENC_CFG_PREFIX ENC_ME_GLOBAL_MOTION, -1,
ENC_ME_GLOBAL_MOTION_TEXT, ENC_ME_GLOBAL_MOTION_TEXT, true )
change_integer_range(-1, 1 );


add_integer( ENC_CFG_PREFIX ENC_ME_PHASECORR, -1,
ENC_ME_PHASECORR_TEXT, ENC_ME_PHASECORR_TEXT, true )
change_integer_range(-1, 1 );

add_string( ENC_CFG_PREFIX ENC_DWTINTRA, NULL,
ENC_DWTINTRA_TEXT, ENC_DWTINTRA_TEXT, false )
change_string_list( enc_wavelet_list, enc_wavelet_list_text )

add_string( ENC_CFG_PREFIX ENC_DWTINTER, NULL,
ENC_DWTINTER_TEXT, ENC_DWTINTER_TEXT, false )
change_string_list( enc_wavelet_list, enc_wavelet_list_text )

add_integer( ENC_CFG_PREFIX ENC_DWTDEPTH, -1,
ENC_DWTDEPTH_TEXT, ENC_DWTDEPTH_LONGTEXT, false )
change_integer_range(-1, SCHRO_LIMIT_ENCODER_TRANSFORM_DEPTH );


add_integer( ENC_CFG_PREFIX ENC_MULTIQUANT, -1,
ENC_MULTIQUANT_TEXT, ENC_MULTIQUANT_LONGTEXT, true )
change_integer_range(-1, 1 );


add_string( ENC_CFG_PREFIX ENC_SCBLK_SIZE, NULL,
ENC_SCBLK_SIZE_TEXT, ENC_SCBLK_SIZE_TEXT, true )
change_string_list( enc_codeblock_size_list, enc_codeblock_size_list_text )

add_string( ENC_CFG_PREFIX ENC_PREFILTER, NULL,
ENC_PREFILTER_TEXT, ENC_PREFILTER_LONGTEXT, false )
change_string_list( enc_filtering_list, enc_filtering_list_text )

add_float( ENC_CFG_PREFIX ENC_PREFILTER_STRENGTH, -1.,
ENC_PREFILTER_STRENGTH_TEXT, ENC_PREFILTER_STRENGTH_LONGTEXT, false )
change_float_range(-1., 100.0);


add_integer( ENC_CFG_PREFIX ENC_SCD, -1,
ENC_SCD_TEXT, ENC_SCD_TEXT, true )
change_integer_range(-1, 1 );


add_string( ENC_CFG_PREFIX ENC_PWT, NULL,
ENC_PWT_TEXT, ENC_PWT_TEXT, true )
change_string_list( enc_perceptual_weighting_list, enc_perceptual_weighting_list )


add_float( ENC_CFG_PREFIX ENC_PDIST, -1,
ENC_PDIST_TEXT, ENC_PDIST_LONGTEXT, true )
change_float_range(-1., 100.);


add_integer( ENC_CFG_PREFIX ENC_NOAC, -1,
ENC_NOAC_TEXT, ENC_NOAC_LONGTEXT, true )
change_integer_range(-1, 1 );


add_integer( ENC_CFG_PREFIX ENC_HSLICES, -1,
ENC_HSLICES_TEXT, ENC_HSLICES_LONGTEXT, true )
change_integer_range(-1, INT_MAX );


add_integer( ENC_CFG_PREFIX ENC_VSLICES, -1,
ENC_VSLICES_TEXT, ENC_VSLICES_LONGTEXT, true )
change_integer_range(-1, INT_MAX );


add_string( ENC_CFG_PREFIX ENC_FORCE_PROFILE, NULL,
ENC_FORCE_PROFILE_TEXT, ENC_FORCE_PROFILE_TEXT, true )
change_string_list( enc_profile_list, enc_profile_list_text )

vlc_module_end ()





static int DecodeBlock ( decoder_t *p_dec, block_t *p_block );
static void Flush( decoder_t * );

struct picture_free_t
{
picture_t *p_pic;
decoder_t *p_dec;
};




typedef struct
{



vlc_tick_t i_lastpts;
vlc_tick_t i_frame_pts_delta;
SchroDecoder *p_schro;
SchroVideoFormat *p_format;
} decoder_sys_t;




static int OpenDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
SchroDecoder *p_schro;

if( p_dec->fmt_in.i_codec != VLC_CODEC_DIRAC )
{
return VLC_EGENERIC;
}


p_sys = malloc(sizeof(decoder_sys_t));
if( p_sys == NULL )
return VLC_ENOMEM;



schro_init();


if( !(p_schro = schro_decoder_new()) )
{
free( p_sys );
return VLC_EGENERIC;
}

p_dec->p_sys = p_sys;
p_sys->p_schro = p_schro;
p_sys->p_format = NULL;
p_sys->i_lastpts = VLC_TICK_INVALID;
p_sys->i_frame_pts_delta = 0;


p_dec->fmt_out.i_codec = VLC_CODEC_I420;


p_dec->pf_decode = DecodeBlock;
p_dec->pf_flush = Flush;

return VLC_SUCCESS;
}




static void SetVideoFormat( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;

p_sys->p_format = schro_decoder_get_video_format(p_sys->p_schro);
if( p_sys->p_format == NULL ) return;

p_sys->i_frame_pts_delta = VLC_TICK_FROM_SEC(1)
* p_sys->p_format->frame_rate_denominator
/ p_sys->p_format->frame_rate_numerator;

switch( p_sys->p_format->chroma_format )
{
case SCHRO_CHROMA_420: p_dec->fmt_out.i_codec = VLC_CODEC_I420; break;
case SCHRO_CHROMA_422: p_dec->fmt_out.i_codec = VLC_CODEC_I422; break;
case SCHRO_CHROMA_444: p_dec->fmt_out.i_codec = VLC_CODEC_I444; break;
default:
p_dec->fmt_out.i_codec = 0;
break;
}

p_dec->fmt_out.video.i_visible_width = p_sys->p_format->clean_width;
p_dec->fmt_out.video.i_x_offset = p_sys->p_format->left_offset;
p_dec->fmt_out.video.i_width = p_sys->p_format->width;

p_dec->fmt_out.video.i_visible_height = p_sys->p_format->clean_height;
p_dec->fmt_out.video.i_y_offset = p_sys->p_format->top_offset;
p_dec->fmt_out.video.i_height = p_sys->p_format->height;


p_dec->fmt_out.video.i_sar_num = p_sys->p_format->aspect_ratio_numerator;
p_dec->fmt_out.video.i_sar_den = p_sys->p_format->aspect_ratio_denominator;

p_dec->fmt_out.video.i_frame_rate =
p_sys->p_format->frame_rate_numerator;
p_dec->fmt_out.video.i_frame_rate_base =
p_sys->p_format->frame_rate_denominator;
}






static void SchroFrameFree( SchroFrame *frame, void *priv)
{
struct picture_free_t *p_free = priv;

if( !p_free )
return;

picture_Release( p_free->p_pic );
free(p_free);
(void)frame;
}




static SchroFrame *CreateSchroFrameFromPic( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
SchroFrame *p_schroframe = schro_frame_new();
picture_t *p_pic = NULL;
struct picture_free_t *p_free;

if( !p_schroframe )
return NULL;

if( decoder_UpdateVideoFormat( p_dec ) )
return NULL;
p_pic = decoder_NewPicture( p_dec );

if( !p_pic )
return NULL;

p_schroframe->format = SCHRO_FRAME_FORMAT_U8_420;
if( p_sys->p_format->chroma_format == SCHRO_CHROMA_422 )
{
p_schroframe->format = SCHRO_FRAME_FORMAT_U8_422;
}
else if( p_sys->p_format->chroma_format == SCHRO_CHROMA_444 )
{
p_schroframe->format = SCHRO_FRAME_FORMAT_U8_444;
}

p_schroframe->width = p_sys->p_format->width;
p_schroframe->height = p_sys->p_format->height;

p_free = malloc( sizeof( *p_free ) );
p_free->p_pic = p_pic;
p_free->p_dec = p_dec;
schro_frame_set_free_callback( p_schroframe, SchroFrameFree, p_free );

for( int i=0; i<3; i++ )
{
p_schroframe->components[i].width = p_pic->p[i].i_visible_pitch;
p_schroframe->components[i].stride = p_pic->p[i].i_pitch;
p_schroframe->components[i].height = p_pic->p[i].i_visible_lines;
p_schroframe->components[i].length =
p_pic->p[i].i_pitch * p_pic->p[i].i_lines;
p_schroframe->components[i].data = p_pic->p[i].p_pixels;

if(i!=0)
{
p_schroframe->components[i].v_shift =
SCHRO_FRAME_FORMAT_V_SHIFT( p_schroframe->format );
p_schroframe->components[i].h_shift =
SCHRO_FRAME_FORMAT_H_SHIFT( p_schroframe->format );
}
}

p_pic->b_progressive = !p_sys->p_format->interlaced;
p_pic->b_top_field_first = p_sys->p_format->top_field_first;
p_pic->i_nb_fields = 2;

return p_schroframe;
}




static void SchroBufferFree( SchroBuffer *buf, void *priv )
{
block_t *p_block = priv;

if( !p_block )
return;

block_Release( p_block );
(void)buf;
}




static void CloseDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;

schro_decoder_free( p_sys->p_schro );
free( p_sys );
}




static void Flush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;

schro_decoder_reset( p_sys->p_schro );
p_sys->i_lastpts = VLC_TICK_INVALID;
}







static int DecodeBlock( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;

if( !p_block ) 
return VLCDEC_SUCCESS;
else {



if( p_block->i_flags & (BLOCK_FLAG_CORRUPTED|BLOCK_FLAG_DISCONTINUITY) )
{
Flush( p_dec );
if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release( p_block );
return VLCDEC_SUCCESS;
}
}

SchroBuffer *p_schrobuffer;
p_schrobuffer = schro_buffer_new_with_data( p_block->p_buffer, p_block->i_buffer );
p_schrobuffer->free = SchroBufferFree;
p_schrobuffer->priv = p_block;
if( p_block->i_pts != VLC_TICK_INVALID ) {
vlc_tick_t *p_pts = malloc( sizeof(*p_pts) );
if( p_pts ) {
*p_pts = p_block->i_pts;

p_schrobuffer->tag = schro_tag_new( p_pts, free );
}
}



schro_decoder_autoparse_push( p_sys->p_schro, p_schrobuffer );

}

while( 1 )
{
SchroFrame *p_schroframe;
picture_t *p_pic;
int state = schro_decoder_autoparse_wait( p_sys->p_schro );

switch( state )
{
case SCHRO_DECODER_FIRST_ACCESS_UNIT:
SetVideoFormat( p_dec );
break;

case SCHRO_DECODER_NEED_BITS:
return VLCDEC_SUCCESS;

case SCHRO_DECODER_NEED_FRAME:
p_schroframe = CreateSchroFrameFromPic( p_dec );

if( !p_schroframe )
{
msg_Err( p_dec, "Could not allocate picture for decoder");
return VLCDEC_SUCCESS;
}

schro_decoder_add_output_picture( p_sys->p_schro, p_schroframe);
break;

case SCHRO_DECODER_OK: {
SchroTag *p_tag = schro_decoder_get_picture_tag( p_sys->p_schro );
p_schroframe = schro_decoder_pull( p_sys->p_schro );
if( !p_schroframe || !p_schroframe->priv )
{


if( p_tag ) schro_tag_free( p_tag );
if( p_schroframe ) schro_frame_unref( p_schroframe );
break;
}
p_pic = ((struct picture_free_t*) p_schroframe->priv)->p_pic;
p_schroframe->priv = NULL;

if( p_tag )
{

p_pic->date = *(vlc_tick_t*) p_tag->value;
schro_tag_free( p_tag );
}
else if( p_sys->i_lastpts != VLC_TICK_INVALID )
{





p_pic->date = p_sys->i_lastpts + p_sys->i_frame_pts_delta;
}
p_sys->i_lastpts = p_pic->date;

schro_frame_unref( p_schroframe );
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
}
case SCHRO_DECODER_EOS:

break;

case SCHRO_DECODER_ERROR:
msg_Err( p_dec, "SCHRO_DECODER_ERROR");
return VLCDEC_SUCCESS;
}
}
}




static block_t *Encode( encoder_t *p_enc, picture_t *p_pict );






struct picture_pts_t
{
vlc_tick_t i_pts; 
uint32_t u_pnum; 
bool b_empty; 
};




#define SCHRO_PTS_TLB_SIZE 256
typedef struct
{



SchroEncoder *p_schro;
SchroVideoFormat *p_format;
int started;
bool b_auto_field_coding;

uint32_t i_input_picnum;
timestamp_fifo_t *p_dts_fifo;

block_t *p_chain;

struct picture_pts_t pts_tlb[SCHRO_PTS_TLB_SIZE];
vlc_tick_t i_pts_offset;
vlc_tick_t i_field_duration;

bool b_eos_signalled;
bool b_eos_pulled;
} encoder_sys_t;

static struct
{
unsigned int i_height;
int i_approx_fps;
SchroVideoFormatEnum i_vf;
} schro_format_guess[] = {

{1, 0, SCHRO_VIDEO_FORMAT_CUSTOM},
{120, 15, SCHRO_VIDEO_FORMAT_QSIF},
{144, 12, SCHRO_VIDEO_FORMAT_QCIF},
{240, 15, SCHRO_VIDEO_FORMAT_SIF},
{288, 12, SCHRO_VIDEO_FORMAT_CIF},
{480, 30, SCHRO_VIDEO_FORMAT_SD480I_60},
{480, 15, SCHRO_VIDEO_FORMAT_4SIF},
{576, 12, SCHRO_VIDEO_FORMAT_4CIF},
{576, 25, SCHRO_VIDEO_FORMAT_SD576I_50},
{720, 50, SCHRO_VIDEO_FORMAT_HD720P_50},
{720, 60, SCHRO_VIDEO_FORMAT_HD720P_60},
{1080, 24, SCHRO_VIDEO_FORMAT_DC2K_24},
{1080, 25, SCHRO_VIDEO_FORMAT_HD1080I_50},
{1080, 30, SCHRO_VIDEO_FORMAT_HD1080I_60},
{1080, 50, SCHRO_VIDEO_FORMAT_HD1080P_50},
{1080, 60, SCHRO_VIDEO_FORMAT_HD1080P_60},
{2160, 24, SCHRO_VIDEO_FORMAT_DC4K_24},
{0, 0, 0},
};




static void ResetPTStlb( encoder_t *p_enc )
{
encoder_sys_t *p_sys = p_enc->p_sys;
for( int i = 0; i < SCHRO_PTS_TLB_SIZE; i++ )
{
p_sys->pts_tlb[i].b_empty = true;
}
}





static void StorePicturePTS( encoder_t *p_enc, uint32_t u_pnum, vlc_tick_t i_pts )
{
encoder_sys_t *p_sys = p_enc->p_sys;

for( int i = 0; i<SCHRO_PTS_TLB_SIZE; i++ )
{
if( p_sys->pts_tlb[i].b_empty )
{
p_sys->pts_tlb[i].u_pnum = u_pnum;
p_sys->pts_tlb[i].i_pts = i_pts;
p_sys->pts_tlb[i].b_empty = false;

return;
}
}

msg_Err( p_enc, "Could not store PTS %"PRId64" for frame %u", i_pts, u_pnum );
}




static vlc_tick_t GetPicturePTS( encoder_t *p_enc, uint32_t u_pnum )
{
encoder_sys_t *p_sys = p_enc->p_sys;

for( int i = 0; i < SCHRO_PTS_TLB_SIZE; i++ )
{
if( !p_sys->pts_tlb[i].b_empty &&
p_sys->pts_tlb[i].u_pnum == u_pnum )
{
p_sys->pts_tlb[i].b_empty = true;
return p_sys->pts_tlb[i].i_pts;
}
}

msg_Err( p_enc, "Could not retrieve PTS for picture %u", u_pnum );
return VLC_TICK_INVALID;
}

static inline bool SchroSetEnum( encoder_t *p_enc, int i_list_size, const char *list[],
const char *psz_name, const char *psz_name_text, const char *psz_value)
{
encoder_sys_t *p_sys = p_enc->p_sys;
if( list && psz_name_text && psz_name && psz_value ) {
for( int i = 0; i < i_list_size; ++i ) {
if( strcmp( list[i], psz_value ) )
continue;
schro_encoder_setting_set_double( p_sys->p_schro, psz_name, i );
return true;
}
msg_Err( p_enc, "Invalid %s: %s", psz_name_text, psz_value );
}
return false;
}

static bool SetEncChromaFormat( encoder_t *p_enc, uint32_t i_codec )
{
encoder_sys_t *p_sys = p_enc->p_sys;

switch( i_codec ) {
case VLC_CODEC_I420:
p_enc->fmt_in.i_codec = i_codec;
p_enc->fmt_in.video.i_bits_per_pixel = 12;
p_sys->p_format->chroma_format = SCHRO_CHROMA_420;
break;
case VLC_CODEC_I422:
p_enc->fmt_in.i_codec = i_codec;
p_enc->fmt_in.video.i_bits_per_pixel = 16;
p_sys->p_format->chroma_format = SCHRO_CHROMA_422;
break;
case VLC_CODEC_I444:
p_enc->fmt_in.i_codec = i_codec;
p_enc->fmt_in.video.i_bits_per_pixel = 24;
p_sys->p_format->chroma_format = SCHRO_CHROMA_444;
break;
default:
return false;
}

return true;
}

#define SCHRO_SET_FLOAT(psz_name, pschro_name) f_tmp = var_GetFloat( p_enc, ENC_CFG_PREFIX psz_name ); if( f_tmp >= 0.0 ) schro_encoder_setting_set_double( p_sys->p_schro, pschro_name, f_tmp );




#define SCHRO_SET_INTEGER(psz_name, pschro_name, ignore_val) i_tmp = var_GetInteger( p_enc, ENC_CFG_PREFIX psz_name ); if( i_tmp > ignore_val ) schro_encoder_setting_set_double( p_sys->p_schro, pschro_name, i_tmp );




#define SCHRO_SET_ENUM(list, psz_name, psz_name_text, pschro_name) psz_tmp = var_GetString( p_enc, ENC_CFG_PREFIX psz_name ); if( !psz_tmp ) goto error; else if ( *psz_tmp != '\0' ) { int i_list_size = ARRAY_SIZE(list); if( !SchroSetEnum( p_enc, i_list_size, list, pschro_name, psz_name_text, psz_tmp ) ) { free( psz_tmp ); goto error; } } free( psz_tmp );















static int OpenEncoder( vlc_object_t *p_this )
{
encoder_t *p_enc = (encoder_t *)p_this;
encoder_sys_t *p_sys;
int i_tmp;
float f_tmp;
char *psz_tmp;

if( p_enc->fmt_out.i_codec != VLC_CODEC_DIRAC &&
!p_enc->obj.force )
{
return VLC_EGENERIC;
}

if( !p_enc->fmt_in.video.i_frame_rate || !p_enc->fmt_in.video.i_frame_rate_base ||
!p_enc->fmt_in.video.i_visible_height || !p_enc->fmt_in.video.i_visible_width )
{
msg_Err( p_enc, "Framerate and picture dimensions must be non-zero" );
return VLC_EGENERIC;
}


if( ( p_sys = calloc( 1, sizeof( *p_sys ) ) ) == NULL )
return VLC_ENOMEM;

p_enc->p_sys = p_sys;
p_enc->pf_encode_video = Encode;
p_enc->fmt_out.i_codec = VLC_CODEC_DIRAC;
p_enc->fmt_out.i_cat = VIDEO_ES;

if( ( p_sys->p_dts_fifo = timestamp_FifoNew(32) ) == NULL )
{
CloseEncoder( p_this );
return VLC_ENOMEM;
}

ResetPTStlb( p_enc );


int i = 0;
SchroVideoFormatEnum guessed_video_fmt = SCHRO_VIDEO_FORMAT_CUSTOM;




do
{
if( schro_format_guess[i].i_height > p_enc->fmt_in.video.i_height )
{
guessed_video_fmt = schro_format_guess[i-1].i_vf;
break;
}
if( schro_format_guess[i].i_height != p_enc->fmt_in.video.i_height )
continue;
int src_fps = p_enc->fmt_in.video.i_frame_rate / p_enc->fmt_in.video.i_frame_rate_base;
int delta_fps = abs( schro_format_guess[i].i_approx_fps - src_fps );
if( delta_fps > 2 )
continue;

guessed_video_fmt = schro_format_guess[i].i_vf;
break;
} while( schro_format_guess[++i].i_height );

schro_init();
p_sys->p_schro = schro_encoder_new();
if( !p_sys->p_schro ) {
msg_Err( p_enc, "Failed to initialize libschroedinger encoder" );
return VLC_EGENERIC;
}
schro_encoder_set_packet_assembly( p_sys->p_schro, true );

if( !( p_sys->p_format = schro_encoder_get_video_format( p_sys->p_schro ) ) ) {
msg_Err( p_enc, "Failed to get Schroedigner video format" );
schro_encoder_free( p_sys->p_schro );
return VLC_EGENERIC;
}


schro_video_format_set_std_video_format( p_sys->p_format, guessed_video_fmt );


p_sys->p_format->width = p_enc->fmt_in.video.i_visible_width;
p_sys->p_format->height = p_enc->fmt_in.video.i_visible_height;
p_sys->p_format->frame_rate_numerator = p_enc->fmt_in.video.i_frame_rate;
p_sys->p_format->frame_rate_denominator = p_enc->fmt_in.video.i_frame_rate_base;
unsigned u_asr_num, u_asr_den;
vlc_ureduce( &u_asr_num, &u_asr_den,
p_enc->fmt_in.video.i_sar_num,
p_enc->fmt_in.video.i_sar_den, 0 );
p_sys->p_format->aspect_ratio_numerator = u_asr_num;
p_sys->p_format->aspect_ratio_denominator = u_asr_den;

config_ChainParse( p_enc, ENC_CFG_PREFIX, ppsz_enc_options, p_enc->p_cfg );

SCHRO_SET_ENUM(enc_rate_control_list, ENC_RATE_CONTROL, ENC_RATE_CONTROL_TEXT, "rate_control")

SCHRO_SET_ENUM(enc_gop_structure_list, ENC_GOP_STRUCTURE, ENC_GOP_STRUCTURE_TEXT, "gop_structure")

psz_tmp = var_GetString( p_enc, ENC_CFG_PREFIX ENC_CHROMAFMT );
if( !psz_tmp )
goto error;
else {
uint32_t i_codec;
if( !strcmp( psz_tmp, "420" ) ) {
i_codec = VLC_CODEC_I420;
}
else if( !strcmp( psz_tmp, "422" ) ) {
i_codec = VLC_CODEC_I422;
}
else if( !strcmp( psz_tmp, "444" ) ) {
i_codec = VLC_CODEC_I444;
}
else {
msg_Err( p_enc, "Invalid chroma format: %s", psz_tmp );
free( psz_tmp );
goto error;
}
SetEncChromaFormat( p_enc, i_codec );
}
free( psz_tmp );

SCHRO_SET_FLOAT(ENC_QUALITY, "quality")

SCHRO_SET_FLOAT(ENC_NOISE_THRESHOLD, "noise_threshold")


i_tmp = var_GetInteger( p_enc, ENC_CFG_PREFIX ENC_BITRATE );
if( i_tmp > -1 )
schro_encoder_setting_set_double( p_sys->p_schro, "bitrate", i_tmp * 1000 );
else
schro_encoder_setting_set_double( p_sys->p_schro, "bitrate", p_enc->fmt_out.i_bitrate );

p_enc->fmt_out.i_bitrate = schro_encoder_setting_get_double( p_sys->p_schro, "bitrate" );

i_tmp = var_GetInteger( p_enc, ENC_CFG_PREFIX ENC_MIN_BITRATE );
if( i_tmp > -1 )
schro_encoder_setting_set_double( p_sys->p_schro, "min_bitrate", i_tmp * 1000 );

i_tmp = var_GetInteger( p_enc, ENC_CFG_PREFIX ENC_MAX_BITRATE );
if( i_tmp > -1 )
schro_encoder_setting_set_double( p_sys->p_schro, "max_bitrate", i_tmp * 1000 );

SCHRO_SET_INTEGER(ENC_AU_DISTANCE, "au_distance", -1)

SCHRO_SET_ENUM(enc_filtering_list, ENC_PREFILTER, ENC_PREFILTER_TEXT, "filtering")

SCHRO_SET_FLOAT(ENC_PREFILTER_STRENGTH, "filter_value")


psz_tmp = var_GetString( p_enc, ENC_CFG_PREFIX ENC_CODINGMODE );
if( !psz_tmp )
goto error;
else if( !strcmp( psz_tmp, "auto" ) ) {
p_sys->b_auto_field_coding = true;
}
else if( !strcmp( psz_tmp, "progressive" ) ) {
p_sys->b_auto_field_coding = false;
schro_encoder_setting_set_double( p_sys->p_schro, "interlaced_coding", false);
}
else if( !strcmp( psz_tmp, "field" ) ) {
p_sys->b_auto_field_coding = false;
schro_encoder_setting_set_double( p_sys->p_schro, "interlaced_coding", true);
}
else {
msg_Err( p_enc, "Invalid codingmode: %s", psz_tmp );
free( psz_tmp );
goto error;
}
free( psz_tmp );

SCHRO_SET_ENUM(enc_block_size_list, ENC_MCBLK_SIZE, ENC_MCBLK_SIZE_TEXT, "motion_block_size")

SCHRO_SET_ENUM(enc_block_overlap_list, ENC_MCBLK_OVERLAP, ENC_MCBLK_OVERLAP_TEXT, "motion_block_overlap")

psz_tmp = var_GetString( p_enc, ENC_CFG_PREFIX ENC_MVPREC );
if( !psz_tmp )
goto error;
else if( *psz_tmp != '\0') {
if( !strcmp( psz_tmp, "1" ) ) {
schro_encoder_setting_set_double( p_sys->p_schro, "mv_precision", 0 );
}
else if( !strcmp( psz_tmp, "1/2" ) ) {
schro_encoder_setting_set_double( p_sys->p_schro, "mv_precision", 1 );
}
else if( !strcmp( psz_tmp, "1/4" ) ) {
schro_encoder_setting_set_double( p_sys->p_schro, "mv_precision", 2 );
}
else if( !strcmp( psz_tmp, "1/8" ) ) {
schro_encoder_setting_set_double( p_sys->p_schro, "mv_precision", 3 );
}
else {
msg_Err( p_enc, "Invalid mv_precision: %s", psz_tmp );
free( psz_tmp );
goto error;
}
}
free( psz_tmp );

SCHRO_SET_INTEGER(ENC_ME_COMBINED, "enable_chroma_me", -1)

SCHRO_SET_ENUM(enc_wavelet_list, ENC_DWTINTRA, ENC_DWTINTRA_TEXT, "intra_wavelet")

SCHRO_SET_ENUM(enc_wavelet_list, ENC_DWTINTER, ENC_DWTINTER_TEXT, "inter_wavelet")

SCHRO_SET_INTEGER(ENC_DWTDEPTH, "transform_depth", -1)

SCHRO_SET_INTEGER(ENC_MULTIQUANT, "enable_multiquant", -1)

SCHRO_SET_INTEGER(ENC_NOAC, "enable_noarith", -1)

SCHRO_SET_ENUM(enc_perceptual_weighting_list, ENC_PWT, ENC_PWT_TEXT, "perceptual_weighting")

SCHRO_SET_FLOAT(ENC_PDIST, "perceptual_distance")

SCHRO_SET_INTEGER(ENC_HSLICES, "horiz_slices", -1)

SCHRO_SET_INTEGER(ENC_VSLICES, "vert_slices", -1)

SCHRO_SET_ENUM(enc_codeblock_size_list, ENC_SCBLK_SIZE, ENC_SCBLK_SIZE_TEXT, "codeblock_size")

SCHRO_SET_INTEGER(ENC_ME_HIERARCHICAL, "enable_hierarchical_estimation", -1)

SCHRO_SET_INTEGER(ENC_ME_DOWNSAMPLE_LEVELS, "downsample_levels", 1)

SCHRO_SET_INTEGER(ENC_ME_GLOBAL_MOTION, "enable_global_motion", -1)

SCHRO_SET_INTEGER(ENC_ME_PHASECORR, "enable_phasecorr_estimation", -1)

SCHRO_SET_INTEGER(ENC_SCD, "enable_scene_change_detection", -1)

SCHRO_SET_ENUM(enc_profile_list, ENC_FORCE_PROFILE, ENC_FORCE_PROFILE_TEXT, "force_profile")

p_sys->started = 0;

return VLC_SUCCESS;
error:
CloseEncoder( p_this );
return VLC_EGENERIC;
}


struct enc_picture_free_t
{
picture_t *p_pic;
encoder_t *p_enc;
};






static void EncSchroFrameFree( SchroFrame *frame, void *priv )
{
struct enc_picture_free_t *p_free = priv;

if( !p_free )
return;

picture_Release( p_free->p_pic );
free( p_free );
(void)frame;
}




static SchroFrame *CreateSchroFrameFromInputPic( encoder_t *p_enc, picture_t *p_pic )
{
encoder_sys_t *p_sys = p_enc->p_sys;
SchroFrame *p_schroframe = schro_frame_new();
struct enc_picture_free_t *p_free;

if( !p_schroframe )
return NULL;

if( !p_pic )
return NULL;

p_schroframe->format = SCHRO_FRAME_FORMAT_U8_420;
if( p_sys->p_format->chroma_format == SCHRO_CHROMA_422 )
{
p_schroframe->format = SCHRO_FRAME_FORMAT_U8_422;
}
else if( p_sys->p_format->chroma_format == SCHRO_CHROMA_444 )
{
p_schroframe->format = SCHRO_FRAME_FORMAT_U8_444;
}

p_schroframe->width = p_sys->p_format->width;
p_schroframe->height = p_sys->p_format->height;

p_free = malloc( sizeof( *p_free ) );
if( unlikely( p_free == NULL ) ) {
schro_frame_unref( p_schroframe );
return NULL;
}

p_free->p_pic = p_pic;
p_free->p_enc = p_enc;
schro_frame_set_free_callback( p_schroframe, EncSchroFrameFree, p_free );

for( int i=0; i<3; i++ )
{
p_schroframe->components[i].width = p_pic->p[i].i_visible_pitch;
p_schroframe->components[i].stride = p_pic->p[i].i_pitch;
p_schroframe->components[i].height = p_pic->p[i].i_visible_lines;
p_schroframe->components[i].length =
p_pic->p[i].i_pitch * p_pic->p[i].i_lines;
p_schroframe->components[i].data = p_pic->p[i].p_pixels;

if( i!=0 )
{
p_schroframe->components[i].v_shift =
SCHRO_FRAME_FORMAT_V_SHIFT( p_schroframe->format );
p_schroframe->components[i].h_shift =
SCHRO_FRAME_FORMAT_H_SHIFT( p_schroframe->format );
}
}

return p_schroframe;
}


static int ReadDiracPictureNumber( uint32_t *p_picnum, block_t *p_block )
{
uint32_t u_pos = 4;

while( u_pos + 13 < p_block->i_buffer )
{

if( p_block->p_buffer[u_pos] & 0x08 )
{
*p_picnum = GetDWBE( p_block->p_buffer + u_pos + 9 );
return 1;
}

uint32_t u_npo = GetDWBE( p_block->p_buffer + u_pos + 1 );
assert( u_npo <= UINT32_MAX - u_pos );
if( u_npo == 0 )
u_npo = 13;
u_pos += u_npo;
}
return 0;
}


static block_t *Encode( encoder_t *p_enc, picture_t *p_pic )
{
encoder_sys_t *p_sys = p_enc->p_sys;
block_t *p_block, *p_output_chain = NULL;
SchroFrame *p_frame;
bool b_go = true;

if( !p_pic ) {
if( !p_sys->started || p_sys->b_eos_pulled )
return NULL;

if( !p_sys->b_eos_signalled ) {
p_sys->b_eos_signalled = 1;
schro_encoder_end_of_stream( p_sys->p_schro );
}
} else {



p_sys->p_format->interlaced = !p_pic->b_progressive;
p_sys->p_format->top_field_first = p_pic->b_top_field_first;

if( p_sys->b_auto_field_coding )
schro_encoder_setting_set_double( p_sys->p_schro, "interlaced_coding", !p_pic->b_progressive );
}

if( !p_sys->started ) {
date_t date;

if( p_pic->format.i_chroma != p_enc->fmt_in.i_codec ) {
char chroma_in[5], chroma_out[5];
vlc_fourcc_to_char( p_pic->format.i_chroma, chroma_in );
chroma_in[4] = '\0';
chroma_out[4] = '\0';
vlc_fourcc_to_char( p_enc->fmt_in.i_codec, chroma_out );
msg_Warn( p_enc, "Resetting chroma from %s to %s", chroma_out, chroma_in );
if( !SetEncChromaFormat( p_enc, p_pic->format.i_chroma ) ) {
msg_Err( p_enc, "Could not reset chroma format to %s", chroma_in );
return NULL;
}
}

date_Init( &date, p_enc->fmt_in.video.i_frame_rate * 2, p_enc->fmt_in.video.i_frame_rate_base );
date_Set( &date, VLC_TICK_0 );



date_Increment( &date, 2 );
p_sys->i_pts_offset = date_Get( &date ) - VLC_TICK_0;
if( schro_encoder_setting_get_double( p_sys->p_schro, "interlaced_coding" ) > 0.0 ) {
date_Set( &date, VLC_TICK_0 );
date_Increment( &date, 1 );
p_sys->i_field_duration = date_Get( &date ) - VLC_TICK_0;
}

schro_video_format_set_std_signal_range( p_sys->p_format, SCHRO_SIGNAL_RANGE_8BIT_VIDEO );
schro_encoder_set_video_format( p_sys->p_schro, p_sys->p_format );
schro_encoder_start( p_sys->p_schro );
p_sys->started = 1;
}

if( !p_sys->b_eos_signalled ) {



picture_Hold( p_pic );

p_frame = CreateSchroFrameFromInputPic( p_enc, p_pic );
if( !p_frame )
return NULL;
schro_encoder_push_frame( p_sys->p_schro, p_frame );




StorePicturePTS( p_enc, p_sys->i_input_picnum, p_pic->date );
p_sys->i_input_picnum++;



timestamp_FifoPut( p_sys->p_dts_fifo, p_pic->date - p_sys->i_pts_offset );




if( schro_encoder_setting_get_double( p_sys->p_schro, "interlaced_coding" ) > 0.0 ) {
StorePicturePTS( p_enc, p_sys->i_input_picnum, p_pic->date + p_sys->i_field_duration );
p_sys->i_input_picnum++;

timestamp_FifoPut( p_sys->p_dts_fifo, p_pic->date - p_sys->i_pts_offset + p_sys->i_field_duration );
}
}

do
{
SchroStateEnum state;
state = schro_encoder_wait( p_sys->p_schro );
switch( state )
{
case SCHRO_STATE_NEED_FRAME:
b_go = false;
break;
case SCHRO_STATE_AGAIN:
break;
case SCHRO_STATE_END_OF_STREAM:
p_sys->b_eos_pulled = 1;
b_go = false;
break;
case SCHRO_STATE_HAVE_BUFFER:
{
SchroBuffer *p_enc_buf;
uint32_t u_pic_num;
int i_presentation_frame;
p_enc_buf = schro_encoder_pull( p_sys->p_schro, &i_presentation_frame );
p_block = block_Alloc( p_enc_buf->length );
if( !p_block )
return NULL;

memcpy( p_block->p_buffer, p_enc_buf->data, p_enc_buf->length );
schro_buffer_unref( p_enc_buf );


if( 0 == p_block->p_buffer[4] )
{
p_block->i_flags |= BLOCK_FLAG_TYPE_I;

if( !p_enc->fmt_out.p_extra ) {
const uint8_t eos[] = { 'B','B','C','D',0x10,0,0,0,13,0,0,0,0 };
uint32_t len = GetDWBE( p_block->p_buffer + 5 );







if( len > UINT32_MAX - sizeof( eos ) )
return NULL;

p_enc->fmt_out.p_extra = malloc( len + sizeof( eos ) );
if( !p_enc->fmt_out.p_extra )
return NULL;
memcpy( p_enc->fmt_out.p_extra, p_block->p_buffer, len );
memcpy( (uint8_t*)p_enc->fmt_out.p_extra + len, eos, sizeof( eos ) );
SetDWBE( (uint8_t*)p_enc->fmt_out.p_extra + len + sizeof(eos) - 4, len );
p_enc->fmt_out.i_extra = len + sizeof( eos );
}
}

if( ReadDiracPictureNumber( &u_pic_num, p_block ) ) {
p_block->i_dts = timestamp_FifoGet( p_sys->p_dts_fifo );
p_block->i_pts = GetPicturePTS( p_enc, u_pic_num );
block_ChainAppend( &p_output_chain, p_block );
} else {

block_ChainAppend( &p_output_chain, p_block );
}
break;
}
default:
break;
}
} while( b_go );

return p_output_chain;
}




static void CloseEncoder( vlc_object_t *p_this )
{
encoder_t *p_enc = (encoder_t *)p_this;
encoder_sys_t *p_sys = p_enc->p_sys;


if( p_sys->p_schro )
schro_encoder_free( p_sys->p_schro );

free( p_sys->p_format );

if( p_sys->p_dts_fifo )
timestamp_FifoRelease( p_sys->p_dts_fifo );

block_ChainRelease( p_sys->p_chain );

free( p_sys );
}
