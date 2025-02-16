


#include "cache.h"
#include "config.h"

static int memory_limit_check(size_t size, int gentle)
{
static size_t limit = 0;
if (!limit) {
limit = git_env_ulong("GIT_ALLOC_LIMIT", 0);
if (!limit)
limit = SIZE_MAX;
}
if (size > limit) {
if (gentle) {
error("attempting to allocate %"PRIuMAX" over limit %"PRIuMAX,
(uintmax_t)size, (uintmax_t)limit);
return -1;
} else
die("attempting to allocate %"PRIuMAX" over limit %"PRIuMAX,
(uintmax_t)size, (uintmax_t)limit);
}
return 0;
}

char *xstrdup(const char *str)
{
char *ret = strdup(str);
if (!ret)
die("Out of memory, strdup failed");
return ret;
}

static void *do_xmalloc(size_t size, int gentle)
{
void *ret;

if (memory_limit_check(size, gentle))
return NULL;
ret = malloc(size);
if (!ret && !size)
ret = malloc(1);
if (!ret) {
if (!gentle)
die("Out of memory, malloc failed (tried to allocate %lu bytes)",
(unsigned long)size);
else {
error("Out of memory, malloc failed (tried to allocate %lu bytes)",
(unsigned long)size);
return NULL;
}
}
#if defined(XMALLOC_POISON)
memset(ret, 0xA5, size);
#endif
return ret;
}

void *xmalloc(size_t size)
{
return do_xmalloc(size, 0);
}

static void *do_xmallocz(size_t size, int gentle)
{
void *ret;
if (unsigned_add_overflows(size, 1)) {
if (gentle) {
error("Data too large to fit into virtual memory space.");
return NULL;
} else
die("Data too large to fit into virtual memory space.");
}
ret = do_xmalloc(size + 1, gentle);
if (ret)
((char*)ret)[size] = 0;
return ret;
}

void *xmallocz(size_t size)
{
return do_xmallocz(size, 0);
}

void *xmallocz_gently(size_t size)
{
return do_xmallocz(size, 1);
}







void *xmemdupz(const void *data, size_t len)
{
return memcpy(xmallocz(len), data, len);
}

char *xstrndup(const char *str, size_t len)
{
char *p = memchr(str, '\0', len);
return xmemdupz(str, p ? p - str : len);
}

void *xrealloc(void *ptr, size_t size)
{
void *ret;

memory_limit_check(size, 0);
ret = realloc(ptr, size);
if (!ret && !size)
ret = realloc(ptr, 1);
if (!ret)
die("Out of memory, realloc failed");
return ret;
}

void *xcalloc(size_t nmemb, size_t size)
{
void *ret;

if (unsigned_mult_overflows(nmemb, size))
die("data too large to fit into virtual memory space");

memory_limit_check(size * nmemb, 0);
ret = calloc(nmemb, size);
if (!ret && (!nmemb || !size))
ret = calloc(1, 1);
if (!ret)
die("Out of memory, calloc failed");
return ret;
}














#if !defined(MAX_IO_SIZE)
#define MAX_IO_SIZE_DEFAULT (8*1024*1024)
#if defined(SSIZE_MAX) && (SSIZE_MAX < MAX_IO_SIZE_DEFAULT)
#define MAX_IO_SIZE SSIZE_MAX
#else
#define MAX_IO_SIZE MAX_IO_SIZE_DEFAULT
#endif
#endif




int xopen(const char *path, int oflag, ...)
{
mode_t mode = 0;
va_list ap;







va_start(ap, oflag);
if (oflag & O_CREAT)
mode = va_arg(ap, int);
va_end(ap);

for (;;) {
int fd = open(path, oflag, mode);
if (fd >= 0)
return fd;
if (errno == EINTR)
continue;

if ((oflag & O_RDWR) == O_RDWR)
die_errno(_("could not open '%s' for reading and writing"), path);
else if ((oflag & O_WRONLY) == O_WRONLY)
die_errno(_("could not open '%s' for writing"), path);
else
die_errno(_("could not open '%s' for reading"), path);
}
}

static int handle_nonblock(int fd, short poll_events, int err)
{
struct pollfd pfd;

if (err != EAGAIN && err != EWOULDBLOCK)
return 0;

pfd.fd = fd;
pfd.events = poll_events;





poll(&pfd, 1, -1);
return 1;
}






ssize_t xread(int fd, void *buf, size_t len)
{
ssize_t nr;
if (len > MAX_IO_SIZE)
len = MAX_IO_SIZE;
while (1) {
nr = read(fd, buf, len);
if (nr < 0) {
if (errno == EINTR)
continue;
if (handle_nonblock(fd, POLLIN, errno))
continue;
}
return nr;
}
}






ssize_t xwrite(int fd, const void *buf, size_t len)
{
ssize_t nr;
if (len > MAX_IO_SIZE)
len = MAX_IO_SIZE;
while (1) {
nr = write(fd, buf, len);
if (nr < 0) {
if (errno == EINTR)
continue;
if (handle_nonblock(fd, POLLOUT, errno))
continue;
}

return nr;
}
}






ssize_t xpread(int fd, void *buf, size_t len, off_t offset)
{
ssize_t nr;
if (len > MAX_IO_SIZE)
len = MAX_IO_SIZE;
while (1) {
nr = pread(fd, buf, len, offset);
if ((nr < 0) && (errno == EAGAIN || errno == EINTR))
continue;
return nr;
}
}

ssize_t read_in_full(int fd, void *buf, size_t count)
{
char *p = buf;
ssize_t total = 0;

while (count > 0) {
ssize_t loaded = xread(fd, p, count);
if (loaded < 0)
return -1;
if (loaded == 0)
return total;
count -= loaded;
p += loaded;
total += loaded;
}

return total;
}

ssize_t write_in_full(int fd, const void *buf, size_t count)
{
const char *p = buf;
ssize_t total = 0;

while (count > 0) {
ssize_t written = xwrite(fd, p, count);
if (written < 0)
return -1;
if (!written) {
errno = ENOSPC;
return -1;
}
count -= written;
p += written;
total += written;
}

return total;
}

ssize_t pread_in_full(int fd, void *buf, size_t count, off_t offset)
{
char *p = buf;
ssize_t total = 0;

while (count > 0) {
ssize_t loaded = xpread(fd, p, count, offset);
if (loaded < 0)
return -1;
if (loaded == 0)
return total;
count -= loaded;
p += loaded;
total += loaded;
offset += loaded;
}

return total;
}

int xdup(int fd)
{
int ret = dup(fd);
if (ret < 0)
die_errno("dup failed");
return ret;
}




FILE *xfopen(const char *path, const char *mode)
{
for (;;) {
FILE *fp = fopen(path, mode);
if (fp)
return fp;
if (errno == EINTR)
continue;

if (*mode && mode[1] == '+')
die_errno(_("could not open '%s' for reading and writing"), path);
else if (*mode == 'w' || *mode == 'a')
die_errno(_("could not open '%s' for writing"), path);
else
die_errno(_("could not open '%s' for reading"), path);
}
}

FILE *xfdopen(int fd, const char *mode)
{
FILE *stream = fdopen(fd, mode);
if (stream == NULL)
die_errno("Out of memory? fdopen failed");
return stream;
}

FILE *fopen_for_writing(const char *path)
{
FILE *ret = fopen(path, "w");

if (!ret && errno == EPERM) {
if (!unlink(path))
ret = fopen(path, "w");
else
errno = EPERM;
}
return ret;
}

static void warn_on_inaccessible(const char *path)
{
warning_errno(_("unable to access '%s'"), path);
}

int warn_on_fopen_errors(const char *path)
{
if (errno != ENOENT && errno != ENOTDIR) {
warn_on_inaccessible(path);
return -1;
}

return 0;
}

FILE *fopen_or_warn(const char *path, const char *mode)
{
FILE *fp = fopen(path, mode);

if (fp)
return fp;

warn_on_fopen_errors(path);
return NULL;
}

int xmkstemp(char *filename_template)
{
int fd;
char origtemplate[PATH_MAX];
strlcpy(origtemplate, filename_template, sizeof(origtemplate));

fd = mkstemp(filename_template);
if (fd < 0) {
int saved_errno = errno;
const char *nonrelative_template;

if (strlen(filename_template) != strlen(origtemplate))
filename_template = origtemplate;

nonrelative_template = absolute_path(filename_template);
errno = saved_errno;
die_errno("Unable to create temporary file '%s'",
nonrelative_template);
}
return fd;
}



#undef TMP_MAX
#define TMP_MAX 16384

int git_mkstemps_mode(char *pattern, int suffix_len, int mode)
{
static const char letters[] =
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789";
static const int num_letters = ARRAY_SIZE(letters) - 1;
static const char x_pattern[] = "XXXXXX";
static const int num_x = ARRAY_SIZE(x_pattern) - 1;
uint64_t value;
struct timeval tv;
char *filename_template;
size_t len;
int fd, count;

len = strlen(pattern);

if (len < num_x + suffix_len) {
errno = EINVAL;
return -1;
}

if (strncmp(&pattern[len - num_x - suffix_len], x_pattern, num_x)) {
errno = EINVAL;
return -1;
}





gettimeofday(&tv, NULL);
value = ((uint64_t)tv.tv_usec << 16) ^ tv.tv_sec ^ getpid();
filename_template = &pattern[len - num_x - suffix_len];
for (count = 0; count < TMP_MAX; ++count) {
uint64_t v = value;
int i;

for (i = 0; i < num_x; i++) {
filename_template[i] = letters[v % num_letters];
v /= num_letters;
}

fd = open(pattern, O_CREAT | O_EXCL | O_RDWR, mode);
if (fd >= 0)
return fd;




if (errno != EEXIST)
break;





value += 7777;
}

pattern[0] = '\0';
return -1;
}

int git_mkstemp_mode(char *pattern, int mode)
{

return git_mkstemps_mode(pattern, 0, mode);
}

int xmkstemp_mode(char *filename_template, int mode)
{
int fd;
char origtemplate[PATH_MAX];
strlcpy(origtemplate, filename_template, sizeof(origtemplate));

fd = git_mkstemp_mode(filename_template, mode);
if (fd < 0) {
int saved_errno = errno;
const char *nonrelative_template;

if (!filename_template[0])
filename_template = origtemplate;

nonrelative_template = absolute_path(filename_template);
errno = saved_errno;
die_errno("Unable to create temporary file '%s'",
nonrelative_template);
}
return fd;
}

static int warn_if_unremovable(const char *op, const char *file, int rc)
{
int err;
if (!rc || errno == ENOENT)
return 0;
err = errno;
warning_errno("unable to %s '%s'", op, file);
errno = err;
return rc;
}

int unlink_or_msg(const char *file, struct strbuf *err)
{
int rc = unlink(file);

assert(err);

if (!rc || errno == ENOENT)
return 0;

strbuf_addf(err, "unable to unlink '%s': %s",
file, strerror(errno));
return -1;
}

int unlink_or_warn(const char *file)
{
return warn_if_unremovable("unlink", file, unlink(file));
}

int rmdir_or_warn(const char *file)
{
return warn_if_unremovable("rmdir", file, rmdir(file));
}

int remove_or_warn(unsigned int mode, const char *file)
{
return S_ISGITLINK(mode) ? rmdir_or_warn(file) : unlink_or_warn(file);
}

static int access_error_is_ok(int err, unsigned flag)
{
return (is_missing_file_error(err) ||
((flag & ACCESS_EACCES_OK) && err == EACCES));
}

int access_or_warn(const char *path, int mode, unsigned flag)
{
int ret = access(path, mode);
if (ret && !access_error_is_ok(errno, flag))
warn_on_inaccessible(path);
return ret;
}

int access_or_die(const char *path, int mode, unsigned flag)
{
int ret = access(path, mode);
if (ret && !access_error_is_ok(errno, flag))
die_errno(_("unable to access '%s'"), path);
return ret;
}

char *xgetcwd(void)
{
struct strbuf sb = STRBUF_INIT;
if (strbuf_getcwd(&sb))
die_errno(_("unable to get current working directory"));
return strbuf_detach(&sb, NULL);
}

int xsnprintf(char *dst, size_t max, const char *fmt, ...)
{
va_list ap;
int len;

va_start(ap, fmt);
len = vsnprintf(dst, max, fmt, ap);
va_end(ap);

if (len < 0)
BUG("your snprintf is broken");
if (len >= max)
BUG("attempt to snprintf into too-small buffer");
return len;
}

void write_file_buf(const char *path, const char *buf, size_t len)
{
int fd = xopen(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
if (write_in_full(fd, buf, len) < 0)
die_errno(_("could not write to '%s'"), path);
if (close(fd))
die_errno(_("could not close '%s'"), path);
}

void write_file(const char *path, const char *fmt, ...)
{
va_list params;
struct strbuf sb = STRBUF_INIT;

va_start(params, fmt);
strbuf_vaddf(&sb, fmt, params);
va_end(params);

strbuf_complete_line(&sb);

write_file_buf(path, sb.buf, sb.len);
strbuf_release(&sb);
}

void sleep_millisec(int millisec)
{
poll(NULL, 0, millisec);
}

int xgethostname(char *buf, size_t len)
{





int ret = gethostname(buf, len);
if (!ret)
buf[len - 1] = 0;
return ret;
}

int is_empty_or_missing_file(const char *filename)
{
struct stat st;

if (stat(filename, &st) < 0) {
if (errno == ENOENT)
return 1;
die_errno(_("could not stat %s"), filename);
}

return !st.st_size;
}
