
















#pragma once

#include "util/c99defs.h"

#if defined(__cplusplus)
extern "C" {
#endif















struct obs_modal_ui {
const char *id; 
const char *task; 
const char *target; 























bool (*exec)(void *object, void *ui_data);

void *type_data;
void (*free_type_data)(void *type_data);
};







EXPORT void obs_register_modal_ui(const struct obs_modal_ui *info);




struct obs_modeless_ui {
const char *id; 
const char *task; 
const char *target; 


















void *(*create)(void *object, void *ui_data);

void *type_data;
void (*free_type_data)(void *type_data);
};







EXPORT void obs_register_modeless_ui(const struct obs_modeless_ui *info);



#define OBS_UI_SUCCESS 0
#define OBS_UI_CANCEL -1
#define OBS_UI_NOTFOUND -2














EXPORT int obs_exec_ui(const char *id, const char *task, const char *target,
void *data, void *ui_data);













EXPORT void *obs_create_ui(const char *id, const char *task, const char *target,
void *data, void *ui_data);

#if defined(__cplusplus)
}
#endif
