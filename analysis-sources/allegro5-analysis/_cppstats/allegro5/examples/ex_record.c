#define ALLEGRO_UNSTABLE
#include "allegro5/allegro.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_acodec.h"
#include "common.c"
#define WANT_8_BIT_DEPTH 
#if defined(WANT_8_BIT_DEPTH)
const ALLEGRO_AUDIO_DEPTH audio_depth = ALLEGRO_AUDIO_DEPTH_UINT8;
typedef uint8_t* audio_buffer_t;
const uint8_t sample_center = 128;
const int8_t min_sample_val = 0x80;
const int8_t max_sample_val = 0x7f;
const int sample_range = 0xff;
const int sample_size = 1;
#else
const ALLEGRO_AUDIO_DEPTH audio_depth = ALLEGRO_AUDIO_DEPTH_INT16;
typedef int16_t* audio_buffer_t;
const int16_t sample_center = 0;
const int16_t min_sample_val = 0x8000;
const int16_t max_sample_val = 0x7fff;
const int sample_range = 0xffff;
const int sample_size = 2;
#endif
const unsigned int samples_per_fragment = 320 * 4;
const unsigned int frequency = 22050;
const unsigned int max_seconds_to_record = 60 * 5;
const unsigned int playback_fragment_count = 4;
const unsigned int playback_samples_per_fragment = 4096;
int main(int argc, char **argv)
{
ALLEGRO_AUDIO_RECORDER *r;
ALLEGRO_AUDIO_STREAM *s;
ALLEGRO_EVENT_QUEUE *q;
ALLEGRO_DISPLAY *d;
ALLEGRO_FILE *fp = NULL;
ALLEGRO_PATH *tmp_path = NULL;
int prev = 0;
bool is_recording = false;
int n = 0; 
(void) argc;
(void) argv;
if (!al_init()) {
abort_example("Could not init Allegro.\n");
}
if (!al_init_primitives_addon()) {
abort_example("Unable to initialize primitives addon");
}
if (!al_install_keyboard()) {
abort_example("Unable to install keyboard");
}
if (!al_install_audio()) {
abort_example("Unable to initialize audio addon");
}
if (!al_init_acodec_addon()) {
abort_example("Unable to initialize acodec addon");
}
r = al_create_audio_recorder(1000, samples_per_fragment, frequency,
audio_depth, ALLEGRO_CHANNEL_CONF_1);
if (!r) {
abort_example("Unable to create audio recorder");
}
s = al_create_audio_stream(playback_fragment_count,
playback_samples_per_fragment, frequency, audio_depth,
ALLEGRO_CHANNEL_CONF_1); 
if (!s) {
abort_example("Unable to create audio stream");
}
al_reserve_samples(0);
al_set_audio_stream_playing(s, false);
al_attach_audio_stream_to_mixer(s, al_get_default_mixer());
q = al_create_event_queue();
al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
d = al_create_display(320, 256);
if (!d) {
abort_example("Error creating display\n");
}
al_set_window_title(d, "SPACE to record. P to playback.");
al_register_event_source(q, al_get_audio_recorder_event_source(r));
al_register_event_source(q, al_get_audio_stream_event_source(s));
al_register_event_source(q, al_get_display_event_source(d));
al_register_event_source(q, al_get_keyboard_event_source());
al_start_audio_recorder(r);
while (true) {
ALLEGRO_EVENT e;
al_wait_for_event(q, &e);
if (e.type == ALLEGRO_EVENT_AUDIO_RECORDER_FRAGMENT) {
ALLEGRO_AUDIO_RECORDER_EVENT *re = al_get_audio_recorder_event(&e);
audio_buffer_t input = (audio_buffer_t) re->buffer;
int sample_count = re->samples; 
const int R = sample_count / 320;
int i, gain = 0;
for (i = 0; i < sample_count; ++i) {
if (gain < abs(input[i] - sample_center))
gain = abs(input[i] - sample_center);
}
al_clear_to_color(al_map_rgb(0,0,0));
if (is_recording) {
if (fp && n < frequency / (float) samples_per_fragment * 
max_seconds_to_record) {
al_fwrite(fp, input, sample_count * sample_size);
++n;
}
for (i = 0; i < 320; ++i) {
int j, c = 0;
for (j = i * R; j < i * R + R && j < sample_count; ++j) {
c += input[j] - sample_center;
}
c /= R;
al_draw_line(i - 1, 128 + ((prev - min_sample_val) /
(float) sample_range) * 256 - 128, i, 128 +
((c - min_sample_val) / (float) sample_range) * 256 - 128,
al_map_rgb(255,255,255), 1.2);
prev = c;
}
}
al_draw_filled_rectangle((gain / (float) max_sample_val) * 320, 251,
0, 256, al_map_rgba(0, 255, 0, 128));
al_flip_display();
}
else if (e.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT) {
if (fp) {
audio_buffer_t output = al_get_audio_stream_fragment(s);
if (output) {
const size_t bytes_to_read =
playback_samples_per_fragment * sample_size;
size_t bytes_read = 0, i;
do {
bytes_read += al_fread(fp, (uint8_t *)output + bytes_read,
bytes_to_read - bytes_read); 
} while (bytes_read < bytes_to_read && !al_feof(fp) &&
!al_ferror(fp));
for (i = bytes_read / sample_size;
i < bytes_to_read / sample_size; ++i) {
output[i] = sample_center;
}
al_set_audio_stream_fragment(s, output);
if (al_ferror(fp) || al_feof(fp)) {
al_drain_audio_stream(s);
al_fclose(fp);
fp = NULL;
}
}
}
} 
else if (e.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
break;
}
else if (e.type == ALLEGRO_EVENT_KEY_CHAR) {
if (e.keyboard.unichar == 27) {
break;
}
else if (e.keyboard.unichar == ' ') {
if (!is_recording) {
is_recording = true;
if (al_get_audio_stream_playing(s)) {
al_drain_audio_stream(s);
}
if (!tmp_path) {
fp = al_make_temp_file("alrecXXX.raw", &tmp_path);
}
else {
if (fp) al_fclose(fp);
fp = al_fopen(al_path_cstr(tmp_path, '/'), "w");
}
n = 0;
}
else {
is_recording = false;
if (fp) {
al_fclose(fp);
fp = NULL;
}
}
}
else if (e.keyboard.unichar == 'p') {
if (!is_recording) {
if (tmp_path) {
fp = al_fopen(al_path_cstr(tmp_path, '/'), "r");
if (fp) {
al_set_audio_stream_playing(s, true);
}
}
}
}
}
}
al_destroy_audio_recorder(r);
al_destroy_audio_stream(s);
if (fp)
al_fclose(fp);
if (tmp_path) {
al_remove_filename(al_path_cstr(tmp_path, '/'));
al_destroy_path(tmp_path);
}
return 0;
}
