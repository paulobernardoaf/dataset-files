
















#pragma once

#include <util/dstr.h>
#include <callback/calldata.h>
#include "obs-scripting.h"

struct obs_script {
enum obs_script_lang type;
bool loaded;

obs_data_t *settings;

struct dstr path;
struct dstr file;
struct dstr desc;
};

struct script_callback;
typedef void (*defer_call_cb)(void *param);

extern void defer_call_post(defer_call_cb call, void *cb);

extern void script_log(obs_script_t *script, int level, const char *format,
...);
extern void script_log_va(obs_script_t *script, int level, const char *format,
va_list args);

#define script_error(script, format, ...) script_log(script, LOG_ERROR, format, ##__VA_ARGS__)

#define script_warn(script, format, ...) script_log(script, LOG_WARNING, format, ##__VA_ARGS__)

#define script_info(script, format, ...) script_log(script, LOG_INFO, format, ##__VA_ARGS__)

#define script_debug(script, format, ...) script_log(script, LOG_DEBUG, format, ##__VA_ARGS__)

