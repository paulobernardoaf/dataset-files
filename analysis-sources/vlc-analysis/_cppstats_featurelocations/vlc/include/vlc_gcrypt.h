
























#include <errno.h>

static inline void vlc_gcrypt_init (void)
{





static bool done = false;

vlc_global_lock (VLC_GCRYPT_MUTEX);
if (!done)
{



gcry_check_version(NULL);
done = true;
}
vlc_global_unlock (VLC_GCRYPT_MUTEX);
}
