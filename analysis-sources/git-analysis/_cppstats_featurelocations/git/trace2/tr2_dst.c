#include "cache.h"
#include "trace2/tr2_dst.h"
#include "trace2/tr2_sid.h"
#include "trace2/tr2_sysenv.h"




#define MAX_AUTO_ATTEMPTS 10





#define DISCARD_SENTINEL_NAME "git-trace2-discard"






static int tr2env_max_files = 0;

static int tr2_dst_want_warning(void)
{
static int tr2env_dst_debug = -1;

if (tr2env_dst_debug == -1) {
const char *env_value = tr2_sysenv_get(TR2_SYSENV_DST_DEBUG);
if (!env_value || !*env_value)
tr2env_dst_debug = 0;
else
tr2env_dst_debug = atoi(env_value) > 0;
}

return tr2env_dst_debug;
}

void tr2_dst_trace_disable(struct tr2_dst *dst)
{
if (dst->need_close)
close(dst->fd);
dst->fd = 0;
dst->initialized = 1;
dst->need_close = 0;
}
















static int tr2_dst_too_many_files(struct tr2_dst *dst, const char *tgt_prefix)
{
int file_count = 0, max_files = 0, ret = 0;
const char *max_files_var;
DIR *dirp;
struct strbuf path = STRBUF_INIT, sentinel_path = STRBUF_INIT;
struct stat statbuf;


max_files_var = tr2_sysenv_get(TR2_SYSENV_MAX_FILES);
if (max_files_var && *max_files_var && ((max_files = atoi(max_files_var)) >= 0))
tr2env_max_files = max_files;

if (!tr2env_max_files) {
ret = 0;
goto cleanup;
}

strbuf_addstr(&path, tgt_prefix);
if (!is_dir_sep(path.buf[path.len - 1])) {
strbuf_addch(&path, '/');
}


strbuf_addbuf(&sentinel_path, &path);
strbuf_addstr(&sentinel_path, DISCARD_SENTINEL_NAME);
if (!stat(sentinel_path.buf, &statbuf)) {
ret = 1;
goto cleanup;
}


dirp = opendir(path.buf);
while (file_count < tr2env_max_files && dirp && readdir(dirp))
file_count++;
if (dirp)
closedir(dirp);

if (file_count >= tr2env_max_files) {
dst->too_many_files = 1;
dst->fd = open(sentinel_path.buf, O_WRONLY | O_CREAT | O_EXCL, 0666);
ret = -1;
goto cleanup;
}

cleanup:
strbuf_release(&path);
strbuf_release(&sentinel_path);
return ret;
}

static int tr2_dst_try_auto_path(struct tr2_dst *dst, const char *tgt_prefix)
{
int too_many_files;
const char *last_slash, *sid = tr2_sid_get();
struct strbuf path = STRBUF_INIT;
size_t base_path_len;
unsigned attempt_count;

last_slash = strrchr(sid, '/');
if (last_slash)
sid = last_slash + 1;

strbuf_addstr(&path, tgt_prefix);
if (!is_dir_sep(path.buf[path.len - 1]))
strbuf_addch(&path, '/');
strbuf_addstr(&path, sid);
base_path_len = path.len;

too_many_files = tr2_dst_too_many_files(dst, tgt_prefix);
if (!too_many_files) {
for (attempt_count = 0; attempt_count < MAX_AUTO_ATTEMPTS; attempt_count++) {
if (attempt_count > 0) {
strbuf_setlen(&path, base_path_len);
strbuf_addf(&path, ".%d", attempt_count);
}

dst->fd = open(path.buf, O_WRONLY | O_CREAT | O_EXCL, 0666);
if (dst->fd != -1)
break;
}
} else if (too_many_files == 1) {
strbuf_release(&path);
if (tr2_dst_want_warning())
warning("trace2: not opening %s trace file due to too "
"many files in target directory %s",
tr2_sysenv_display_name(dst->sysenv_var),
tgt_prefix);
return 0;
}

if (dst->fd == -1) {
if (tr2_dst_want_warning())
warning("trace2: could not open '%.*s' for '%s' tracing: %s",
(int) base_path_len, path.buf,
tr2_sysenv_display_name(dst->sysenv_var),
strerror(errno));

tr2_dst_trace_disable(dst);
strbuf_release(&path);
return 0;
}

strbuf_release(&path);

dst->need_close = 1;
dst->initialized = 1;

return dst->fd;
}

static int tr2_dst_try_path(struct tr2_dst *dst, const char *tgt_value)
{
int fd = open(tgt_value, O_WRONLY | O_APPEND | O_CREAT, 0666);
if (fd == -1) {
if (tr2_dst_want_warning())
warning("trace2: could not open '%s' for '%s' tracing: %s",
tgt_value,
tr2_sysenv_display_name(dst->sysenv_var),
strerror(errno));

tr2_dst_trace_disable(dst);
return 0;
}

dst->fd = fd;
dst->need_close = 1;
dst->initialized = 1;

return dst->fd;
}

#if !defined(NO_UNIX_SOCKETS)
#define PREFIX_AF_UNIX "af_unix:"
#define PREFIX_AF_UNIX_STREAM "af_unix:stream:"
#define PREFIX_AF_UNIX_DGRAM "af_unix:dgram:"

static int tr2_dst_try_uds_connect(const char *path, int sock_type, int *out_fd)
{
int fd;
struct sockaddr_un sa;

fd = socket(AF_UNIX, sock_type, 0);
if (fd == -1)
return errno;

sa.sun_family = AF_UNIX;
strlcpy(sa.sun_path, path, sizeof(sa.sun_path));

if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
int e = errno;
close(fd);
return e;
}

*out_fd = fd;
return 0;
}

#define TR2_DST_UDS_TRY_STREAM (1 << 0)
#define TR2_DST_UDS_TRY_DGRAM (1 << 1)

static int tr2_dst_try_unix_domain_socket(struct tr2_dst *dst,
const char *tgt_value)
{
unsigned int uds_try = 0;
int fd;
int e;
const char *path = NULL;












if (skip_prefix(tgt_value, PREFIX_AF_UNIX_STREAM, &path))
uds_try |= TR2_DST_UDS_TRY_STREAM;

else if (skip_prefix(tgt_value, PREFIX_AF_UNIX_DGRAM, &path))
uds_try |= TR2_DST_UDS_TRY_DGRAM;

else if (skip_prefix(tgt_value, PREFIX_AF_UNIX, &path))
uds_try |= TR2_DST_UDS_TRY_STREAM | TR2_DST_UDS_TRY_DGRAM;

if (!path || !*path) {
if (tr2_dst_want_warning())
warning("trace2: invalid AF_UNIX value '%s' for '%s' tracing",
tgt_value,
tr2_sysenv_display_name(dst->sysenv_var));

tr2_dst_trace_disable(dst);
return 0;
}

if (!is_absolute_path(path) ||
strlen(path) >= sizeof(((struct sockaddr_un *)0)->sun_path)) {
if (tr2_dst_want_warning())
warning("trace2: invalid AF_UNIX path '%s' for '%s' tracing",
path, tr2_sysenv_display_name(dst->sysenv_var));

tr2_dst_trace_disable(dst);
return 0;
}

if (uds_try & TR2_DST_UDS_TRY_STREAM) {
e = tr2_dst_try_uds_connect(path, SOCK_STREAM, &fd);
if (!e)
goto connected;
if (e != EPROTOTYPE)
goto error;
}
if (uds_try & TR2_DST_UDS_TRY_DGRAM) {
e = tr2_dst_try_uds_connect(path, SOCK_DGRAM, &fd);
if (!e)
goto connected;
}

error:
if (tr2_dst_want_warning())
warning("trace2: could not connect to socket '%s' for '%s' tracing: %s",
path, tr2_sysenv_display_name(dst->sysenv_var),
strerror(e));

tr2_dst_trace_disable(dst);
return 0;

connected:
dst->fd = fd;
dst->need_close = 1;
dst->initialized = 1;

return dst->fd;
}
#endif

static void tr2_dst_malformed_warning(struct tr2_dst *dst,
const char *tgt_value)
{
warning("trace2: unknown value for '%s': '%s'",
tr2_sysenv_display_name(dst->sysenv_var), tgt_value);
}

int tr2_dst_get_trace_fd(struct tr2_dst *dst)
{
const char *tgt_value;


if (dst->initialized)
return dst->fd;

dst->initialized = 1;

tgt_value = tr2_sysenv_get(dst->sysenv_var);

if (!tgt_value || !strcmp(tgt_value, "") || !strcmp(tgt_value, "0") ||
!strcasecmp(tgt_value, "false")) {
dst->fd = 0;
return dst->fd;
}

if (!strcmp(tgt_value, "1") || !strcasecmp(tgt_value, "true")) {
dst->fd = STDERR_FILENO;
return dst->fd;
}

if (strlen(tgt_value) == 1 && isdigit(*tgt_value)) {
dst->fd = atoi(tgt_value);
return dst->fd;
}

if (is_absolute_path(tgt_value)) {
if (is_directory(tgt_value))
return tr2_dst_try_auto_path(dst, tgt_value);
else
return tr2_dst_try_path(dst, tgt_value);
}

#if !defined(NO_UNIX_SOCKETS)
if (starts_with(tgt_value, PREFIX_AF_UNIX))
return tr2_dst_try_unix_domain_socket(dst, tgt_value);
#endif


tr2_dst_malformed_warning(dst, tgt_value);
tr2_dst_trace_disable(dst);
return 0;
}

int tr2_dst_trace_want(struct tr2_dst *dst)
{
return !!tr2_dst_get_trace_fd(dst);
}

void tr2_dst_write_line(struct tr2_dst *dst, struct strbuf *buf_line)
{
int fd = tr2_dst_get_trace_fd(dst);

strbuf_complete_line(buf_line); 
















if (write(fd, buf_line->buf, buf_line->len) >= 0)
return;

if (tr2_dst_want_warning())
warning("unable to write trace to '%s': %s",
tr2_sysenv_display_name(dst->sysenv_var),
strerror(errno));
tr2_dst_trace_disable(dst);
}
