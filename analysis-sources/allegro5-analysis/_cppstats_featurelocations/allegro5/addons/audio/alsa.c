





















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_audio.h"

#include <alloca.h>
#include <alsa/asoundlib.h>

ALLEGRO_DEBUG_CHANNEL("alsa")

#define ALSA_CHECK(a) do { int err = (a); if (err < 0) { ALLEGRO_ERROR("%s: %s\n", snd_strerror(err), #a); goto Error; } } while(0)









static snd_output_t *snd_output = NULL;
static char *default_device = "default";
static char *alsa_device = NULL;







#define DEFAULT_PERIOD_SIZE 32
#define MIN_PERIOD_SIZE 1


#define DEFAULT_BUFFER_SIZE 2048

static unsigned int get_period_size(void)
{
const char *val = al_get_config_value(al_get_system_config(),
"alsa", "buffer_size");
if (val && val[0] != '\0') {
int n = atoi(val);
if (n < MIN_PERIOD_SIZE)
n = MIN_PERIOD_SIZE;
return n;
}

return DEFAULT_PERIOD_SIZE;
}

static snd_pcm_uframes_t get_buffer_size(void)
{
const char *val = al_get_config_value(al_get_system_config(),
"alsa", "buffer_size2");
if (val && val[0] != '\0') {
int n = atoi(val);
if (n < 1)
n = 1;
return n;
}

return DEFAULT_BUFFER_SIZE;
}

typedef struct ALSA_VOICE {
unsigned int frame_size; 
unsigned int len; 
snd_pcm_uframes_t frag_len; 
bool reversed; 

volatile bool stop;
volatile bool stopped;

struct pollfd *ufds;
int ufds_count;

ALLEGRO_THREAD *poll_thread;

snd_pcm_t *pcm_handle;
bool mmapped;
} ALSA_VOICE;




static int alsa_open(void)
{
alsa_device = default_device;

const char *config_device;
config_device = al_get_config_value(al_get_system_config(), "alsa", "device");
if (config_device && config_device[0] != '\0')
alsa_device = strdup(config_device);

ALSA_CHECK(snd_output_stdio_attach(&snd_output, stdout, 0));


snd_pcm_t *test_pcm_handle;
int alsa_err = snd_pcm_open(&test_pcm_handle, alsa_device,
SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
if (alsa_err < 0) {
ALLEGRO_WARN("ALSA is not available on the system.\n");
return 1;
}
else {
snd_pcm_close(test_pcm_handle);
}

return 0;


Error:
ALLEGRO_ERROR("Error initializing alsa!\n");
return 1;
}





static void alsa_close(void)
{
if (alsa_device != default_device)
al_free(alsa_device);

alsa_device = NULL;

snd_output_close(snd_output);
snd_config_update_free_global();
}



static int xrun_recovery(snd_pcm_t *handle, int err)
{
if (err == -EPIPE) { 
err = snd_pcm_prepare(handle);
if (err < 0) {
ALLEGRO_ERROR("Can't recover from underrun, prepare failed: %s\n", snd_strerror(err));
}
else {
ALLEGRO_DEBUG("Recovered from underrun\n");
}
return 0;
}
else if (err == -ESTRPIPE) { 
err = snd_pcm_resume(handle);
if (err < 0) {
ALLEGRO_ERROR("Can't recover from suspend, resume failed: %s\n", snd_strerror(err));
}
else {
ALLEGRO_DEBUG("Resumed successfully\n");
}
return 0;
}
else {
ALLEGRO_ERROR("Unknown error code: %d\n", err);
ASSERT(0);
}

return err;
}












static int alsa_update_nonstream_voice(ALLEGRO_VOICE *voice, void **buf, int *bytes)
{
ALSA_VOICE *alsa_voice = (ALSA_VOICE*)voice->extra;
int bpos = voice->attached_stream->pos * alsa_voice->frame_size;
int blen = alsa_voice->len * alsa_voice->frame_size;

*buf = (char *)voice->attached_stream->spl_data.buffer.ptr + bpos;

if (!alsa_voice->reversed) {
if (bpos + *bytes > blen) {
*bytes = blen - bpos;
if (voice->attached_stream->loop == ALLEGRO_PLAYMODE_ONCE) {
alsa_voice->stop = true;
voice->attached_stream->pos = 0;
}
if (voice->attached_stream->loop == ALLEGRO_PLAYMODE_LOOP) {
voice->attached_stream->pos = 0;
}
else if (voice->attached_stream->loop == ALLEGRO_PLAYMODE_BIDIR) {
alsa_voice->reversed = true;
voice->attached_stream->pos = alsa_voice->len;
}
return 1;
}
else
voice->attached_stream->pos += *bytes / alsa_voice->frame_size;
}
else {
if (bpos - *bytes < 0) {
*bytes = bpos;





voice->attached_stream->pos = 0;
alsa_voice->reversed = false;
return 1;
}
else
voice->attached_stream->pos -= *bytes / alsa_voice->frame_size;
}

return 0;
}



static int alsa_voice_is_ready(ALSA_VOICE *alsa_voice)
{
unsigned short revents;
int err;

poll(alsa_voice->ufds, alsa_voice->ufds_count, 0);
snd_pcm_poll_descriptors_revents(alsa_voice->pcm_handle, alsa_voice->ufds,
alsa_voice->ufds_count, &revents);

if (revents & POLLERR) {
if (snd_pcm_state(alsa_voice->pcm_handle) == SND_PCM_STATE_XRUN ||
snd_pcm_state(alsa_voice->pcm_handle) == SND_PCM_STATE_SUSPENDED) {

if (snd_pcm_state(alsa_voice->pcm_handle) == SND_PCM_STATE_XRUN)
err = -EPIPE;
else
err = -ESTRPIPE;

if (xrun_recovery(alsa_voice->pcm_handle, err) < 0) {
ALLEGRO_ERROR("Write error: %s\n", snd_strerror(err));
return -POLLERR;
}
} else {
ALLEGRO_ERROR("Wait for poll failed\n");
return -POLLERR;
}
}

if (revents & POLLOUT)
return true;

return false;
}




static void *alsa_update_mmap(ALLEGRO_THREAD *self, void *arg)
{
ALLEGRO_VOICE *voice = (ALLEGRO_VOICE*)arg;
ALSA_VOICE *alsa_voice = (ALSA_VOICE*)voice->extra;
snd_pcm_state_t last_state = -1;
snd_pcm_state_t state;
snd_pcm_uframes_t frames;
const snd_pcm_channel_area_t *areas;
snd_pcm_uframes_t offset;
char *mmap;
snd_pcm_sframes_t commitres;
int ret;

ALLEGRO_INFO("ALSA update_mmap thread started\n");

while (!al_get_thread_should_stop(self)) {
if (alsa_voice->stop && !alsa_voice->stopped) {
snd_pcm_drop(alsa_voice->pcm_handle);
al_lock_mutex(voice->mutex);
alsa_voice->stopped = true;
al_signal_cond(voice->cond);
al_unlock_mutex(voice->mutex);
}

if (!alsa_voice->stop && alsa_voice->stopped) {
alsa_voice->stopped = false;
}

if (alsa_voice->stopped) {


al_lock_mutex(voice->mutex);
while (alsa_voice->stop && !al_get_thread_should_stop(self)) {
al_wait_cond(voice->cond, voice->mutex);
}
al_unlock_mutex(voice->mutex);
continue;
}

state = snd_pcm_state(alsa_voice->pcm_handle);
if (state != last_state) {
ALLEGRO_DEBUG("state changed to: %s\n", snd_pcm_state_name(state));
last_state = state;
}
if (state == SND_PCM_STATE_SETUP) {
int rc = snd_pcm_prepare(alsa_voice->pcm_handle);
ALLEGRO_DEBUG("snd_pcm_prepare returned: %d\n", rc);
continue;
}
if (state == SND_PCM_STATE_PREPARED) {
int rc = snd_pcm_start(alsa_voice->pcm_handle);
ALLEGRO_DEBUG("snd_pcm_start returned: %d\n", rc);
}

ret = alsa_voice_is_ready(alsa_voice);
if (ret < 0)
break;
if (ret == 0) {
al_rest(0.005); 
continue;
}

snd_pcm_avail_update(alsa_voice->pcm_handle);
frames = alsa_voice->frag_len;
ret = snd_pcm_mmap_begin(alsa_voice->pcm_handle, &areas, &offset, &frames);
if (ret < 0) {
if ((ret = xrun_recovery(alsa_voice->pcm_handle, ret)) < 0) {
ALLEGRO_ERROR("MMAP begin avail error: %s\n", snd_strerror(ret));
}
break;
}

if (frames == 0) {
al_rest(0.005); 
goto commit;
}


mmap = (char *) areas[0].addr
+ areas[0].first / 8
+ offset * areas[0].step / 8;


if (!voice->is_streaming && !alsa_voice->stopped) {
void *buf;
bool reverse = alsa_voice->reversed;
int bytes = frames * alsa_voice->frame_size;

alsa_update_nonstream_voice(voice, &buf, &bytes);
frames = bytes / alsa_voice->frame_size;
if (!reverse) {
memcpy(mmap, buf, bytes);
}
else {

unsigned int i;
int fs = alsa_voice->frame_size;
for (i = 1; i <= frames; i++)
memcpy(mmap + i * fs, (char *) buf - i * fs, fs);
}
}
else if (voice->is_streaming && !alsa_voice->stopped) {

unsigned int iframes = frames;
const void *data = _al_voice_update(voice, voice->mutex, &iframes);
frames = iframes;
if (data == NULL)
goto silence;
memcpy(mmap, data, frames * alsa_voice->frame_size);
}
else {
silence:

al_fill_silence(mmap, frames, voice->depth, voice->chan_conf);
}

commit:
commitres = snd_pcm_mmap_commit(alsa_voice->pcm_handle, offset, frames);
if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) {
if ((ret = xrun_recovery(alsa_voice->pcm_handle, commitres >= 0 ? -EPIPE : commitres)) < 0) {
ALLEGRO_ERROR("MMAP commit error: %s\n", snd_strerror(ret));
break;
}
}
}

ALLEGRO_INFO("ALSA update_mmap thread stopped\n");

return NULL;
}


static void *alsa_update_rw(ALLEGRO_THREAD *self, void *arg)
{
ALLEGRO_VOICE *voice = (ALLEGRO_VOICE*)arg;
ALSA_VOICE *alsa_voice = (ALSA_VOICE*)voice->extra;
snd_pcm_state_t last_state = -1;
snd_pcm_state_t state;
snd_pcm_uframes_t frames;
snd_pcm_sframes_t err;

ALLEGRO_INFO("ALSA update_rw thread started\n");

while (!al_get_thread_should_stop(self)) {
if (alsa_voice->stop && !alsa_voice->stopped) {
snd_pcm_drop(alsa_voice->pcm_handle);
al_lock_mutex(voice->mutex);
alsa_voice->stopped = true;
al_signal_cond(voice->cond);
al_unlock_mutex(voice->mutex);
}

if (!alsa_voice->stop && alsa_voice->stopped) {
alsa_voice->stopped = false;
}

if (alsa_voice->stopped) {


al_lock_mutex(voice->mutex);
while (alsa_voice->stop && !al_get_thread_should_stop(self)) {
al_wait_cond(voice->cond, voice->mutex);
}
al_unlock_mutex(voice->mutex);
continue;
}

state = snd_pcm_state(alsa_voice->pcm_handle);
if (state != last_state) {
ALLEGRO_DEBUG("state changed to: %s\n", snd_pcm_state_name(state));
last_state = state;
}
if (state == SND_PCM_STATE_SETUP) {
int rc = snd_pcm_prepare(alsa_voice->pcm_handle);
ALLEGRO_DEBUG("snd_pcm_prepare returned: %d\n", rc);
continue;
}
if (state == SND_PCM_STATE_PREPARED) {
int rc = snd_pcm_start(alsa_voice->pcm_handle);
ALLEGRO_DEBUG("snd_pcm_start returned: %d\n", rc);
}

snd_pcm_wait(alsa_voice->pcm_handle, 10);
err = snd_pcm_avail_update(alsa_voice->pcm_handle);
if (err < 0) {
if (err == -EPIPE) {
snd_pcm_prepare(alsa_voice->pcm_handle);
}
else {
ALLEGRO_WARN("Alsa r/w thread exited "
"with error code %s.\n", snd_strerror(-err));
break;
}
}
if (err == 0) {
continue;
}
frames = err;
if (frames > alsa_voice->frag_len)
frames = alsa_voice->frag_len;

int bytes = frames * alsa_voice->frame_size;
uint8_t data[bytes];
void *buf;
if (!voice->is_streaming && !alsa_voice->stopped) {
ASSERT(!alsa_voice->reversed); 
alsa_update_nonstream_voice(voice, &buf, &bytes);
frames = bytes / alsa_voice->frame_size;
}
else if (voice->is_streaming && !alsa_voice->stopped) {

unsigned int iframes = frames;
buf = (void *)_al_voice_update(voice, voice->mutex, &iframes);
if (buf == NULL)
goto silence;
frames = iframes;
}
else {
silence:

al_fill_silence(data, frames, voice->depth, voice->chan_conf);
buf = data;
}
err = snd_pcm_writei(alsa_voice->pcm_handle, buf, frames);
if (err < 0) {
if (err == -EPIPE) {
snd_pcm_prepare(alsa_voice->pcm_handle);
}
}
}

ALLEGRO_INFO("ALSA update_rw thread stopped\n");

return NULL;
}







static int alsa_load_voice(ALLEGRO_VOICE *voice, const void *data)
{
ALSA_VOICE *ex_data = voice->extra;

voice->attached_stream->pos = 0;
ex_data->len = voice->attached_stream->spl_data.len;

return 0;
(void)data;
}





static void alsa_unload_voice(ALLEGRO_VOICE *voice)
{
(void)voice;
}







static int alsa_start_voice(ALLEGRO_VOICE *voice)
{
ALSA_VOICE *ex_data = voice->extra;


ex_data->stop = false;
al_signal_cond(voice->cond);

return 0;
}





static int alsa_stop_voice(ALLEGRO_VOICE *voice)
{
ALSA_VOICE *ex_data = voice->extra;


ex_data->stop = true;
al_signal_cond(voice->cond);

if (!voice->is_streaming) {
voice->attached_stream->pos = 0;
}

while (!ex_data->stopped) {
al_wait_cond(voice->cond, voice->mutex);
}

return 0;
}





static bool alsa_voice_is_playing(const ALLEGRO_VOICE *voice)
{
ALSA_VOICE *ex_data = voice->extra;
return !ex_data->stopped;
}




static int alsa_allocate_voice(ALLEGRO_VOICE *voice)
{
snd_pcm_format_t format;
int chan_count;
unsigned int req_freq;

ALSA_VOICE *ex_data = al_calloc(1, sizeof(ALSA_VOICE));
if (!ex_data)
return 1;

chan_count = al_get_channel_count(voice->chan_conf);
ex_data->frame_size = chan_count * al_get_audio_depth_size(voice->depth);
if (!ex_data->frame_size)
goto Error;

ex_data->stop = true;
ex_data->stopped = true;
ex_data->reversed = false;

ex_data->frag_len = get_period_size();

if (voice->depth == ALLEGRO_AUDIO_DEPTH_INT8)
format = SND_PCM_FORMAT_S8;
else if (voice->depth == ALLEGRO_AUDIO_DEPTH_UINT8)
format = SND_PCM_FORMAT_U8;
else if (voice->depth == ALLEGRO_AUDIO_DEPTH_INT16)
format = SND_PCM_FORMAT_S16;
else if (voice->depth == ALLEGRO_AUDIO_DEPTH_UINT16)
format = SND_PCM_FORMAT_U16;
else if (voice->depth == ALLEGRO_AUDIO_DEPTH_INT24)
format = SND_PCM_FORMAT_S24;
else if (voice->depth == ALLEGRO_AUDIO_DEPTH_UINT24)
format = SND_PCM_FORMAT_U24;
else if (voice->depth == ALLEGRO_AUDIO_DEPTH_FLOAT32)
format = SND_PCM_FORMAT_FLOAT;
else
goto Error;


if (voice->chan_conf == ALLEGRO_CHANNEL_CONF_3)
goto Error;

req_freq = voice->frequency;

ALSA_CHECK(snd_pcm_open(&ex_data->pcm_handle, alsa_device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK));

snd_pcm_hw_params_t *hwparams;
snd_pcm_hw_params_alloca(&hwparams);

ALSA_CHECK(snd_pcm_hw_params_any(ex_data->pcm_handle, hwparams));
if (snd_pcm_hw_params_set_access(ex_data->pcm_handle, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED) == 0) {
ex_data->mmapped = true;
}
else {
ALSA_CHECK(snd_pcm_hw_params_set_access(ex_data->pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED));
ex_data->mmapped = false;
}
ALSA_CHECK(snd_pcm_hw_params_set_format(ex_data->pcm_handle, hwparams, format));
ALSA_CHECK(snd_pcm_hw_params_set_channels(ex_data->pcm_handle, hwparams, chan_count));
ALSA_CHECK(snd_pcm_hw_params_set_rate_near(ex_data->pcm_handle, hwparams, &req_freq, NULL));
ALSA_CHECK(snd_pcm_hw_params_set_period_size_near(ex_data->pcm_handle, hwparams, &ex_data->frag_len, NULL));
snd_pcm_uframes_t buffer_size = get_buffer_size();
ALSA_CHECK(snd_pcm_hw_params_set_buffer_size_near(ex_data->pcm_handle, hwparams, &buffer_size));
ALSA_CHECK(snd_pcm_hw_params(ex_data->pcm_handle, hwparams));

if (voice->frequency != req_freq) {
ALLEGRO_ERROR("Unsupported rate! Requested %u, got %iu.\n", voice->frequency, req_freq);
goto Error;
}

snd_pcm_sw_params_t *swparams;
snd_pcm_sw_params_alloca(&swparams);

ALSA_CHECK(snd_pcm_sw_params_current(ex_data->pcm_handle, swparams));
ALSA_CHECK(snd_pcm_sw_params_set_start_threshold(ex_data->pcm_handle, swparams, ex_data->frag_len));
ALSA_CHECK(snd_pcm_sw_params_set_avail_min(ex_data->pcm_handle, swparams, ex_data->frag_len));
ALSA_CHECK(snd_pcm_sw_params(ex_data->pcm_handle, swparams));

ex_data->ufds_count = snd_pcm_poll_descriptors_count(ex_data->pcm_handle);
ex_data->ufds = al_malloc(sizeof(struct pollfd) * ex_data->ufds_count);
ALSA_CHECK(snd_pcm_poll_descriptors(ex_data->pcm_handle, ex_data->ufds, ex_data->ufds_count));

voice->extra = ex_data;

if (ex_data->mmapped) {
ex_data->poll_thread = al_create_thread(alsa_update_mmap, (void*)voice);
}
else {
ALLEGRO_WARN("Falling back to non-mmapped transfer.\n");
snd_pcm_nonblock(ex_data->pcm_handle, 0);
ex_data->poll_thread = al_create_thread(alsa_update_rw, (void*)voice);
}
al_start_thread(ex_data->poll_thread);

return 0;

Error:
if (ex_data->pcm_handle)
snd_pcm_close(ex_data->pcm_handle);
al_free(ex_data);
voice->extra = NULL;
return 1;
}






static void alsa_deallocate_voice(ALLEGRO_VOICE *voice)
{
ALSA_VOICE *alsa_voice = (ALSA_VOICE*)voice->extra;

al_lock_mutex(voice->mutex);
al_set_thread_should_stop(alsa_voice->poll_thread);
al_broadcast_cond(voice->cond);
al_unlock_mutex(voice->mutex);
al_join_thread(alsa_voice->poll_thread, NULL);

snd_pcm_drop(alsa_voice->pcm_handle);
snd_pcm_close(alsa_voice->pcm_handle);

al_destroy_thread(alsa_voice->poll_thread);
al_free(alsa_voice->ufds);
al_free(voice->extra);
voice->extra = NULL;
}






static unsigned int alsa_get_voice_position(const ALLEGRO_VOICE *voice)
{
return voice->attached_stream->pos;
}






static int alsa_set_voice_position(ALLEGRO_VOICE *voice, unsigned int val)
{
voice->attached_stream->pos = val;
return 0;
}

typedef struct ALSA_RECORDER_DATA
{
snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;
} ALSA_RECORDER_DATA;

static void *alsa_update_recorder(ALLEGRO_THREAD *t, void *thread_data)
{
ALLEGRO_AUDIO_RECORDER *r = thread_data;
ALSA_RECORDER_DATA *alsa = r->extra;
ALLEGRO_EVENT user_event;
uint8_t *null_buffer;
unsigned int fragment_i = 0;

null_buffer = al_malloc(1024 * r->sample_size);
if (!null_buffer) {
ALLEGRO_ERROR("Unable to create buffer for draining ALSA.\n");
return NULL;
}

while (!al_get_thread_should_stop(t))
{
al_lock_mutex(r->mutex);
if (!r->is_recording) {





al_unlock_mutex(r->mutex);
snd_pcm_readi(alsa->capture_handle, null_buffer, 1024);
}
else {
ALLEGRO_AUDIO_RECORDER_EVENT *e;
snd_pcm_sframes_t count;
al_unlock_mutex(r->mutex);
if ((count = snd_pcm_readi(alsa->capture_handle, r->fragments[fragment_i], r->samples)) > 0) {
user_event.user.type = ALLEGRO_EVENT_AUDIO_RECORDER_FRAGMENT;
e = al_get_audio_recorder_event(&user_event);
e->buffer = r->fragments[fragment_i];
e->samples = count;
al_emit_user_event(&r->source, &user_event, NULL);

if (++fragment_i == r->fragment_count) {
fragment_i = 0;
}
}
}
}

al_free(null_buffer);
return NULL;
}

static int alsa_allocate_recorder(ALLEGRO_AUDIO_RECORDER *r)
{
ALSA_RECORDER_DATA *data;
unsigned int frequency = r->frequency;
snd_pcm_format_t format;
const char *device = default_device;
const char *config_device;
config_device =
al_get_config_value(al_get_system_config(), "alsa", "capture_device");
if (config_device && config_device[0] != '\0')
device = config_device;

data = al_calloc(1, sizeof(*data));

if (!data) {
goto Error;
}

if (r->depth == ALLEGRO_AUDIO_DEPTH_INT8)
format = SND_PCM_FORMAT_S8;
else if (r->depth == ALLEGRO_AUDIO_DEPTH_UINT8)
format = SND_PCM_FORMAT_U8;
else if (r->depth == ALLEGRO_AUDIO_DEPTH_INT16)
format = SND_PCM_FORMAT_S16;
else if (r->depth == ALLEGRO_AUDIO_DEPTH_UINT16)
format = SND_PCM_FORMAT_U16;
else if (r->depth == ALLEGRO_AUDIO_DEPTH_INT24)
format = SND_PCM_FORMAT_S24;
else if (r->depth == ALLEGRO_AUDIO_DEPTH_UINT24)
format = SND_PCM_FORMAT_U24;
else if (r->depth == ALLEGRO_AUDIO_DEPTH_FLOAT32)
format = SND_PCM_FORMAT_FLOAT;
else
goto Error;

ALSA_CHECK(snd_pcm_open(&data->capture_handle, device, SND_PCM_STREAM_CAPTURE, 0));
ALSA_CHECK(snd_pcm_hw_params_malloc(&data->hw_params));
ALSA_CHECK(snd_pcm_hw_params_any(data->capture_handle, data->hw_params));
ALSA_CHECK(snd_pcm_hw_params_set_access(data->capture_handle, data->hw_params, SND_PCM_ACCESS_RW_INTERLEAVED));
ALSA_CHECK(snd_pcm_hw_params_set_format(data->capture_handle, data->hw_params, format));
ALSA_CHECK(snd_pcm_hw_params_set_rate_near(data->capture_handle, data->hw_params, &frequency, 0));

if (frequency != r->frequency) {
ALLEGRO_ERROR("Unsupported rate! Requested %u, got %iu.\n", r->frequency, frequency);
goto Error;
}

ALSA_CHECK(snd_pcm_hw_params_set_channels(data->capture_handle, data->hw_params, al_get_channel_count(r->chan_conf)));
ALSA_CHECK(snd_pcm_hw_params(data->capture_handle, data->hw_params));

ALSA_CHECK(snd_pcm_prepare(data->capture_handle));

r->extra = data;
r->thread = al_create_thread(alsa_update_recorder, r);

return 0;

Error:

if (data) {
if (data->hw_params) {
snd_pcm_hw_params_free(data->hw_params);
}
if (data->capture_handle) {
snd_pcm_close(data->capture_handle);
}
al_free(data);
}

return 1;
}

static void alsa_deallocate_recorder(ALLEGRO_AUDIO_RECORDER *r)
{
ALSA_RECORDER_DATA *data = r->extra;

snd_pcm_hw_params_free(data->hw_params);
snd_pcm_close(data->capture_handle);
}

ALLEGRO_AUDIO_DRIVER _al_kcm_alsa_driver =
{
"ALSA",

alsa_open,
alsa_close,

alsa_allocate_voice,
alsa_deallocate_voice,

alsa_load_voice,
alsa_unload_voice,

alsa_start_voice,
alsa_stop_voice,

alsa_voice_is_playing,

alsa_get_voice_position,
alsa_set_voice_position,

alsa_allocate_recorder,
alsa_deallocate_recorder
};


