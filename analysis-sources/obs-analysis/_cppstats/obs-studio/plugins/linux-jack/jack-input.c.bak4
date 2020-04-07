
















#include "jack-wrapper.h"

#include <obs-module.h>




static const char *jack_input_getname(void *unused)
{
UNUSED_PARAMETER(unused);
return obs_module_text("JACKInput");
}




static void jack_destroy(void *vptr)
{
struct jack_data *data = (struct jack_data *)vptr;

if (!data)
return;

deactivate_jack(data);

if (data->device)
bfree(data->device);
pthread_mutex_destroy(&data->jack_mutex);
bfree(data);
}




static void jack_update(void *vptr, obs_data_t *settings)
{
struct jack_data *data = (struct jack_data *)vptr;
if (!data)
return;

const char *new_device;
bool settings_changed = false;
bool new_jack_start_server = obs_data_get_bool(settings, "startjack");
int new_channel_count = obs_data_get_int(settings, "channels");

if (new_jack_start_server != data->start_jack_server) {
data->start_jack_server = new_jack_start_server;
settings_changed = true;
}

if (new_channel_count != data->channels)




settings_changed = true;

new_device = obs_source_get_name(data->source);
if (!data->device || strcmp(data->device, new_device) != 0) {
if (data->device)
bfree(data->device);
data->device = bstrdup(new_device);
settings_changed = true;
}

if (settings_changed) {
deactivate_jack(data);

data->channels = new_channel_count;

if (jack_init(data) != 0) {
deactivate_jack(data);
}
}
}




static void *jack_create(obs_data_t *settings, obs_source_t *source)
{
struct jack_data *data = bzalloc(sizeof(struct jack_data));

pthread_mutex_init(&data->jack_mutex, NULL);
data->source = source;
data->channels = -1;

jack_update(data, settings);

if (data->jack_client == NULL) {
jack_destroy(data);
return NULL;
}
return data;
}




static void jack_input_defaults(obs_data_t *settings)
{
obs_data_set_default_int(settings, "channels", 2);
obs_data_set_default_bool(settings, "startjack", false);
}




static obs_properties_t *jack_input_properties(void *unused)
{
(void)unused;

obs_properties_t *props = obs_properties_create();

obs_properties_add_int(props, "channels", obs_module_text("Channels"),
1, 8, 1);
obs_properties_add_bool(props, "startjack",
obs_module_text("StartJACKServer"));

return props;
}

struct obs_source_info jack_output_capture = {
.id = "jack_output_capture",
.type = OBS_SOURCE_TYPE_INPUT,
.output_flags = OBS_SOURCE_AUDIO,
.get_name = jack_input_getname,
.create = jack_create,
.destroy = jack_destroy,
.update = jack_update,
.get_defaults = jack_input_defaults,
.get_properties = jack_input_properties,
.icon_type = OBS_ICON_TYPE_AUDIO_OUTPUT,
};
