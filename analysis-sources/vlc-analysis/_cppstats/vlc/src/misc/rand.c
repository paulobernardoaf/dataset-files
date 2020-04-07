#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_rand.h>
static struct
{
bool init;
unsigned short subi[3];
vlc_mutex_t lock;
} rand48 = { false, { 0, 0, 0, }, VLC_STATIC_MUTEX, };
static void init_rand48 (void)
{
if (!rand48.init)
{
vlc_rand_bytes (rand48.subi, sizeof (rand48.subi));
rand48.init = true;
#if 0 
for (unsigned i = 0; i < 3; i++)
subi[i] &= 0xffff;
#endif
}
}
double vlc_drand48 (void)
{
double ret;
vlc_mutex_lock (&rand48.lock);
init_rand48 ();
ret = erand48 (rand48.subi);
vlc_mutex_unlock (&rand48.lock);
return ret;
}
long vlc_lrand48 (void)
{
long ret;
vlc_mutex_lock (&rand48.lock);
init_rand48 ();
ret = nrand48 (rand48.subi);
vlc_mutex_unlock (&rand48.lock);
return ret;
}
long vlc_mrand48 (void)
{
long ret;
vlc_mutex_lock (&rand48.lock);
init_rand48 ();
ret = jrand48 (rand48.subi);
vlc_mutex_unlock (&rand48.lock);
return ret;
}
