#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "../lib/libvlc_internal.h"
#include "modules/modules.h"
#include "config/configuration.h"
#include "preparser/preparser.h"
#include "media_source/media_source.h"
#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h>
#include "config/vlc_getopt.h"
#include <vlc_playlist.h>
#include <vlc_interface.h>
#include <vlc_actions.h>
#include <vlc_charset.h>
#include <vlc_dialog.h>
#include <vlc_keystore.h>
#include <vlc_fs.h>
#include <vlc_cpu.h>
#include <vlc_url.h>
#include <vlc_modules.h>
#include <vlc_media_library.h>
#include <vlc_thumbnailer.h>
#include "libvlc.h"
#include <vlc_vlm.h>
#include <assert.h>
static void GetFilenames ( libvlc_int_t *, unsigned, const char *const [] );
libvlc_int_t * libvlc_InternalCreate( void )
{
libvlc_int_t *p_libvlc;
libvlc_priv_t *priv;
p_libvlc = (vlc_custom_create)( NULL, sizeof (*priv), "libvlc" );
if( p_libvlc == NULL )
return NULL;
priv = libvlc_priv (p_libvlc);
vlc_mutex_init(&priv->lock);
priv->interfaces = NULL;
priv->main_playlist = NULL;
priv->p_vlm = NULL;
priv->media_source_provider = NULL;
vlc_ExitInit( &priv->exit );
return p_libvlc;
}
int libvlc_InternalInit( libvlc_int_t *p_libvlc, int i_argc,
const char *ppsz_argv[] )
{
libvlc_priv_t *priv = libvlc_priv (p_libvlc);
char * psz_modules = NULL;
char * psz_parser = NULL;
char * psz_control = NULL;
char *psz_val;
int i_ret = VLC_EGENERIC;
if (unlikely(vlc_LogPreinit(p_libvlc)))
return VLC_ENOMEM;
system_Init();
module_InitBank ();
if( config_LoadCmdLine( p_libvlc, i_argc, ppsz_argv, NULL ) )
{
module_EndBank (false);
return VLC_EGENERIC;
}
vlc_threads_setup (p_libvlc);
module_LoadPlugins (p_libvlc);
if( !var_InheritBool( p_libvlc, "ignore-config" ) )
{
if( var_InheritBool( p_libvlc, "reset-config" ) )
config_SaveConfigFile( p_libvlc ); 
else
config_LoadConfigFile( p_libvlc );
}
int vlc_optind;
if( config_LoadCmdLine( p_libvlc, i_argc, ppsz_argv, &vlc_optind ) )
goto error;
vlc_LogInit(p_libvlc);
#if defined( ENABLE_NLS ) && ( defined( HAVE_GETTEXT ) || defined( HAVE_INCLUDED_GETTEXT ) )
vlc_bindtextdomain (PACKAGE_NAME);
#endif
msg_Dbg( p_libvlc, "translation test: code is \"%s\"", _("C") );
if (config_PrintHelp (VLC_OBJECT(p_libvlc)))
{
libvlc_InternalCleanup (p_libvlc);
exit(0);
}
i_ret = VLC_ENOMEM;
if( libvlc_InternalDialogInit( p_libvlc ) != VLC_SUCCESS )
goto error;
if( libvlc_InternalKeystoreInit( p_libvlc ) != VLC_SUCCESS )
msg_Warn( p_libvlc, "memory keystore init failed" );
vlc_CPU_dump( VLC_OBJECT(p_libvlc) );
if( var_InheritBool( p_libvlc, "media-library") )
{
priv->p_media_library = libvlc_MlCreate( p_libvlc );
if ( priv->p_media_library == NULL )
msg_Warn( p_libvlc, "Media library initialization failed" );
}
priv->p_thumbnailer = vlc_thumbnailer_Create( VLC_OBJECT( p_libvlc ) );
if ( priv->p_thumbnailer == NULL )
msg_Warn( p_libvlc, "Failed to instantiate VLC thumbnailer" );
if( libvlc_InternalActionsInit( p_libvlc ) != VLC_SUCCESS )
goto error;
priv->parser = input_preparser_New(VLC_OBJECT(p_libvlc));
if( !priv->parser )
goto error;
priv->media_source_provider = vlc_media_source_provider_New( VLC_OBJECT( p_libvlc ) );
if( !priv->media_source_provider )
goto error;
var_Create( p_libvlc, "snapshot-file", VLC_VAR_STRING );
var_Create( p_libvlc, "record-file", VLC_VAR_STRING );
var_Create( p_libvlc, "window", VLC_VAR_STRING );
var_Create( p_libvlc, "vout-cb-type", VLC_VAR_INTEGER );
var_Create( p_libvlc, "user-agent", VLC_VAR_STRING );
var_SetString( p_libvlc, "user-agent",
"VLC media player (LibVLC "VERSION")" );
var_Create( p_libvlc, "http-user-agent", VLC_VAR_STRING );
var_SetString( p_libvlc, "http-user-agent",
"VLC/"PACKAGE_VERSION" LibVLC/"PACKAGE_VERSION );
var_Create( p_libvlc, "app-icon-name", VLC_VAR_STRING );
var_SetString( p_libvlc, "app-icon-name", PACKAGE_NAME );
var_Create( p_libvlc, "app-id", VLC_VAR_STRING );
var_SetString( p_libvlc, "app-id", "org.VideoLAN.VLC" );
var_Create( p_libvlc, "app-version", VLC_VAR_STRING );
var_SetString( p_libvlc, "app-version", PACKAGE_VERSION );
system_Configure( p_libvlc, i_argc - vlc_optind, ppsz_argv + vlc_optind );
#if defined(ENABLE_VLM)
psz_parser = var_InheritString( p_libvlc, "vlm-conf" );
if( psz_parser )
{
priv->p_vlm = vlm_New( p_libvlc, psz_parser );
if( !priv->p_vlm )
msg_Err( p_libvlc, "VLM initialization failed" );
free( psz_parser );
}
#endif
psz_modules = var_InheritString( p_libvlc, "extraintf" );
psz_control = var_InheritString( p_libvlc, "control" );
if( psz_modules && psz_control )
{
char* psz_tmp;
if( asprintf( &psz_tmp, "%s:%s", psz_modules, psz_control ) != -1 )
{
free( psz_modules );
psz_modules = psz_tmp;
}
}
else if( psz_control )
{
free( psz_modules );
psz_modules = strdup( psz_control );
}
psz_parser = psz_modules;
while ( psz_parser && *psz_parser )
{
char *psz_module, *psz_temp;
psz_module = psz_parser;
psz_parser = strchr( psz_module, ':' );
if ( psz_parser )
{
*psz_parser = '\0';
psz_parser++;
}
if( asprintf( &psz_temp, "%s,none", psz_module ) != -1)
{
libvlc_InternalAddIntf( p_libvlc, psz_temp );
free( psz_temp );
}
}
free( psz_modules );
free( psz_control );
if( var_InheritBool( p_libvlc, "network-synchronisation") )
libvlc_InternalAddIntf( p_libvlc, "netsync,none" );
#if defined(__APPLE__)
var_Create( p_libvlc, "drawable-view-top", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-view-left", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-view-bottom", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-view-right", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-clip-top", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-clip-left", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-clip-bottom", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-clip-right", VLC_VAR_INTEGER );
var_Create( p_libvlc, "drawable-nsobject", VLC_VAR_ADDRESS );
#endif
GetFilenames( p_libvlc, i_argc - vlc_optind, ppsz_argv + vlc_optind );
psz_val = var_InheritString( p_libvlc, "open" );
if ( psz_val != NULL )
{
intf_InsertItem( p_libvlc, psz_val, 0, NULL, 0 );
free( psz_val );
}
var_Create(p_libvlc, "intf-popupmenu", VLC_VAR_BOOL);
var_Create(p_libvlc, "intf-toggle-fscontrol", VLC_VAR_VOID);
var_Create(p_libvlc, "intf-boss", VLC_VAR_VOID);
var_Create(p_libvlc, "intf-show", VLC_VAR_VOID);
return VLC_SUCCESS;
error:
libvlc_InternalCleanup( p_libvlc );
return i_ret;
}
void libvlc_InternalCleanup( libvlc_int_t *p_libvlc )
{
libvlc_priv_t *priv = libvlc_priv (p_libvlc);
if (priv->parser != NULL)
input_preparser_Deactivate(priv->parser);
msg_Dbg( p_libvlc, "removing all interfaces" );
intf_DestroyAll( p_libvlc );
if ( priv->p_thumbnailer )
vlc_thumbnailer_Release( priv->p_thumbnailer );
if( priv->media_source_provider )
vlc_media_source_provider_Delete( priv->media_source_provider );
libvlc_InternalDialogClean( p_libvlc );
libvlc_InternalKeystoreClean( p_libvlc );
#if defined(ENABLE_VLM)
if( priv->p_vlm )
{
vlm_Delete( priv->p_vlm );
}
#endif
#if !defined( _WIN32 ) && !defined( __OS2__ )
char *pidfile = var_InheritString( p_libvlc, "pidfile" );
if( pidfile != NULL )
{
msg_Dbg( p_libvlc, "removing PID file %s", pidfile );
if( unlink( pidfile ) )
msg_Warn( p_libvlc, "cannot remove PID file %s: %s",
pidfile, vlc_strerror_c(errno) );
free( pidfile );
}
#endif
if (priv->parser != NULL)
input_preparser_Delete(priv->parser);
if (priv->main_playlist)
vlc_playlist_Delete(priv->main_playlist);
if ( priv->p_media_library )
libvlc_MlRelease( priv->p_media_library );
libvlc_InternalActionsClean( p_libvlc );
if( !var_InheritBool( p_libvlc, "ignore-config" ) )
config_AutoSaveConfigFile( VLC_OBJECT(p_libvlc) );
vlc_LogDestroy(p_libvlc->obj.logger);
module_EndBank (true);
#if defined(_WIN32) || defined(__OS2__)
system_End( );
#endif
}
void libvlc_InternalDestroy( libvlc_int_t *p_libvlc )
{
vlc_object_delete(p_libvlc);
}
static void GetFilenames( libvlc_int_t *p_vlc, unsigned n,
const char *const args[] )
{
while( n > 0 )
{
unsigned i_options = 0;
while( args[--n][0] == ':' )
{
i_options++;
if( n == 0 )
{
msg_Warn( p_vlc, "options %s without item", args[n] );
return; 
}
}
char *mrl = NULL;
if( strstr( args[n], "://" ) == NULL )
{
mrl = vlc_path2uri( args[n], NULL );
if( !mrl )
continue;
}
intf_InsertItem( p_vlc, (mrl != NULL) ? mrl : args[n], i_options,
( i_options ? &args[n + 1] : NULL ),
VLC_INPUT_OPTION_TRUSTED );
free( mrl );
}
}
int vlc_MetadataRequest(libvlc_int_t *libvlc, input_item_t *item,
input_item_meta_request_option_t i_options,
const input_preparser_callbacks_t *cbs,
void *cbs_userdata,
int timeout, void *id)
{
libvlc_priv_t *priv = libvlc_priv(libvlc);
if (unlikely(priv->parser == NULL))
return VLC_ENOMEM;
input_preparser_Push( priv->parser, item, i_options, cbs, cbs_userdata, timeout, id );
return VLC_SUCCESS;
}
int libvlc_MetadataRequest(libvlc_int_t *libvlc, input_item_t *item,
input_item_meta_request_option_t i_options,
const input_preparser_callbacks_t *cbs,
void *cbs_userdata,
int timeout, void *id)
{
libvlc_priv_t *priv = libvlc_priv(libvlc);
assert(i_options & META_REQUEST_OPTION_SCOPE_ANY);
if (unlikely(priv->parser == NULL))
return VLC_ENOMEM;
vlc_mutex_lock( &item->lock );
if( item->i_preparse_depth == 0 )
item->i_preparse_depth = 1;
vlc_mutex_unlock( &item->lock );
return vlc_MetadataRequest(libvlc, item, i_options, cbs, cbs_userdata, timeout, id);
}
int libvlc_ArtRequest(libvlc_int_t *libvlc, input_item_t *item,
input_item_meta_request_option_t i_options,
const input_fetcher_callbacks_t *cbs,
void *cbs_userdata)
{
libvlc_priv_t *priv = libvlc_priv(libvlc);
assert(i_options & META_REQUEST_OPTION_FETCH_ANY);
if (unlikely(priv->parser == NULL))
return VLC_ENOMEM;
input_preparser_fetcher_Push(priv->parser, item, i_options,
cbs, cbs_userdata);
return VLC_SUCCESS;
}
void libvlc_MetadataCancel(libvlc_int_t *libvlc, void *id)
{
libvlc_priv_t *priv = libvlc_priv(libvlc);
if (unlikely(priv->parser == NULL))
return;
input_preparser_Cancel(priv->parser, id);
}
