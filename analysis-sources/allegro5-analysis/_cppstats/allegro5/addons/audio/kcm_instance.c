#include <math.h>
#include <stdio.h>
#include "allegro5/allegro_audio.h"
#include "allegro5/internal/aintern_audio.h"
#include "allegro5/internal/aintern_audio_cfg.h"
static void maybe_lock_mutex(ALLEGRO_MUTEX *mutex)
{
if (mutex) {
al_lock_mutex(mutex);
}
}
static void maybe_unlock_mutex(ALLEGRO_MUTEX *mutex)
{
if (mutex) {
al_unlock_mutex(mutex);
}
}
void _al_kcm_stream_set_mutex(ALLEGRO_SAMPLE_INSTANCE *stream, ALLEGRO_MUTEX *mutex)
{
ASSERT(stream);
if (stream->mutex == mutex)
return;
stream->mutex = mutex;
if (stream->is_mixer) {
ALLEGRO_MIXER *mixer = (ALLEGRO_MIXER *)stream;
int i;
for (i = _al_vector_size(&mixer->streams) - 1; i >= 0; i--) {
ALLEGRO_SAMPLE_INSTANCE **slot = _al_vector_ref(&mixer->streams, i);
ALLEGRO_SAMPLE_INSTANCE *spl = *slot;
_al_kcm_stream_set_mutex(spl, mutex);
}
}
}
static void stream_free(ALLEGRO_SAMPLE_INSTANCE *spl)
{
if (spl) {
if (spl->is_mixer) {
ALLEGRO_MIXER *mixer = (ALLEGRO_MIXER *)spl;
int i;
_al_kcm_stream_set_mutex(&mixer->ss, NULL);
for (i = _al_vector_size(&mixer->streams) - 1; i >= 0; i--) {
ALLEGRO_SAMPLE_INSTANCE **slot = _al_vector_ref(&mixer->streams, i);
ALLEGRO_SAMPLE_INSTANCE *spl = *slot;
spl->parent.u.ptr = NULL;
spl->spl_read = NULL;
al_free(spl->matrix);
spl->matrix = NULL;
}
_al_vector_free(&mixer->streams);
if (spl->spl_data.buffer.ptr) {
ASSERT(spl->spl_data.free_buf);
al_free(spl->spl_data.buffer.ptr);
spl->spl_data.buffer.ptr = NULL;
}
spl->spl_data.free_buf = false;
}
ASSERT(! spl->spl_data.free_buf);
al_free(spl);
}
}
void _al_kcm_detach_from_parent(ALLEGRO_SAMPLE_INSTANCE *spl)
{
ALLEGRO_MIXER *mixer;
int i;
if (!spl || !spl->parent.u.ptr)
return;
if (spl->parent.is_voice) {
al_detach_voice(spl->parent.u.voice);
return;
}
mixer = spl->parent.u.mixer;
for (i = _al_vector_size(&mixer->streams) - 1; i >= 0; i--) {
ALLEGRO_SAMPLE_INSTANCE **slot = _al_vector_ref(&mixer->streams, i);
if (*slot == spl) {
maybe_lock_mutex(mixer->ss.mutex);
_al_vector_delete_at(&mixer->streams, i);
spl->parent.u.mixer = NULL;
_al_kcm_stream_set_mutex(spl, NULL);
spl->spl_read = NULL;
maybe_unlock_mutex(mixer->ss.mutex);
break;
}
}
al_free(spl->matrix);
spl->matrix = NULL;
}
ALLEGRO_SAMPLE_INSTANCE *al_create_sample_instance(ALLEGRO_SAMPLE *sample_data)
{
ALLEGRO_SAMPLE_INSTANCE *spl;
spl = al_calloc(1, sizeof(*spl));
if (!spl) {
_al_set_error(ALLEGRO_GENERIC_ERROR,
"Out of memory allocating sample object");
return NULL;
}
if (sample_data) {
spl->spl_data = *sample_data;
}
spl->spl_data.free_buf = false;
spl->loop = ALLEGRO_PLAYMODE_ONCE;
spl->speed = 1.0f;
spl->gain = 1.0f;
spl->pan = 0.0f;
spl->pos = 0;
spl->loop_start = 0;
spl->loop_end = sample_data ? sample_data->len : 0;
spl->step = 0;
spl->matrix = NULL;
spl->is_mixer = false;
spl->spl_read = NULL;
spl->mutex = NULL;
spl->parent.u.ptr = NULL;
spl->dtor_item = _al_kcm_register_destructor("sample_instance", spl,
(void (*)(void *))al_destroy_sample_instance);
return spl;
}
void al_destroy_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl)
{
_al_kcm_destroy_sample(spl, true);
}
void _al_kcm_destroy_sample(ALLEGRO_SAMPLE_INSTANCE *spl, bool unregister)
{
if (spl) {
if (unregister) {
_al_kcm_unregister_destructor(spl->dtor_item);
}
_al_kcm_detach_from_parent(spl);
stream_free(spl);
}
}
bool al_play_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return al_set_sample_instance_playing(spl, true);
}
bool al_stop_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return al_set_sample_instance_playing(spl, false);
}
unsigned int al_get_sample_instance_frequency(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->spl_data.frequency;
}
unsigned int al_get_sample_instance_length(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->spl_data.len;
}
unsigned int al_get_sample_instance_position(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
if (spl->parent.u.ptr && spl->parent.is_voice) {
ALLEGRO_VOICE *voice = spl->parent.u.voice;
return al_get_voice_position(voice);
}
return spl->pos;
}
float al_get_sample_instance_speed(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->speed;
}
float al_get_sample_instance_gain(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->gain;
}
float al_get_sample_instance_pan(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->pan;
}
float al_get_sample_instance_time(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return (float)(spl->spl_data.len)
/ (float)spl->spl_data.frequency;
}
ALLEGRO_AUDIO_DEPTH al_get_sample_instance_depth(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->spl_data.depth;
}
ALLEGRO_CHANNEL_CONF al_get_sample_instance_channels(
const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->spl_data.chan_conf;
}
ALLEGRO_PLAYMODE al_get_sample_instance_playmode(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return spl->loop;
}
bool al_get_sample_instance_playing(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
if (spl->parent.u.ptr && spl->parent.is_voice) {
ALLEGRO_VOICE *voice = spl->parent.u.voice;
return al_get_voice_playing(voice);
}
return spl->is_playing;
}
bool al_get_sample_instance_attached(const ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return (spl->parent.u.ptr != NULL);
}
bool al_set_sample_instance_position(ALLEGRO_SAMPLE_INSTANCE *spl,
unsigned int val)
{
ASSERT(spl);
if (spl->parent.u.ptr && spl->parent.is_voice) {
ALLEGRO_VOICE *voice = spl->parent.u.voice;
if (!al_set_voice_position(voice, val))
return false;
}
else {
maybe_lock_mutex(spl->mutex);
spl->pos = val;
maybe_unlock_mutex(spl->mutex);
}
return true;
}
bool al_set_sample_instance_length(ALLEGRO_SAMPLE_INSTANCE *spl,
unsigned int val)
{
ASSERT(spl);
if (spl->is_playing) {
_al_set_error(ALLEGRO_INVALID_OBJECT,
"Attempted to change the length of a playing sample");
return false;
}
spl->spl_data.len = val;
spl->loop_end = val;
return true;
}
bool al_set_sample_instance_speed(ALLEGRO_SAMPLE_INSTANCE *spl, float val)
{
ASSERT(spl);
if (fabsf(val) < (1.0f/64.0f)) {
_al_set_error(ALLEGRO_INVALID_PARAM,
"Attempted to set zero speed");
return false;
}
if (spl->parent.u.ptr && spl->parent.is_voice) {
_al_set_error(ALLEGRO_GENERIC_ERROR,
"Could not set voice playback speed");
return false;
}
spl->speed = val;
if (spl->parent.u.mixer) {
ALLEGRO_MIXER *mixer = spl->parent.u.mixer;
maybe_lock_mutex(spl->mutex);
spl->step = (spl->spl_data.frequency) * spl->speed;
spl->step_denom = mixer->ss.spl_data.frequency;
if (spl->step == 0) {
if (spl->speed > 0.0f)
spl->step = 1;
else
spl->step = -1;
}
maybe_unlock_mutex(spl->mutex);
}
return true;
}
bool al_set_sample_instance_gain(ALLEGRO_SAMPLE_INSTANCE *spl, float val)
{
ASSERT(spl);
if (spl->parent.u.ptr && spl->parent.is_voice) {
_al_set_error(ALLEGRO_GENERIC_ERROR,
"Could not set gain of sample attached to voice");
return false;
}
if (spl->gain != val) {
spl->gain = val;
if (spl->parent.u.mixer) {
ALLEGRO_MIXER *mixer = spl->parent.u.mixer;
maybe_lock_mutex(spl->mutex);
_al_kcm_mixer_rejig_sample_matrix(mixer, spl);
maybe_unlock_mutex(spl->mutex);
}
}
return true;
}
bool al_set_sample_instance_pan(ALLEGRO_SAMPLE_INSTANCE *spl, float val)
{
ASSERT(spl);
if (spl->parent.u.ptr && spl->parent.is_voice) {
_al_set_error(ALLEGRO_GENERIC_ERROR,
"Could not set panning of sample attached to voice");
return false;
}
if (val != ALLEGRO_AUDIO_PAN_NONE && (val < -1.0 || val > 1.0)) {
_al_set_error(ALLEGRO_GENERIC_ERROR, "Invalid pan value");
return false;
}
if (spl->pan != val) {
spl->pan = val;
if (spl->parent.u.mixer) {
ALLEGRO_MIXER *mixer = spl->parent.u.mixer;
maybe_lock_mutex(spl->mutex);
_al_kcm_mixer_rejig_sample_matrix(mixer, spl);
maybe_unlock_mutex(spl->mutex);
}
}
return true;
}
bool al_set_sample_instance_playmode(ALLEGRO_SAMPLE_INSTANCE *spl,
ALLEGRO_PLAYMODE val)
{
ASSERT(spl);
if (val < ALLEGRO_PLAYMODE_ONCE || val > ALLEGRO_PLAYMODE_BIDIR) {
_al_set_error(ALLEGRO_INVALID_PARAM,
"Invalid loop mode");
return false;
}
maybe_lock_mutex(spl->mutex);
spl->loop = val;
if (spl->loop != ALLEGRO_PLAYMODE_ONCE) {
if (spl->pos < spl->loop_start)
spl->pos = spl->loop_start;
else if (spl->pos > spl->loop_end-1)
spl->pos = spl->loop_end-1;
}
maybe_unlock_mutex(spl->mutex);
return true;
}
bool al_set_sample_instance_playing(ALLEGRO_SAMPLE_INSTANCE *spl, bool val)
{
ASSERT(spl);
if (!spl->parent.u.ptr || !spl->spl_data.buffer.ptr) {
spl->is_playing = val;
return true;
}
if (spl->parent.is_voice) {
ALLEGRO_VOICE *voice = spl->parent.u.voice;
return al_set_voice_playing(voice, val);
}
maybe_lock_mutex(spl->mutex);
spl->is_playing = val;
if (!val)
spl->pos = 0;
maybe_unlock_mutex(spl->mutex);
return true;
}
bool al_detach_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
_al_kcm_detach_from_parent(spl);
ASSERT(spl->spl_read == NULL);
return true;
}
bool al_set_sample(ALLEGRO_SAMPLE_INSTANCE *spl, ALLEGRO_SAMPLE *data)
{
sample_parent_t old_parent;
bool need_reattach;
ASSERT(spl);
if (spl->is_playing) {
if (!al_set_sample_instance_playing(spl, false)) {
ASSERT(false);
return false;
}
}
if (!data) {
if (spl->parent.u.ptr) {
_al_kcm_detach_from_parent(spl);
}
spl->spl_data.buffer.ptr = NULL;
return true;
}
need_reattach = false;
if (spl->parent.u.ptr != NULL) {
if (spl->spl_data.frequency != data->frequency ||
spl->spl_data.depth != data->depth ||
spl->spl_data.chan_conf != data->chan_conf) {
old_parent = spl->parent;
need_reattach = true;
_al_kcm_detach_from_parent(spl);
}
}
spl->spl_data = *data;
spl->spl_data.free_buf = false;
spl->pos = 0;
spl->loop_start = 0;
spl->loop_end = data->len;
if (need_reattach) {
if (old_parent.is_voice) {
if (!al_attach_sample_instance_to_voice(spl, old_parent.u.voice)) {
spl->spl_data.buffer.ptr = NULL;
return false;
}
}
else {
if (!al_attach_sample_instance_to_mixer(spl, old_parent.u.mixer)) {
spl->spl_data.buffer.ptr = NULL;
return false;
}
}
}
return true;
}
ALLEGRO_SAMPLE *al_get_sample(ALLEGRO_SAMPLE_INSTANCE *spl)
{
ASSERT(spl);
return &(spl->spl_data);
}
bool al_set_sample_instance_channel_matrix(ALLEGRO_SAMPLE_INSTANCE *spl, const float *matrix)
{
ASSERT(spl);
ASSERT(matrix);
if (spl->parent.u.ptr && spl->parent.is_voice) {
_al_set_error(ALLEGRO_GENERIC_ERROR,
"Could not set channel matrix of sample attached to voice");
return false;
}
if (spl->parent.u.mixer) {
ALLEGRO_MIXER *mixer = spl->parent.u.mixer;
size_t dst_chans = al_get_channel_count(mixer->ss.spl_data.chan_conf);
size_t src_chans = al_get_channel_count(spl->spl_data.chan_conf);
ASSERT(spl->matrix);
maybe_lock_mutex(spl->mutex);
memcpy(spl->matrix, matrix, dst_chans * src_chans * sizeof(float));
maybe_unlock_mutex(spl->mutex);
}
return true;
}
