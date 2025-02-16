#pragma once
#include "../util/c99defs.h"
#include "calldata.h"
#if defined(__cplusplus)
extern "C" {
#endif
struct signal_handler;
typedef struct signal_handler signal_handler_t;
typedef void (*global_signal_callback_t)(void *, const char *, calldata_t *);
typedef void (*signal_callback_t)(void *, calldata_t *);
EXPORT signal_handler_t *signal_handler_create(void);
EXPORT void signal_handler_destroy(signal_handler_t *handler);
EXPORT bool signal_handler_add(signal_handler_t *handler,
const char *signal_decl);
static inline bool signal_handler_add_array(signal_handler_t *handler,
const char **signal_decls)
{
bool success = true;
if (!signal_decls)
return false;
while (*signal_decls)
if (!signal_handler_add(handler, *(signal_decls++)))
success = false;
return success;
}
EXPORT void signal_handler_connect(signal_handler_t *handler,
const char *signal,
signal_callback_t callback, void *data);
EXPORT void signal_handler_connect_ref(signal_handler_t *handler,
const char *signal,
signal_callback_t callback, void *data);
EXPORT void signal_handler_disconnect(signal_handler_t *handler,
const char *signal,
signal_callback_t callback, void *data);
EXPORT void signal_handler_connect_global(signal_handler_t *handler,
global_signal_callback_t callback,
void *data);
EXPORT void signal_handler_disconnect_global(signal_handler_t *handler,
global_signal_callback_t callback,
void *data);
EXPORT void signal_handler_remove_current(void);
EXPORT void signal_handler_signal(signal_handler_t *handler, const char *signal,
calldata_t *params);
#if defined(__cplusplus)
}
#endif
