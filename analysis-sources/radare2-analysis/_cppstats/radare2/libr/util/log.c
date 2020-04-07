#define LOG_CONFIGSTR_SIZE 512
#define LOG_OUTPUTBUF_SIZE 512
#include <r_core.h>
#include <stdarg.h>
static RList *log_cbs = NULL; 
static int cfg_loglvl = R_LOGLVL_ERROR; 
static int cfg_logtraplvl = R_LOGLVL_FATAL; 
static bool cfg_logsrcinfo = false; 
static bool cfg_logcolors = false; 
static char cfg_logfile[LOG_CONFIGSTR_SIZE] = ""; 
static const char *level_tags[] = { 
[R_LOGLVL_SILLY] = "SILLY",
[R_LOGLVL_VERBOSE] = "VERBOSE",
[R_LOGLVL_DEBUG] = "DEBUG",
[R_LOGLVL_INFO] = "INFO",
[R_LOGLVL_WARN] = "WARNING",
[R_LOGLVL_ERROR] = "ERROR",
[R_LOGLVL_FATAL] = "FATAL"
};
R_API void r_log_set_level(RLogLevel level) {
cfg_loglvl = level;
}
R_API void r_log_set_traplevel(RLogLevel level) {
cfg_logtraplvl = level;
}
R_API void r_log_set_file(const char *filename) {
int value_len = r_str_nlen (filename, LOG_CONFIGSTR_SIZE) + 1;
strncpy (cfg_logfile, filename, value_len);
}
R_API void r_log_set_srcinfo(bool show_info) {
cfg_logsrcinfo = show_info;
}
R_API void r_log_set_colors(bool show_info) {
cfg_logcolors = show_info;
}
R_API void r_log_add_callback(RLogCallback cbfunc) {
if (!log_cbs) {
log_cbs = r_list_new ();
}
if (!r_list_contains (log_cbs, cbfunc)) {
r_list_append (log_cbs, cbfunc);
}
}
R_API void r_log_del_callback(RLogCallback cbfunc) {
if (log_cbs) {
r_list_delete_data (log_cbs, cbfunc);
}
}
R_API void r_vlog(const char *funcname, const char *filename,
ut32 lineno, RLogLevel level, const char *tag, const char *fmtstr, va_list args) {
va_list args_copy;
va_copy (args_copy, args);
if (level < cfg_loglvl && level < cfg_logtraplvl) {
va_end (args_copy);
return;
}
char output_buf[LOG_OUTPUTBUF_SIZE] = ""; 
if (!tag) {
tag = R_BETWEEN (0, level, R_ARRAY_SIZE (level_tags) - 1)? level_tags[level]: "";
}
int offset = snprintf (output_buf, LOG_OUTPUTBUF_SIZE, "%s: ", tag);
if (cfg_logsrcinfo) {
offset += snprintf (output_buf + offset, LOG_OUTPUTBUF_SIZE - offset, "%s in %s:%i: ", funcname, filename, lineno);
}
vsnprintf (output_buf + offset, LOG_OUTPUTBUF_SIZE - offset, fmtstr, args);
if (log_cbs && r_list_length (log_cbs) > 0) {
RListIter *it;
RLogCallback cb;
r_list_foreach (log_cbs, it, cb) {
cb (output_buf, funcname, filename, lineno, level, NULL, fmtstr, args_copy);
}
} else {
fprintf (stderr, "%s", output_buf);
}
va_end (args_copy);
if (cfg_logfile[0] != 0x00) {
FILE *file = r_sandbox_fopen (cfg_logfile, "a+"); 
if (!file) {
file = r_sandbox_fopen (cfg_logfile, "w+");
}
if (file) {
fprintf (file, "%s", output_buf);
fclose (file);
} else {
eprintf ("%s failed to write to file: %s\n", MACRO_LOG_FUNC, cfg_logfile);
}
}
if (level >= cfg_logtraplvl && level != R_LOGLVL_NONE) {
fflush (stdout); 
fflush (stderr);
r_sys_breakpoint (); 
}
}
R_API void r_log(const char *funcname, const char *filename,
ut32 lineno, RLogLevel level, const char *tag, const char *fmtstr, ...) {
va_list args;
va_start (args, fmtstr);
r_vlog (funcname, filename, lineno, level, tag, fmtstr, args);
va_end (args);
}
