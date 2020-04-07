





















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <vlc_common.h>

#if defined(__native_client__)
long pathconf (const char *path, int name)
{
VLC_UNUSED(path);
VLC_UNUSED(name);
return -1;
}
#elif defined(_WIN32)

#else
#error pathconf not implemented on your platform!
#endif
