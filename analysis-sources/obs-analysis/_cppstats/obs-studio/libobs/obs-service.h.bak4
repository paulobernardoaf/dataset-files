
















#pragma once








#if defined(__cplusplus)
extern "C" {
#endif

struct obs_service_info {

const char *id;

const char *(*get_name)(void *type_data);
void *(*create)(obs_data_t *settings, obs_service_t *service);
void (*destroy)(void *data);


void (*activate)(void *data, obs_data_t *settings);
void (*deactivate)(void *data);

void (*update)(void *data, obs_data_t *settings);

void (*get_defaults)(obs_data_t *settings);

obs_properties_t *(*get_properties)(void *data);










bool (*initialize)(void *data, obs_output_t *output);

const char *(*get_url)(void *data);
const char *(*get_key)(void *data);

const char *(*get_username)(void *data);
const char *(*get_password)(void *data);

bool (*supports_multitrack)(void *data);

void (*apply_encoder_settings)(void *data,
obs_data_t *video_encoder_settings,
obs_data_t *audio_encoder_settings);

void *type_data;
void (*free_type_data)(void *type_data);

const char *(*get_output_type)(void *data);


};

EXPORT void obs_register_service_s(const struct obs_service_info *info,
size_t size);

#define obs_register_service(info) obs_register_service_s(info, sizeof(struct obs_service_info))


#if defined(__cplusplus)
}
#endif
