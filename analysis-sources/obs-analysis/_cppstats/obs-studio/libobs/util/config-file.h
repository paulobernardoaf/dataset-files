#pragma once
#include "c99defs.h"
#if defined(__cplusplus)
extern "C" {
#endif
struct config_data;
typedef struct config_data config_t;
#define CONFIG_SUCCESS 0
#define CONFIG_FILENOTFOUND -1
#define CONFIG_ERROR -2
enum config_open_type {
CONFIG_OPEN_EXISTING,
CONFIG_OPEN_ALWAYS,
};
EXPORT config_t *config_create(const char *file);
EXPORT int config_open(config_t **config, const char *file,
enum config_open_type open_type);
EXPORT int config_open_string(config_t **config, const char *str);
EXPORT int config_save(config_t *config);
EXPORT int config_save_safe(config_t *config, const char *temp_ext,
const char *backup_ext);
EXPORT void config_close(config_t *config);
EXPORT size_t config_num_sections(config_t *config);
EXPORT const char *config_get_section(config_t *config, size_t idx);
EXPORT void config_set_string(config_t *config, const char *section,
const char *name, const char *value);
EXPORT void config_set_int(config_t *config, const char *section,
const char *name, int64_t value);
EXPORT void config_set_uint(config_t *config, const char *section,
const char *name, uint64_t value);
EXPORT void config_set_bool(config_t *config, const char *section,
const char *name, bool value);
EXPORT void config_set_double(config_t *config, const char *section,
const char *name, double value);
EXPORT const char *config_get_string(config_t *config, const char *section,
const char *name);
EXPORT int64_t config_get_int(config_t *config, const char *section,
const char *name);
EXPORT uint64_t config_get_uint(config_t *config, const char *section,
const char *name);
EXPORT bool config_get_bool(config_t *config, const char *section,
const char *name);
EXPORT double config_get_double(config_t *config, const char *section,
const char *name);
EXPORT bool config_remove_value(config_t *config, const char *section,
const char *name);
EXPORT int config_open_defaults(config_t *config, const char *file);
EXPORT void config_set_default_string(config_t *config, const char *section,
const char *name, const char *value);
EXPORT void config_set_default_int(config_t *config, const char *section,
const char *name, int64_t value);
EXPORT void config_set_default_uint(config_t *config, const char *section,
const char *name, uint64_t value);
EXPORT void config_set_default_bool(config_t *config, const char *section,
const char *name, bool value);
EXPORT void config_set_default_double(config_t *config, const char *section,
const char *name, double value);
EXPORT const char *config_get_default_string(config_t *config,
const char *section,
const char *name);
EXPORT int64_t config_get_default_int(config_t *config, const char *section,
const char *name);
EXPORT uint64_t config_get_default_uint(config_t *config, const char *section,
const char *name);
EXPORT bool config_get_default_bool(config_t *config, const char *section,
const char *name);
EXPORT double config_get_default_double(config_t *config, const char *section,
const char *name);
EXPORT bool config_has_user_value(config_t *config, const char *section,
const char *name);
EXPORT bool config_has_default_value(config_t *config, const char *section,
const char *name);
#if defined(__cplusplus)
}
#endif
