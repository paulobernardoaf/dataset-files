





















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <errno.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_access.h>
#include <vlc_url.h>
#include <vlc_tls.h>

static ssize_t Read(stream_t *access, void *buf, size_t len)
{
return vlc_tls_Read(access->p_sys, buf, len, false);
}

static int Control( stream_t *p_access, int i_query, va_list args )
{
bool *pb_bool;

switch( i_query )
{
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
pb_bool = va_arg( args, bool * );
*pb_bool = false;
break;
case STREAM_CAN_PAUSE:
pb_bool = va_arg( args, bool * );
*pb_bool = true; 
break;
case STREAM_CAN_CONTROL_PACE:
pb_bool = va_arg( args, bool * );
*pb_bool = true; 
break;

case STREAM_GET_PTS_DELAY:
*va_arg( args, vlc_tick_t * ) =
VLC_TICK_FROM_MS(var_InheritInteger( p_access, "network-caching" ));
break;

case STREAM_SET_PAUSE_STATE:

break;

default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}

static int Open(vlc_object_t *obj)
{
stream_t *access = (stream_t *)obj;
vlc_tls_t *sock;
vlc_url_t url;

if (vlc_UrlParse(&url, access->psz_url)
|| url.psz_host == NULL || url.i_port == 0)
{
msg_Err(access, "invalid location: %s", access->psz_location);
vlc_UrlClean(&url);
return VLC_EGENERIC;
}

sock = vlc_tls_SocketOpenTCP(obj, url.psz_host, url.i_port);
vlc_UrlClean(&url);
if (sock == NULL)
return VLC_EGENERIC;

access->p_sys = sock;
access->pf_read = Read;
access->pf_block = NULL;
access->pf_control = Control;
access->pf_seek = NULL;
return VLC_SUCCESS;
}

static void Close( vlc_object_t *p_this )
{
stream_t *access = (stream_t *)p_this;

vlc_tls_SessionDelete(access->p_sys);
}




vlc_module_begin ()
set_shortname( N_("TCP") )
set_description( N_("TCP input") )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_ACCESS )

set_capability( "access", 0 )
add_shortcut( "tcp" )
set_callbacks( Open, Close )
vlc_module_end ()
