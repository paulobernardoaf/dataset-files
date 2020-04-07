






















#include <vlc_charset.h>
#include <vlc_strings.h>

#if defined(__cplusplus)
extern "C" {
#endif

input_attachment_t* ParseFlacPicture( const uint8_t *p_data, size_t i_data,
int i_attachments, int *i_cover_score, int *i_cover_idx );

void vorbis_ParseComment( es_format_t *p_fmt, vlc_meta_t **pp_meta,
const uint8_t *p_data, size_t i_data,
int *i_attachments, input_attachment_t ***attachments,
int *i_cover_score, int *i_cover_idx,
int *i_seekpoint, seekpoint_t ***ppp_seekpoint,
float (* ppf_replay_gain)[AUDIO_REPLAY_GAIN_MAX],
float (* ppf_replay_peak)[AUDIO_REPLAY_GAIN_MAX] );

static const struct {
const char *psz_tag;
const char *psz_i18n;
} Katei18nCategories[] = {

{ "CC", N_("Closed captions") },
{ "SUB", N_("Subtitles") },
{ "TAD", N_("Textual audio descriptions") },
{ "KTV", N_("Karaoke") },
{ "TIK", N_("Ticker text") },
{ "AR", N_("Active regions") },
{ "NB", N_("Semantic annotations") },
{ "META", N_("Metadata") },
{ "TRX", N_("Transcript") },
{ "LRC", N_("Lyrics") },
{ "LIN", N_("Linguistic markup") },
{ "CUE", N_("Cue points") },


{ "subtitles", N_("Subtitles") },
{ "spu-subtitles", N_("Subtitles (images)") },
{ "lyrics", N_("Lyrics") },


{ "K-SPU", N_("Subtitles (images)") },
{ "K-SLD-T", N_("Slides (text)") },
{ "K-SLD-I", N_("Slides (images)") },
};

const char *FindKateCategoryName( const char *psz_tag );

#if defined(__cplusplus)
}
#endif

