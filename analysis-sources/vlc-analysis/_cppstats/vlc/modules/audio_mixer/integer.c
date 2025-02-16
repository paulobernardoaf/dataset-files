#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <math.h>
#include <limits.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_aout_volume.h>
static int Activate (vlc_object_t *);
vlc_module_begin ()
set_category (CAT_AUDIO)
set_subcategory (SUBCAT_AUDIO_MISC)
set_description (N_("Integer audio volume"))
set_capability ("audio volume", 9)
set_callback(Activate)
vlc_module_end ()
static void FilterS32N (audio_volume_t *vol, block_t *block, float volume)
{
int32_t *p = (int32_t *)block->p_buffer;
int_fast32_t mult = lroundf (volume * 0x1.p24f);
if (mult == (1 << 24))
return;
for (size_t n = block->i_buffer / sizeof (*p); n > 0; n--)
{
int_fast64_t s = (*p * (int_fast64_t)mult) >> INT64_C(24);
if (s > INT32_MAX)
s = INT32_MAX;
else
if (s < INT32_MIN)
s = INT32_MIN;
*(p++) = s;
}
(void) vol;
}
static void FilterS16N (audio_volume_t *vol, block_t *block, float volume)
{
int16_t *p = (int16_t *)block->p_buffer;
int_fast16_t mult = lroundf (volume * 0x1.p8f);
if (mult == (1 << 8))
return;
for (size_t n = block->i_buffer / sizeof (*p); n > 0; n--)
{
int_fast32_t s = (*p * (int_fast32_t)mult) >> 8;
if (s > INT16_MAX)
s = INT16_MAX;
else
if (s < INT16_MIN)
s = INT16_MIN;
*(p++) = s;
}
(void) vol;
}
static void FilterU8 (audio_volume_t *vol, block_t *block, float volume)
{
uint8_t *p = (uint8_t *)block->p_buffer;
int_fast16_t mult = lroundf (volume * 0x1.p8f);
if (mult == (1 << 8))
return;
for (size_t n = block->i_buffer / sizeof (*p); n > 0; n--)
{
int_fast32_t s = (((int_fast8_t)(*p - 128)) * (int_fast32_t)mult) >> 8;
if (s > INT8_MAX)
s = INT8_MAX;
else
if (s < INT8_MIN)
s = INT8_MIN;
*(p++) = s + 128;
}
(void) vol;
}
static int Activate (vlc_object_t *obj)
{
audio_volume_t *vol = (audio_volume_t *)obj;
switch (vol->format)
{
case VLC_CODEC_S32N:
vol->amplify = FilterS32N;
break;
case VLC_CODEC_S16N:
vol->amplify = FilterS16N;
break;
case VLC_CODEC_U8:
vol->amplify = FilterU8;
break;
default:
return -1;
}
return 0;
}
