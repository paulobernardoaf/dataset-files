

























#include <errno.h>
#include <fcntl.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_actions.h>

#if defined(HAVE_POLL)
#include <poll.h>
#endif

#include <lirc/lirc_client.h>

#define LIRC_TEXT N_("Change the lirc configuration file")
#define LIRC_LONGTEXT N_( "Tell lirc to read this configuration file. By default it " "searches in the users home directory." )






static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );

vlc_module_begin ()
set_shortname( N_("Infrared") )
set_category( CAT_INTERFACE )
set_subcategory( SUBCAT_INTERFACE_CONTROL )
set_description( N_("Infrared remote control interface") )
set_capability( "interface", 0 )
set_callbacks( Open, Close )

add_string( "lirc-file", NULL,
LIRC_TEXT, LIRC_LONGTEXT, true )
vlc_module_end ()




struct intf_sys_t
{
struct lirc_config *config;
vlc_thread_t thread;
int i_fd;
};




static void *Run( void * );

static void Process( intf_thread_t * );




static int Open( vlc_object_t *p_this )
{
intf_thread_t *p_intf = (intf_thread_t *)p_this;
intf_sys_t *p_sys;


p_intf->p_sys = p_sys = malloc( sizeof( intf_sys_t ) );
if( p_sys == NULL )
return VLC_ENOMEM;

p_sys->i_fd = lirc_init( "vlc", 1 );
if( p_sys->i_fd == -1 )
{
msg_Err( p_intf, "lirc initialisation failed" );
goto error;
}


fcntl( p_sys->i_fd, F_SETFL, fcntl( p_sys->i_fd, F_GETFL ) | O_NONBLOCK );


char *psz_file = var_InheritString( p_intf, "lirc-file" );
int val = lirc_readconfig( psz_file, &p_sys->config, NULL );
free( psz_file );
if( val != 0 )
{
msg_Err( p_intf, "failure while reading lirc config" );
lirc_deinit();
goto error;
}

if( vlc_clone( &p_sys->thread, Run, p_intf, VLC_THREAD_PRIORITY_LOW ) )
{
lirc_freeconfig( p_sys->config );
lirc_deinit();
goto error;
}

return VLC_SUCCESS;

error:
free( p_sys );
return VLC_EGENERIC;
}




static void Close( vlc_object_t *p_this )
{
intf_thread_t *p_intf = (intf_thread_t *)p_this;
intf_sys_t *p_sys = p_intf->p_sys;

vlc_cancel( p_sys->thread );
vlc_join( p_sys->thread, NULL );


lirc_freeconfig( p_sys->config );
lirc_deinit();
free( p_sys );
}




static void *Run( void *data )
{
intf_thread_t *p_intf = data;
intf_sys_t *p_sys = p_intf->p_sys;

struct pollfd ufd;
ufd.fd = p_sys->i_fd;
ufd.events = POLLIN;

for( ;; )
{

if( poll( &ufd, 1, -1 ) == -1 )
{
if( errno == EINTR )
continue;
break;
}


int canc = vlc_savecancel();
Process( p_intf );
vlc_restorecancel(canc);
}
return NULL;
}

static void Process( intf_thread_t *p_intf )
{
for( ;; )
{
char *code, *c;
if( lirc_nextcode( &code ) )
return;

if( code == NULL )
return;

while( (lirc_code2char( p_intf->p_sys->config, code, &c ) == 0)
&& (c != NULL) )
{
if( !strncmp( "key-", c, 4 ) )
{
vlc_action_id_t i_key = vlc_actions_get_id( c );
if( i_key )
var_SetInteger( vlc_object_instance(p_intf), "key-action", i_key );
else
msg_Err( p_intf, "Unknown hotkey '%s'", c );
}
else
{
msg_Err( p_intf, "this doesn't appear to be a valid keycombo "
"lirc sent us. Please look at the "
"doc/lirc/example.lirc file in VLC" );
break;
}
}
free( code );
}
}
