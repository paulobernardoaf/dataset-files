#include <stdarg.h>
enum vlc_log_type
{
VLC_MSG_INFO=0, 
VLC_MSG_ERR, 
VLC_MSG_WARN, 
VLC_MSG_DBG, 
};
typedef struct vlc_log_t
{
uintptr_t i_object_id; 
const char *psz_object_type; 
const char *psz_module; 
const char *psz_header; 
const char *file; 
int line; 
const char *func; 
unsigned long tid; 
} vlc_log_t;
VLC_API void vlc_object_Log(vlc_object_t *obj, int prio, const char *module,
const char *file, unsigned line, const char *func,
const char *format, ...) VLC_FORMAT(7, 8);
VLC_API void vlc_object_vaLog(vlc_object_t *obj, int prio, const char *module,
const char *file, unsigned line, const char *func,
const char *format, va_list ap);
#define msg_GenericVa(o, p, fmt, ap) vlc_object_vaLog(VLC_OBJECT(o), p, vlc_module_name, __FILE__, __LINE__, __func__, fmt, ap)
#define msg_Generic(o, p, ...) vlc_object_Log(VLC_OBJECT(o), p, vlc_module_name, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define msg_Info(p_this, ...) msg_Generic(p_this, VLC_MSG_INFO, __VA_ARGS__)
#define msg_Err(p_this, ...) msg_Generic(p_this, VLC_MSG_ERR, __VA_ARGS__)
#define msg_Warn(p_this, ...) msg_Generic(p_this, VLC_MSG_WARN, __VA_ARGS__)
#define msg_Dbg(p_this, ...) msg_Generic(p_this, VLC_MSG_DBG, __VA_ARGS__)
extern const char vlc_module_name[];
VLC_API const char *vlc_strerror(int);
VLC_API const char *vlc_strerror_c(int);
struct vlc_logger;
VLC_API void vlc_Log(struct vlc_logger *const *logger, int prio,
const char *type, const char *module,
const char *file, unsigned line, const char *func,
const char *format, ...) VLC_FORMAT(8, 9);
VLC_API void vlc_vaLog(struct vlc_logger *const *logger, int prio,
const char *type, const char *module,
const char *file, unsigned line, const char *func,
const char *format, va_list ap);
#define vlc_log_gen(logger, prio, ...) vlc_Log(&(logger), prio, "generic", vlc_module_name, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define vlc_info(logger, ...) vlc_log_gen(logger, VLC_MSG_INFO, __VA_ARGS__)
#define vlc_error(logger, ...) vlc_log_gen(logger, VLC_MSG_ERR, __VA_ARGS__)
#define vlc_warning(logger, ...) vlc_log_gen(logger, VLC_MSG_WARN, __VA_ARGS__)
#define vlc_debug(logger, ...) vlc_log_gen(logger, VLC_MSG_DBG, __VA_ARGS__)
typedef void (*vlc_log_cb) (void *data, int type, const vlc_log_t *item,
const char *fmt, va_list args);
struct vlc_logger_operations
{
vlc_log_cb log;
void (*destroy)(void *data);
};
VLC_API struct vlc_logger *vlc_LogHeaderCreate(struct vlc_logger *parent,
const char *str) VLC_USED;
VLC_API void vlc_LogDestroy(struct vlc_logger *);
