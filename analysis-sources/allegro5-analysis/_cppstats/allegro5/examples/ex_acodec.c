#define ALLEGRO_UNSTABLE
#include <stdio.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"
#include "common.c"
int main(int argc, char **argv)
{
ALLEGRO_VOICE *voice;
ALLEGRO_MIXER *mixer;
ALLEGRO_SAMPLE_INSTANCE *sample;
int i;
char const **filenames;
int n;
if (argc < 2) {
n = 1;
filenames = malloc(sizeof *filenames);
filenames[0] = "data/welcome.wav";
}
else {
n = argc - 1;
filenames = malloc(sizeof *filenames * n);
for (i = 1; i < argc; ++i) {
filenames[i - 1] = argv[i];
}
}
if (!al_init()) {
abort_example("Could not init Allegro.\n");
}
open_log();
al_init_acodec_addon();
if (!al_install_audio()) {
abort_example("Could not init sound!\n");
}
voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16,
ALLEGRO_CHANNEL_CONF_2);
if (!voice) {
abort_example("Could not create ALLEGRO_VOICE.\n");
}
mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32,
ALLEGRO_CHANNEL_CONF_2);
if (!mixer) {
abort_example("al_create_mixer failed.\n");
}
if (!al_attach_mixer_to_voice(mixer, voice)) {
abort_example("al_attach_mixer_to_voice failed.\n");
}
sample = al_create_sample_instance(NULL);
if (!sample) {
abort_example("al_create_sample failed.\n");
}
for (i = 0; i < n; ++i) {
ALLEGRO_SAMPLE *sample_data = NULL;
const char *filename = filenames[i];
float sample_time = 0;
float mono_to_stereo[] = {1.0, 0.0};
sample_data = al_load_sample(filename);
if (!sample_data) {
abort_example("Could not load sample from '%s'!\n",
filename);
continue;
}
if (!al_set_sample(sample, sample_data)) {
abort_example("al_set_sample_instance_ptr failed.\n");
continue;
}
if (!al_attach_sample_instance_to_mixer(sample, mixer)) {
abort_example("al_attach_sample_instance_to_mixer failed.\n");
goto done;
}
al_set_sample_instance_playmode(sample, ALLEGRO_PLAYMODE_LOOP);
al_play_sample_instance(sample);
sample_time = al_get_sample_instance_time(sample);
log_printf("Playing '%s' (%.3f seconds) normally.\n", filename,
sample_time);
al_rest(sample_time);
if (al_get_channel_count(al_get_sample_instance_channels(sample)) == 1) {
if (!al_set_sample_instance_channel_matrix(sample, mono_to_stereo)) {
abort_example("Failed to set channel matrix.\n");
}
log_printf("Playing left channel only.\n");
al_rest(sample_time);
}
if (!al_set_sample_instance_gain(sample, 0.5)) {
abort_example("Failed to set gain.\n");
}
log_printf("Playing with gain 0.5.\n");
al_rest(sample_time);
if (!al_set_sample_instance_gain(sample, 0.25)) {
abort_example("Failed to set gain.\n");
}
log_printf("Playing with gain 0.25.\n");
al_rest(sample_time);
al_stop_sample_instance(sample);
log_printf("Done playing '%s'\n", filename);
al_set_sample(sample, NULL);
al_destroy_sample(sample_data);
}
al_destroy_sample_instance(sample);
al_destroy_mixer(mixer);
al_destroy_voice(voice);
al_uninstall_audio();
done:
close_log(true);
return 0;
}
