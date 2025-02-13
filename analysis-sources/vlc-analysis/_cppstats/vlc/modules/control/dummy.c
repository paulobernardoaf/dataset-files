#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>
static int Open( vlc_object_t * );
vlc_module_begin ()
set_shortname( N_("Dummy") )
set_description( N_("Dummy interface") )
set_capability( "interface", 0 )
set_callback( Open )
#if defined(_WIN32) && !VLC_WINSTORE_APP
add_obsolete_bool( "dummy-quiet" )
#endif
vlc_module_end ()
static int Open( vlc_object_t *p_this )
{
intf_thread_t *p_intf = (intf_thread_t*) p_this;
msg_Info( p_intf, "using the dummy interface module..." );
return VLC_SUCCESS;
}
