









#include <stdio.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include "common.c"






int main(int argc, char **argv)
{
int i;
ALLEGRO_VOICE* voice;
ALLEGRO_MIXER* mixer;
bool loop = false;
int arg_start = 1;

if (!al_init()) {
abort_example("Could not init Allegro.\n");
}

open_log();

if (argc < 2) {
log_printf("This example needs to be run from the command line.\n");
log_printf("Usage: %s [--loop] {audio_files}\n", argv[0]);
goto done;
}

if (strcmp(argv[1], "--loop") == 0) {
loop = true;
arg_start = 2;
}

al_init_acodec_addon();

if (!al_install_audio()) {
abort_example("Could not init sound!\n");
}

voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16,
ALLEGRO_CHANNEL_CONF_2);
if (!voice) {
abort_example("Could not create ALLEGRO_VOICE.\n");
}
log_printf("Voice created.\n");

#if !defined(BYPASS_MIXER)
mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32,
ALLEGRO_CHANNEL_CONF_2);
if (!mixer) {
abort_example("Could not create ALLEGRO_MIXER.\n");
}
log_printf("Mixer created.\n");

if (!al_attach_mixer_to_voice(mixer, voice)) {
abort_example("al_attach_mixer_to_voice failed.\n");
}
#endif

for (i = arg_start; i < argc; ++i)
{
ALLEGRO_AUDIO_STREAM* stream;
const char* filename = argv[i];
bool playing = true;
ALLEGRO_EVENT event;
ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

stream = al_load_audio_stream(filename, 4, 2048);
if (!stream) {
log_printf("Could not create an ALLEGRO_AUDIO_STREAM from '%s'!\n",
filename);
continue;
}
log_printf("Stream created from '%s'.\n", filename);
if (loop) {
al_set_audio_stream_playmode(stream, loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
}

al_register_event_source(queue, al_get_audio_stream_event_source(stream));

#if !defined(BYPASS_MIXER)
if (!al_attach_audio_stream_to_mixer(stream, mixer)) {
log_printf("al_attach_audio_stream_to_mixer failed.\n");
continue;
}
#else
if (!al_attach_audio_stream_to_voice(stream, voice)) {
abort_example("al_attach_audio_stream_to_voice failed.\n");
}
#endif

log_printf("Playing %s ... Waiting for stream to finish ", filename);
do {
al_wait_for_event(queue, &event);
if(event.type == ALLEGRO_EVENT_AUDIO_STREAM_FINISHED)
playing = false;
} while (playing);
log_printf("\n");

al_destroy_event_queue(queue);
al_destroy_audio_stream(stream);
}
log_printf("Done\n");

#if !defined(BYPASS_MIXER)
al_destroy_mixer(mixer);
#endif
al_destroy_voice(voice);

al_uninstall_audio();
done:
close_log(true);

return 0;
}


