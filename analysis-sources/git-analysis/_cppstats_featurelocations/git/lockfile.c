



#include "cache.h"
#include "lockfile.h"








static void trim_last_path_component(struct strbuf *path)
{
int i = path->len;


while (i && path->buf[i - 1] == '/')
i--;





while (i && path->buf[i - 1] != '/')
i--;

strbuf_setlen(path, i);
}



#define MAXDEPTH 5












static void resolve_symlink(struct strbuf *path)
{
int depth = MAXDEPTH;
static struct strbuf link = STRBUF_INIT;

while (depth--) {
if (strbuf_readlink(&link, path->buf, path->len) < 0)
break;

if (is_absolute_path(link.buf))

strbuf_reset(path);
else




trim_last_path_component(path);

strbuf_addbuf(path, &link);
}
strbuf_reset(&link);
}


static int lock_file(struct lock_file *lk, const char *path, int flags)
{
struct strbuf filename = STRBUF_INIT;

strbuf_addstr(&filename, path);
if (!(flags & LOCK_NO_DEREF))
resolve_symlink(&filename);

strbuf_addstr(&filename, LOCK_SUFFIX);
lk->tempfile = create_tempfile(filename.buf);
strbuf_release(&filename);
return lk->tempfile ? lk->tempfile->fd : -1;
}







#define INITIAL_BACKOFF_MS 1L
#define BACKOFF_MAX_MULTIPLIER 1000






static int lock_file_timeout(struct lock_file *lk, const char *path,
int flags, long timeout_ms)
{
int n = 1;
int multiplier = 1;
long remaining_ms = 0;
static int random_initialized = 0;

if (timeout_ms == 0)
return lock_file(lk, path, flags);

if (!random_initialized) {
srand((unsigned int)getpid());
random_initialized = 1;
}

if (timeout_ms > 0)
remaining_ms = timeout_ms;

while (1) {
long backoff_ms, wait_ms;
int fd;

fd = lock_file(lk, path, flags);

if (fd >= 0)
return fd; 
else if (errno != EEXIST)
return -1; 
else if (timeout_ms > 0 && remaining_ms <= 0)
return -1; 

backoff_ms = multiplier * INITIAL_BACKOFF_MS;

wait_ms = (750 + rand() % 500) * backoff_ms / 1000;
sleep_millisec(wait_ms);
remaining_ms -= wait_ms;


multiplier += 2*n + 1;
if (multiplier > BACKOFF_MAX_MULTIPLIER)
multiplier = BACKOFF_MAX_MULTIPLIER;
else
n++;
}
}

void unable_to_lock_message(const char *path, int err, struct strbuf *buf)
{
if (err == EEXIST) {
strbuf_addf(buf, _("Unable to create '%s.lock': %s.\n\n"
"Another git process seems to be running in this repository, e.g.\n"
"an editor opened by 'git commit'. Please make sure all processes\n"
"are terminated then try again. If it still fails, a git process\n"
"may have crashed in this repository earlier:\n"
"remove the file manually to continue."),
absolute_path(path), strerror(err));
} else
strbuf_addf(buf, _("Unable to create '%s.lock': %s"),
absolute_path(path), strerror(err));
}

NORETURN void unable_to_lock_die(const char *path, int err)
{
struct strbuf buf = STRBUF_INIT;

unable_to_lock_message(path, err, &buf);
die("%s", buf.buf);
}


int hold_lock_file_for_update_timeout(struct lock_file *lk, const char *path,
int flags, long timeout_ms)
{
int fd = lock_file_timeout(lk, path, flags, timeout_ms);
if (fd < 0) {
if (flags & LOCK_DIE_ON_ERROR)
unable_to_lock_die(path, errno);
if (flags & LOCK_REPORT_ON_ERROR) {
struct strbuf buf = STRBUF_INIT;
unable_to_lock_message(path, errno, &buf);
error("%s", buf.buf);
strbuf_release(&buf);
}
}
return fd;
}

char *get_locked_file_path(struct lock_file *lk)
{
struct strbuf ret = STRBUF_INIT;

strbuf_addstr(&ret, get_tempfile_path(lk->tempfile));
if (ret.len <= LOCK_SUFFIX_LEN ||
strcmp(ret.buf + ret.len - LOCK_SUFFIX_LEN, LOCK_SUFFIX))
BUG("get_locked_file_path() called for malformed lock object");

strbuf_setlen(&ret, ret.len - LOCK_SUFFIX_LEN);
return strbuf_detach(&ret, NULL);
}

int commit_lock_file(struct lock_file *lk)
{
char *result_path = get_locked_file_path(lk);

if (commit_lock_file_to(lk, result_path)) {
int save_errno = errno;
free(result_path);
errno = save_errno;
return -1;
}
free(result_path);
return 0;
}
