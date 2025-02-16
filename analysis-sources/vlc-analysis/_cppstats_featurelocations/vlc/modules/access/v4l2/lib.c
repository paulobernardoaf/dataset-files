



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <dlfcn.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <vlc_common.h>

#include "v4l2.h"

static int fd_open (int fd, int flags)
{
(void) flags;
return fd;
}

static void *v4l2_handle = NULL;

static int (*v4l2_fd_open_cb)(int, int) = fd_open;


int (*v4l2_close) (int) = close;
int (*v4l2_ioctl) (int, unsigned long int, ...) = ioctl;
ssize_t (*v4l2_read) (int, void *, size_t) = read;

void * (*v4l2_mmap) (void *, size_t, int, int, int, int64_t) = mmap;
int (*v4l2_munmap) (void *, size_t) = munmap;

static void v4l2_lib_load (void)
{
void *h;

h = dlopen ("libmediaclient.so", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
if (h == NULL)
h = dlopen ("libv4l2.so.0", RTLD_LAZY | RTLD_LOCAL);
if (h == NULL)
return;

void *sym = dlsym(h, "v4l2_fd_open");
if (sym != NULL)
v4l2_fd_open_cb = sym;

#define SYM(name) sym = dlsym (h, "v4l2_"#name); if (sym != NULL) v4l2_##name = sym



SYM(close); SYM(ioctl);
SYM(read); SYM(mmap); SYM(munmap);

v4l2_handle = h;
}

__attribute__((destructor))
static void v4l2_lib_unload (void)
{
if (v4l2_handle != NULL)
dlclose (v4l2_handle);
}

int v4l2_fd_open(int fd, int flags)
{
static pthread_once_t once = PTHREAD_ONCE_INIT;

pthread_once(&once, v4l2_lib_load);
return v4l2_fd_open_cb(fd, flags);
}
