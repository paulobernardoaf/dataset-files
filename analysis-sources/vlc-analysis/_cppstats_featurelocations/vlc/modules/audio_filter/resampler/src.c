



























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_filter.h>
#include <samplerate.h>
#include <math.h>

#define SRC_CONV_TYPE_TEXT N_("Sample rate converter type")
#define SRC_CONV_TYPE_LONGTEXT N_( "Different resampling algorithms are supported. " "The best one is slower, while the fast one exhibits low quality.")


static const int conv_type_values[] = {
SRC_SINC_BEST_QUALITY, SRC_SINC_MEDIUM_QUALITY, SRC_SINC_FASTEST,
SRC_ZERO_ORDER_HOLD, SRC_LINEAR,
};
static const char *const conv_type_texts[] = {
N_("Sinc function (best quality)"), N_("Sinc function (medium quality)"),
N_("Sinc function (fast)"), N_("Zero Order Hold (fastest)"), N_("Linear (fastest)"),
};

static int Open (vlc_object_t *);
static int OpenResampler (vlc_object_t *);
static void Close (vlc_object_t *);

vlc_module_begin ()
set_shortname (N_("SRC resampler"))
set_description (N_("Secret Rabbit Code (libsamplerate) resampler") )
set_category (CAT_AUDIO)
set_subcategory (SUBCAT_AUDIO_RESAMPLER)
add_integer ("src-converter-type", SRC_SINC_FASTEST,
SRC_CONV_TYPE_TEXT, SRC_CONV_TYPE_LONGTEXT, true)
change_integer_list (conv_type_values, conv_type_texts)
set_capability ("audio converter", 50)
set_callbacks (Open, Close)

add_submodule ()
set_capability ("audio resampler", 50)
set_callbacks (OpenResampler, Close)
vlc_module_end ()

static block_t *Resample (filter_t *, block_t *);

static int Open (vlc_object_t *obj)
{
filter_t *filter = (filter_t *)obj;


if (filter->fmt_in.audio.i_rate == filter->fmt_out.audio.i_rate)
return VLC_EGENERIC;
return OpenResampler (obj);
}

static int OpenResampler (vlc_object_t *obj)
{
filter_t *filter = (filter_t *)obj;


if (filter->fmt_in.audio.i_format != VLC_CODEC_FL32
|| filter->fmt_out.audio.i_format != VLC_CODEC_FL32

|| filter->fmt_in.audio.i_channels != filter->fmt_out.audio.i_channels )
return VLC_EGENERIC;

int type = var_InheritInteger (obj, "src-converter-type");
int err;

SRC_STATE *s = src_new (type, filter->fmt_in.audio.i_channels, &err);
if (s == NULL)
{
msg_Err (obj, "cannot initialize resampler: %s", src_strerror (err));
return VLC_EGENERIC;
}

filter->p_sys = s;
filter->pf_audio_filter = Resample;
return VLC_SUCCESS;
}

static void Close (vlc_object_t *obj)
{
filter_t *filter = (filter_t *)obj;
SRC_STATE *s = (SRC_STATE *)filter->p_sys;

src_delete (s);
}

static block_t *Resample (filter_t *filter, block_t *in)
{
block_t *out = NULL;
const size_t framesize = filter->fmt_out.audio.i_bytes_per_frame;

SRC_STATE *s = (SRC_STATE *)filter->p_sys;
SRC_DATA src;

src.src_ratio = (double)filter->fmt_out.audio.i_rate
/ (double)filter->fmt_in.audio.i_rate;

int err = src_set_ratio (s, src.src_ratio);
if (err != 0)
{
msg_Err (filter, "cannot update resampling ratio: %s",
src_strerror (err));
goto error;
}

src.input_frames = in->i_nb_samples;
src.output_frames = ceil (src.src_ratio * src.input_frames);
src.end_of_input = 0;

out = block_Alloc (src.output_frames * framesize);
if (unlikely(out == NULL))
goto error;

src.data_in = (float *)in->p_buffer;
src.data_out = (float *)out->p_buffer;

err = src_process (s, &src);
if (err != 0)
{
msg_Err (filter, "cannot resample: %s", src_strerror (err));
block_Release (out);
out = NULL;
goto error;
}

if (src.input_frames_used < src.input_frames)
msg_Err (filter, "lost %ld of %ld input frames",
src.input_frames - src.input_frames_used, src.input_frames);

out->i_buffer = src.output_frames_gen * framesize;
out->i_nb_samples = src.output_frames_gen;
out->i_pts = in->i_pts;
out->i_length = vlc_tick_from_samples(src.output_frames_gen, filter->fmt_out.audio.i_rate);
error:
block_Release (in);
return out;
}
