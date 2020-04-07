









#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#if !defined(WIN32)
#include <sys/time.h> 
#endif
#include <uv.h>

#include "auto/config.h"
#include "nvim/log.h"
#include "nvim/types.h"
#include "nvim/os/os.h"
#include "nvim/os/time.h"

#define LOG_FILE_ENV "NVIM_LOG_FILE"


static char log_file_path[MAXPATHL + 1] = { 0 };

static uv_mutex_t mutex;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "log.c.generated.h"
#endif

#if defined(HAVE_EXECINFO_BACKTRACE)
#include <execinfo.h>
#endif

static bool log_try_create(char *fname)
{
if (fname == NULL || fname[0] == '\0') {
return false;
}
FILE *log_file = fopen(fname, "a");
if (log_file == NULL) {
return false;
}
fclose(log_file);
return true;
}









static bool log_path_init(void)
{
if (log_file_path[0]) {
return true;
}
size_t size = sizeof(log_file_path);
expand_env((char_u *)"$" LOG_FILE_ENV, (char_u *)log_file_path,
(int)size - 1);
if (strequal("$" LOG_FILE_ENV, log_file_path)
|| log_file_path[0] == '\0'
|| os_isdir((char_u *)log_file_path)
|| !log_try_create(log_file_path)) {

char *defaultpath = stdpaths_user_data_subpath("log", 0, true);
size_t len = xstrlcpy(log_file_path, defaultpath, size);
xfree(defaultpath);

if (len >= size || !log_try_create(log_file_path)) {
len = xstrlcpy(log_file_path, ".nvimlog", size);
}

if (len >= size || !log_try_create(log_file_path)) {
log_file_path[0] = '\0';
return false;
}
os_setenv(LOG_FILE_ENV, log_file_path, true);
}
return true;
}

void log_init(void)
{
uv_mutex_init(&mutex);
log_path_init();
}

void log_lock(void)
{
uv_mutex_lock(&mutex);
}

void log_unlock(void)
{
uv_mutex_unlock(&mutex);
}









bool logmsg(int log_level, const char *context, const char *func_name,
int line_num, bool eol, const char *fmt, ...)
FUNC_ATTR_UNUSED FUNC_ATTR_PRINTF(6, 7)
{
if (log_level < MIN_LOG_LEVEL) {
return false;
}

#if defined(EXITFREE)


assert(!entered_free_all_mem);
#endif

log_lock();
bool ret = false;
FILE *log_file = open_log_file();

if (log_file == NULL) {
goto end;
}

va_list args;
va_start(args, fmt);
ret = v_do_log_to_file(log_file, log_level, context, func_name, line_num,
eol, fmt, args);
va_end(args);

if (log_file != stderr && log_file != stdout) {
fclose(log_file);
}
end:
log_unlock();
return ret;
}

void log_uv_handles(void *loop)
{
uv_loop_t *l = loop;
log_lock();
FILE *log_file = open_log_file();

if (log_file == NULL) {
goto end;
}

uv_print_all_handles(l, log_file);

if (log_file != stderr && log_file != stdout) {
fclose(log_file);
}
end:
log_unlock();
}




FILE *open_log_file(void)
{
static bool opening_log_file = false;


if (opening_log_file) {
do_log_to_file(stderr, ERROR_LOG_LEVEL, NULL, __func__, __LINE__, true,
"Cannot LOG() recursively.");
return stderr;
}

FILE *log_file = NULL;
opening_log_file = true;
if (log_path_init()) {
log_file = fopen(log_file_path, "a");
}
opening_log_file = false;

if (log_file != NULL) {
return log_file;
}





do_log_to_file(stderr, ERROR_LOG_LEVEL, NULL, __func__, __LINE__, true,
"Logging to stderr, failed to open $" LOG_FILE_ENV ": %s",
log_file_path);
return stderr;
}

#if defined(HAVE_EXECINFO_BACKTRACE)
void log_callstack_to_file(FILE *log_file, const char *const func_name,
const int line_num)
{
void *trace[100];
int trace_size = backtrace(trace, ARRAY_SIZE(trace));

char exepath[MAXPATHL] = { 0 };
size_t exepathlen = MAXPATHL;
if (os_exepath(exepath, &exepathlen) != 0) {
abort();
}
assert(24 + exepathlen < IOSIZE); 

char cmdbuf[IOSIZE + (20 * ARRAY_SIZE(trace)) + MAXPATHL];
snprintf(cmdbuf, sizeof(cmdbuf), "addr2line -e %s -f -p", exepath);
for (int i = 1; i < trace_size; i++) {
char buf[20]; 
snprintf(buf, sizeof(buf), " %p", trace[i]);
xstrlcat(cmdbuf, buf, sizeof(cmdbuf));
}



do_log_to_file(log_file, DEBUG_LOG_LEVEL, NULL, func_name, line_num, true,
"trace:");
FILE *fp = popen(cmdbuf, "r");
char linebuf[IOSIZE];
while (fgets(linebuf, sizeof(linebuf) - 1, fp) != NULL) {
fprintf(log_file, " %s", linebuf);
}
pclose(fp);

if (log_file != stderr && log_file != stdout) {
fclose(log_file);
}
}

void log_callstack(const char *const func_name, const int line_num)
{
log_lock();
FILE *log_file = open_log_file();
if (log_file == NULL) {
goto end;
}

log_callstack_to_file(log_file, func_name, line_num);

end:
log_unlock();
}
#endif

static bool do_log_to_file(FILE *log_file, int log_level, const char *context,
const char *func_name, int line_num, bool eol,
const char *fmt, ...)
FUNC_ATTR_PRINTF(7, 8)
{
va_list args;
va_start(args, fmt);
bool ret = v_do_log_to_file(log_file, log_level, context, func_name,
line_num, eol, fmt, args);
va_end(args);

return ret;
}

static bool v_do_log_to_file(FILE *log_file, int log_level,
const char *context, const char *func_name,
int line_num, bool eol, const char *fmt,
va_list args)
{
static const char *log_levels[] = {
[DEBUG_LOG_LEVEL] = "DEBUG",
[INFO_LOG_LEVEL] = "INFO ",
[WARN_LOG_LEVEL] = "WARN ",
[ERROR_LOG_LEVEL] = "ERROR",
};
assert(log_level >= DEBUG_LOG_LEVEL && log_level <= ERROR_LOG_LEVEL);


struct tm local_time;
if (os_localtime(&local_time) == NULL) {
return false;
}
char date_time[20];
if (strftime(date_time, sizeof(date_time), "%Y-%m-%dT%H:%M:%S",
&local_time) == 0) {
return false;
}

int millis = 0;
#if !defined(WIN32)
struct timeval curtime;
if (gettimeofday(&curtime, NULL) == 0) {
millis = (int)curtime.tv_usec / 1000;
}
#endif


int64_t pid = os_get_pid();
int rv = (line_num == -1 || func_name == NULL)
? fprintf(log_file, "%s %s.%03d %-5" PRId64 " %s",
log_levels[log_level], date_time, millis, pid,
(context == NULL ? "?:" : context))
: fprintf(log_file, "%s %s.%03d %-5" PRId64 " %s%s:%d: ",
log_levels[log_level], date_time, millis, pid,
(context == NULL ? "" : context),
func_name, line_num);
if (rv < 0) {
return false;
}
if (vfprintf(log_file, fmt, args) < 0) {
return false;
}
if (eol) {
fputc('\n', log_file);
}
if (fflush(log_file) == EOF) {
return false;
}

return true;
}

