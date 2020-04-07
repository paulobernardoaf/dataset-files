#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <vlc/vlc.h>
#include "libvlc_internal.h"
#include <vlc_common.h>
#include <vlc_interface.h>
void libvlc_log_get_context(const libvlc_log_t *ctx,
const char **restrict module,
const char **restrict file,
unsigned *restrict line)
{
if (module != NULL)
*module = ctx->psz_module;
if (file != NULL)
*file = ctx->file;
if (line != NULL)
*line = ctx->line;
}
void libvlc_log_get_object(const libvlc_log_t *ctx,
const char **restrict name,
const char **restrict header,
uintptr_t *restrict id)
{
if (name != NULL)
*name = (ctx->psz_object_type != NULL)
? ctx->psz_object_type : "generic";
if (header != NULL)
*header = ctx->psz_header;
if (id != NULL)
*id = ctx->i_object_id;
}
static void libvlc_logf (void *data, int level, const vlc_log_t *item,
const char *fmt, va_list ap)
{
libvlc_instance_t *inst = data;
switch (level)
{
case VLC_MSG_INFO: level = LIBVLC_NOTICE; break;
case VLC_MSG_ERR: level = LIBVLC_ERROR; break;
case VLC_MSG_WARN: level = LIBVLC_WARNING; break;
case VLC_MSG_DBG: level = LIBVLC_DEBUG; break;
}
inst->log.cb (inst->log.data, level, item, fmt, ap);
}
static const struct vlc_logger_operations libvlc_log_ops = {
libvlc_logf, NULL
};
void libvlc_log_unset (libvlc_instance_t *inst)
{
vlc_LogSet (inst->p_libvlc_int, NULL, NULL);
}
void libvlc_log_set (libvlc_instance_t *inst, libvlc_log_cb cb, void *data)
{
libvlc_log_unset (inst); 
inst->log.cb = cb;
inst->log.data = data;
vlc_LogSet(inst->p_libvlc_int, &libvlc_log_ops, inst);
}
static void libvlc_log_file (void *data, int level, const libvlc_log_t *log,
const char *fmt, va_list ap)
{
FILE *stream = data;
flockfile (stream);
vfprintf (stream, fmt, ap);
fputc ('\n', stream);
funlockfile (stream);
(void) level; (void) log;
}
void libvlc_log_set_file (libvlc_instance_t *inst, FILE *stream)
{
libvlc_log_set (inst, libvlc_log_file, stream);
}
