



#include <string.h>
#include <math.h>

#include "test.h"

static void test_equalizer (const char ** argv, int argc)
{
libvlc_instance_t *vlc;
libvlc_media_player_t *mp;
libvlc_equalizer_t *equalizer;
unsigned u_presets, u_bands;

test_log ("Testing equalizer\n");

vlc = libvlc_new (argc, argv);
assert (NULL != vlc);

mp = libvlc_media_player_new (vlc);
assert (NULL != mp);



test_log ("Testing equalizer presets\n");

u_presets = libvlc_audio_equalizer_get_preset_count();
assert (u_presets > 0);

for (unsigned i = 0; i < u_presets; i++)
{
assert (NULL != libvlc_audio_equalizer_get_preset_name(i));

equalizer = libvlc_audio_equalizer_new_from_preset(i);
assert (equalizer);

libvlc_audio_equalizer_release(equalizer);
}



assert (NULL == libvlc_audio_equalizer_get_preset_name(u_presets));
assert (NULL == libvlc_audio_equalizer_new_from_preset(u_presets));



test_log ("Testing equalizer bands\n");

u_bands = libvlc_audio_equalizer_get_band_count();
assert (u_bands > 0);

for (unsigned i = 0; i < u_bands; i++)
assert (-1.0f != libvlc_audio_equalizer_get_band_frequency(i));



assert (-1.0f == libvlc_audio_equalizer_get_band_frequency(u_bands));



test_log ("Testing equalizer initialisation\n");

equalizer = libvlc_audio_equalizer_new();
assert (equalizer);

assert (libvlc_audio_equalizer_get_preamp(equalizer) == 0.0f);

for (unsigned i = 0; i < u_bands; i++)
assert (libvlc_audio_equalizer_get_amp_at_index(equalizer, i) == 0.0f);



test_log ("Testing equalizer preamp\n");

assert (0 == libvlc_audio_equalizer_set_preamp (equalizer, 19.9f));
assert (libvlc_audio_equalizer_get_preamp(equalizer) == 19.9f);

assert (0 == libvlc_audio_equalizer_set_preamp (equalizer, 20.0f));
assert (libvlc_audio_equalizer_get_preamp(equalizer) == 20.0f);

assert (0 == libvlc_audio_equalizer_set_preamp (equalizer, 20.1f));
assert (libvlc_audio_equalizer_get_preamp(equalizer) == 20.0f);

assert (0 == libvlc_audio_equalizer_set_preamp (equalizer, -19.9f));
assert (libvlc_audio_equalizer_get_preamp(equalizer) == -19.9f);

assert (0 == libvlc_audio_equalizer_set_preamp (equalizer, -20.0f));
assert (libvlc_audio_equalizer_get_preamp(equalizer) == -20.0f);

assert (0 == libvlc_audio_equalizer_set_preamp (equalizer, -20.1f));
assert (libvlc_audio_equalizer_get_preamp(equalizer) == -20.0f);



test_log ("Testing equalizer amp at index\n");

for (unsigned i = 0; i < u_bands; i++)
{
assert (0 == libvlc_audio_equalizer_set_amp_at_index (equalizer, 19.9f, i));
assert (libvlc_audio_equalizer_get_amp_at_index (equalizer, i) == 19.9f);

assert (0 == libvlc_audio_equalizer_set_amp_at_index (equalizer, 20.0f, i));
assert (libvlc_audio_equalizer_get_amp_at_index (equalizer, i) == 20.0f);

assert (0 == libvlc_audio_equalizer_set_amp_at_index (equalizer, 20.1f, i));
assert (libvlc_audio_equalizer_get_amp_at_index (equalizer, i) == 20.0f);

assert (0 == libvlc_audio_equalizer_set_amp_at_index (equalizer, -19.9f, i));
assert (libvlc_audio_equalizer_get_amp_at_index (equalizer, i) == -19.9f);

assert (0 == libvlc_audio_equalizer_set_amp_at_index (equalizer, -20.0f, i));
assert (libvlc_audio_equalizer_get_amp_at_index (equalizer, i) == -20.0f);

assert (0 == libvlc_audio_equalizer_set_amp_at_index (equalizer, -20.1f, i));
assert (libvlc_audio_equalizer_get_amp_at_index (equalizer, i) == -20.0f);
}



assert (isnan(libvlc_audio_equalizer_get_amp_at_index (equalizer, u_bands)));
assert (-1 == libvlc_audio_equalizer_set_amp_at_index (equalizer, 19.9f, u_bands));



test_log ("Testing release NULL\n");

libvlc_audio_equalizer_release (NULL);



test_log ("Testing set equalizer\n");

assert (0 == libvlc_media_player_set_equalizer(mp, NULL));
assert (0 == libvlc_media_player_set_equalizer(mp, equalizer));



test_log ("Testing equalizer-bands string limit\n");

for (unsigned i = 0; i < u_bands; i++)
assert (0 == libvlc_audio_equalizer_set_amp_at_index (equalizer, -19.1234567f, i));

assert (0 == libvlc_media_player_set_equalizer(mp, equalizer));



test_log ("Testing equalizer cleanup\n");

libvlc_audio_equalizer_release (equalizer);

test_log ("Finished testing equalizer\n");

libvlc_media_player_release (mp);

libvlc_release (vlc);
}

int main (void)
{
test_init();

test_equalizer (test_defaults_args, test_defaults_nargs);

return 0;
}
