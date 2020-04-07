#include "../util/darray.h"
#include "decl.h"
#include "proc.h"
struct proc_info {
struct decl_info func;
void *data;
proc_handler_proc_t callback;
};
static inline void proc_info_free(struct proc_info *pi)
{
decl_info_free(&pi->func);
}
struct proc_handler {
DARRAY(struct proc_info) procs;
};
proc_handler_t *proc_handler_create(void)
{
struct proc_handler *handler = bmalloc(sizeof(struct proc_handler));
da_init(handler->procs);
return handler;
}
void proc_handler_destroy(proc_handler_t *handler)
{
if (handler) {
for (size_t i = 0; i < handler->procs.num; i++)
proc_info_free(handler->procs.array + i);
da_free(handler->procs);
bfree(handler);
}
}
void proc_handler_add(proc_handler_t *handler, const char *decl_string,
proc_handler_proc_t proc, void *data)
{
if (!handler)
return;
struct proc_info pi;
memset(&pi, 0, sizeof(struct proc_info));
if (!parse_decl_string(&pi.func, decl_string)) {
blog(LOG_ERROR, "Function declaration invalid: %s",
decl_string);
return;
}
pi.callback = proc;
pi.data = data;
da_push_back(handler->procs, &pi);
}
bool proc_handler_call(proc_handler_t *handler, const char *name,
calldata_t *params)
{
if (!handler)
return false;
for (size_t i = 0; i < handler->procs.num; i++) {
struct proc_info *info = handler->procs.array + i;
if (strcmp(info->func.name, name) == 0) {
info->callback(info->data, params);
return true;
}
}
return false;
}
