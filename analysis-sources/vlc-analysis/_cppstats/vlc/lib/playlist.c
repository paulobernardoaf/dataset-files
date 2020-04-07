#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "libvlc_internal.h"
#include "../src/libvlc.h"
#include <vlc/vlc.h>
#include <assert.h>
void libvlc_playlist_play( libvlc_instance_t *p_instance )
{
libvlc_InternalPlay( p_instance->p_libvlc_int );
}
int libvlc_add_intf( libvlc_instance_t *p_instance, const char *name )
{
if( libvlc_InternalAddIntf( p_instance->p_libvlc_int, name ))
{
if( name != NULL )
libvlc_printerr("interface \"%s\" initialization failed", name );
else
libvlc_printerr("default interface initialization failed");
return -1;
}
return 0;
}
