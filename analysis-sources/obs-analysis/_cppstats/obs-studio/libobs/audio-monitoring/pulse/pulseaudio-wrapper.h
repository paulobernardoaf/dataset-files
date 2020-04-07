#include <inttypes.h>
#include <pulse/stream.h>
#include <pulse/context.h>
#include <pulse/introspect.h>
#pragma once
struct pulseaudio_default_output {
char *default_sink_name;
};
struct enum_cb {
obs_enum_audio_device_cb cb;
void *data;
int cont;
};
void get_default_id(char **id);
bool devices_match(const char *id1, const char *id2);
int_fast32_t pulseaudio_init();
void pulseaudio_unref();
void pulseaudio_lock();
void pulseaudio_unlock();
void pulseaudio_wait();
void pulseaudio_signal(int wait_for_accept);
void pulseaudio_accept();
int_fast32_t pulseaudio_get_source_info_list(pa_source_info_cb_t cb,
void *userdata);
int_fast32_t pulseaudio_get_source_info(pa_source_info_cb_t cb,
const char *name, void *userdata);
int_fast32_t pulseaudio_get_server_info(pa_server_info_cb_t cb, void *userdata);
pa_stream *pulseaudio_stream_new(const char *name, const pa_sample_spec *ss,
const pa_channel_map *map);
int_fast32_t pulseaudio_connect_playback(pa_stream *s, const char *name,
const pa_buffer_attr *attr,
pa_stream_flags_t flags);
void pulseaudio_write_callback(pa_stream *p, pa_stream_request_cb_t cb,
void *userdata);
void pulseaudio_set_underflow_callback(pa_stream *p, pa_stream_notify_cb_t cb,
void *userdata);
