
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <vlc_common.h>
#include "modules/modules.h"

#include <sys/types.h>
#include <dlfcn.h>

#if defined(HAVE_VALGRIND_VALGRIND_H)
#include <valgrind/valgrind.h>
#endif

void *vlc_dlopen(const char *path, bool lazy)
{
#if defined (RTLD_NOW)
const int flags = lazy ? RTLD_LAZY : RTLD_NOW;
#elif defined (DL_LAZY)
const int flags = DL_LAZY;
VLC_UNUSED(lazy);
#else
const int flags = 0;
VLC_UNUSED(lazy);
#endif
return dlopen (path, flags);
}

int vlc_dlclose(void *handle)
{
#if !defined(__SANITIZE_ADDRESS__)
#if defined(HAVE_VALGRIND_VALGRIND_H)
if( RUNNING_ON_VALGRIND > 0 )
return 0; 
#endif
int err = dlclose( handle );
assert(err == 0);
return err;
#else
(void) handle;
return 0;
#endif
}

void *vlc_dlsym(void *handle, const char *name)
{
return dlsym(handle, name);
}

char *vlc_dlerror(void)
{
const char *errmsg = dlerror();

return (errmsg != NULL) ? strdup(errmsg) : NULL;
}
