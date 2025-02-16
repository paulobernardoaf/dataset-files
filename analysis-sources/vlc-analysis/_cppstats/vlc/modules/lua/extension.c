#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "vlc.h"
#include "libs.h"
#include "extension.h"
#include "assert.h"
#include <vlc_common.h>
#include <vlc_interface.h>
#include <vlc_events.h>
#include <vlc_dialog.h>
static const luaL_Reg p_reg[] =
{
{ NULL, NULL }
};
static const char caps[][20] = {
#define EXT_HAS_MENU (1 << 0) 
"menu",
#define EXT_TRIGGER_ONLY (1 << 1) 
"trigger",
#define EXT_INPUT_LISTENER (1 << 2) 
"input-listener",
#define EXT_META_LISTENER (1 << 3) 
"meta-listener",
#define EXT_PLAYING_LISTENER (1 << 4) 
"playing-listener",
};
static int ScanExtensions( extensions_manager_t *p_this );
static int ScanLuaCallback( vlc_object_t *p_this, const char *psz_script,
const struct luabatch_context_t * );
static int Control( extensions_manager_t *, int, va_list );
static int GetMenuEntries( extensions_manager_t *p_mgr, extension_t *p_ext,
char ***pppsz_titles, uint16_t **ppi_ids );
static lua_State* GetLuaState( extensions_manager_t *p_mgr,
extension_t *p_ext );
static int TriggerMenu( extension_t *p_ext, int id );
static int TriggerExtension( extensions_manager_t *p_mgr,
extension_t *p_ext );
static void WatchTimerCallback( void* );
static int vlclua_extension_deactivate( lua_State *L );
static int vlclua_extension_keep_alive( lua_State *L );
static int vlclua_extension_dialog_callback( vlc_object_t *p_this,
char const *psz_var,
vlc_value_t oldval,
vlc_value_t newval,
void *p_data );
static void inputItemMetaChanged( const vlc_event_t *p_event,
void *data );
int Open_Extension( vlc_object_t *p_this )
{
if( lua_Disabled( p_this ) )
return VLC_EGENERIC;
msg_Dbg( p_this, "Opening Lua Extension module" );
extensions_manager_t *p_mgr = ( extensions_manager_t* ) p_this;
p_mgr->pf_control = Control;
p_mgr->p_sys = NULL;
vlc_mutex_init( &p_mgr->lock );
if( ScanExtensions( p_mgr ) != VLC_SUCCESS )
{
msg_Err( p_mgr, "Can't load extensions modules" );
return VLC_EGENERIC;
}
var_Create( p_this, "dialog-event", VLC_VAR_ADDRESS );
var_AddCallback( p_this, "dialog-event",
vlclua_extension_dialog_callback, NULL );
return VLC_SUCCESS;
}
void Close_Extension( vlc_object_t *p_this )
{
extensions_manager_t *p_mgr = ( extensions_manager_t* ) p_this;
var_DelCallback( p_this, "dialog-event",
vlclua_extension_dialog_callback, NULL );
var_Destroy( p_mgr, "dialog-event" );
extension_t *p_ext = NULL;
ARRAY_FOREACH( p_ext, p_mgr->extensions )
{
if( !p_ext )
break;
vlc_mutex_lock( &p_ext->p_sys->command_lock );
if( p_ext->p_sys->b_activated == true && p_ext->p_sys->p_progress_id == NULL )
{
p_ext->p_sys->b_exiting = true;
QueueDeactivateCommand( p_ext );
}
else
{
if ( p_ext->p_sys->L != NULL )
vlclua_fd_interrupt( &p_ext->p_sys->dtable );
p_ext->p_sys->b_exiting = true;
vlc_cond_signal( &p_ext->p_sys->wait );
}
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
if( p_ext->p_sys->b_thread_running == true )
vlc_join( p_ext->p_sys->thread, NULL );
if( p_ext->p_sys->L )
{
lua_close( p_ext->p_sys->L );
vlclua_fd_cleanup( &p_ext->p_sys->dtable );
}
free( p_ext->psz_name );
free( p_ext->psz_title );
free( p_ext->psz_author );
free( p_ext->psz_description );
free( p_ext->psz_shortdescription );
free( p_ext->psz_url );
free( p_ext->psz_version );
free( p_ext->p_icondata );
vlc_timer_destroy( p_ext->p_sys->timer );
free( p_ext->p_sys );
free( p_ext );
}
ARRAY_RESET( p_mgr->extensions );
}
static int ScanExtensions( extensions_manager_t *p_mgr )
{
int i_ret =
vlclua_scripts_batch_execute( VLC_OBJECT( p_mgr ),
"extensions",
&ScanLuaCallback,
NULL );
if( !i_ret )
return VLC_EGENERIC;
return VLC_SUCCESS;
}
static int vlclua_dummy_require( lua_State *L )
{
(void) L;
return 0;
}
static int vlclua_extension_require( lua_State *L )
{
const char *psz_module = luaL_checkstring( L, 1 );
vlc_object_t *p_this = vlclua_get_this( L );
extension_t *p_ext = vlclua_extension_get( L );
msg_Dbg( p_this, "loading module '%s' from extension package",
psz_module );
char *psz_fullpath, *psz_package, *sep;
psz_package = strdup( p_ext->psz_name );
sep = strrchr( psz_package, '/' );
if( !sep )
{
free( psz_package );
return luaL_error( L, "could not find package name" );
}
*sep = '\0';
if( -1 == asprintf( &psz_fullpath,
"%s/modules/%s.luac", psz_package, psz_module ) )
{
free( psz_package );
return 1;
}
int i_ret = vlclua_dofile( p_this, L, psz_fullpath );
if( i_ret != 0 )
{
psz_fullpath[ strlen( psz_fullpath ) - 1 ] = '\0';
i_ret = vlclua_dofile( p_this, L, psz_fullpath );
}
free( psz_fullpath );
free( psz_package );
if( i_ret != 0 )
{
return luaL_error( L, "unable to load module '%s' from package",
psz_module );
}
return 0;
}
int ScanLuaCallback( vlc_object_t *p_this, const char *psz_filename,
const struct luabatch_context_t *dummy )
{
VLC_UNUSED(dummy);
extensions_manager_t *p_mgr = ( extensions_manager_t* ) p_this;
bool b_ok = false;
msg_Dbg( p_mgr, "Scanning Lua script %s", psz_filename );
char *psz_script = NULL;
int i_flen = strlen( psz_filename );
if( !strncasecmp( psz_filename + i_flen - 4, ".vle", 4 ) )
{
msg_Dbg( p_this, "reading Lua script in a zip archive" );
psz_script = calloc( 1, i_flen + 6 + 12 + 1 );
if( !psz_script )
return 0;
strcpy( psz_script, "zip://" );
strncat( psz_script, psz_filename, i_flen + 19 );
strncat( psz_script, "!/script.lua", i_flen + 19 );
}
else
{
psz_script = strdup( psz_filename );
if( !psz_script )
return 0;
}
extension_t *p_ext = ( extension_t* ) calloc( 1, sizeof( extension_t ) );
if( !p_ext )
{
free( psz_script );
return 0;
}
p_ext->psz_name = psz_script;
p_ext->p_sys = (extension_sys_t*) calloc( 1, sizeof( extension_sys_t ) );
if( !p_ext->p_sys || !p_ext->psz_name )
{
free( p_ext->psz_name );
free( p_ext->p_sys );
free( p_ext );
return 0;
}
p_ext->p_sys->p_mgr = p_mgr;
if( vlc_timer_create( &p_ext->p_sys->timer, WatchTimerCallback, p_ext ) )
{
free( p_ext->psz_name );
free( p_ext->p_sys );
free( p_ext );
return 0;
}
vlc_mutex_init( &p_ext->p_sys->command_lock );
vlc_mutex_init( &p_ext->p_sys->running_lock );
vlc_cond_init( &p_ext->p_sys->wait );
lua_State *L = luaL_newstate();
lua_register( L, "require", &vlclua_dummy_require );
if( vlclua_dofile( p_this, L, psz_script ) ) 
{
msg_Warn( p_mgr, "Error loading script %s: %s", psz_script,
lua_tostring( L, lua_gettop( L ) ) );
lua_pop( L, 1 );
goto exit;
}
lua_getglobal( L, "descriptor" );
if( !lua_isfunction( L, -1 ) )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function descriptor() not found", psz_script );
goto exit;
}
if( lua_pcall( L, 0, 1, 0 ) )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function descriptor(): %s", psz_script,
lua_tostring( L, lua_gettop( L ) ) );
goto exit;
}
if( lua_gettop( L ) )
{
if( lua_istable( L, -1 ) )
{
lua_getfield( L, -1, "capabilities" );
if( lua_istable( L, -1 ) )
{
lua_pushnil( L );
while( lua_next( L, -2 ) != 0 )
{
const char *psz_cap = luaL_checkstring( L, -1 );
bool found = false;
for( size_t i = 0; i < sizeof(caps)/sizeof(caps[0]); i++ )
{
if( !strcmp( caps[i], psz_cap ) )
{
p_ext->p_sys->i_capabilities |= 1 << i;
found = true;
break;
}
}
if( !found )
{
msg_Warn( p_mgr, "Extension capability '%s' unknown in"
" script %s", psz_cap, psz_script );
}
lua_pop( L, 1 );
}
}
else
{
msg_Warn( p_mgr, "In script %s, function descriptor() "
"did not return a table of capabilities.",
psz_script );
}
lua_pop( L, 1 );
lua_getfield( L, -1, "title" );
if( lua_isstring( L, -1 ) )
{
p_ext->psz_title = strdup( luaL_checkstring( L, -1 ) );
}
else
{
msg_Dbg( p_mgr, "In script %s, function descriptor() "
"did not return a string as title.",
psz_script );
p_ext->psz_title = strdup( psz_script );
}
lua_pop( L, 1 );
lua_getfield( L, -1, "author" );
p_ext->psz_author = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "description" );
p_ext->psz_description = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "shortdesc" );
p_ext->psz_shortdescription = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "url" );
p_ext->psz_url = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "version" );
p_ext->psz_version = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "icon" );
if( !lua_isnil( L, -1 ) && lua_isstring( L, -1 ) )
{
int len = lua_strlen( L, -1 );
p_ext->p_icondata = malloc( len );
if( p_ext->p_icondata )
{
p_ext->i_icondata_size = len;
memcpy( p_ext->p_icondata, lua_tostring( L, -1 ), len );
}
}
lua_pop( L, 1 );
}
else
{
msg_Warn( p_mgr, "In script %s, function descriptor() "
"did not return a table!", psz_script );
goto exit;
}
}
else
{
msg_Err( p_mgr, "Script %s went completely foobar", psz_script );
goto exit;
}
msg_Dbg( p_mgr, "Script %s has the following capability flags: 0x%x",
psz_script, p_ext->p_sys->i_capabilities );
b_ok = true;
exit:
lua_close( L );
if( !b_ok )
{
free( p_ext->psz_name );
free( p_ext->psz_title );
free( p_ext->psz_url );
free( p_ext->psz_author );
free( p_ext->psz_description );
free( p_ext->psz_shortdescription );
free( p_ext->psz_version );
free( p_ext->p_sys );
free( p_ext );
}
else
{
ARRAY_APPEND( p_mgr->extensions, p_ext );
}
return VLC_EGENERIC;
}
static int Control( extensions_manager_t *p_mgr, int i_control, va_list args )
{
extension_t *p_ext = NULL;
bool *pb = NULL;
uint16_t **ppus = NULL;
char ***pppsz = NULL;
int i = 0;
switch( i_control )
{
case EXTENSION_ACTIVATE:
p_ext = va_arg( args, extension_t* );
return Activate( p_mgr, p_ext );
case EXTENSION_DEACTIVATE:
p_ext = va_arg( args, extension_t* );
return Deactivate( p_mgr, p_ext );
case EXTENSION_IS_ACTIVATED:
p_ext = va_arg( args, extension_t* );
pb = va_arg( args, bool* );
vlc_mutex_lock( &p_ext->p_sys->command_lock );
*pb = p_ext->p_sys->b_activated;
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
break;
case EXTENSION_HAS_MENU:
p_ext = va_arg( args, extension_t* );
pb = va_arg( args, bool* );
*pb = ( p_ext->p_sys->i_capabilities & EXT_HAS_MENU ) ? 1 : 0;
break;
case EXTENSION_GET_MENU:
p_ext = va_arg( args, extension_t* );
pppsz = va_arg( args, char*** );
ppus = va_arg( args, uint16_t** );
if( p_ext == NULL )
return VLC_EGENERIC;
return GetMenuEntries( p_mgr, p_ext, pppsz, ppus );
case EXTENSION_TRIGGER_ONLY:
p_ext = va_arg( args, extension_t* );
pb = va_arg( args, bool* );
*pb = ( p_ext->p_sys->i_capabilities & EXT_TRIGGER_ONLY ) ? 1 : 0;
break;
case EXTENSION_TRIGGER:
p_ext = va_arg( args, extension_t* );
return TriggerExtension( p_mgr, p_ext );
case EXTENSION_TRIGGER_MENU:
p_ext = va_arg( args, extension_t* );
i = va_arg( args, int );
return TriggerMenu( p_ext, i );
case EXTENSION_SET_INPUT:
{
p_ext = va_arg( args, extension_t* );
input_item_t *p_item = va_arg( args, struct input_item_t * );
if( p_ext == NULL )
return VLC_EGENERIC;
vlc_mutex_lock( &p_ext->p_sys->command_lock );
if ( p_ext->p_sys->b_exiting == true )
{
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
return VLC_EGENERIC;
}
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
vlc_mutex_lock( &p_ext->p_sys->running_lock );
input_item_t *old = p_ext->p_sys->p_item;
if( old )
{
if( p_ext->p_sys->i_capabilities & EXT_META_LISTENER )
{
vlc_event_detach( &old->event_manager,
vlc_InputItemMetaChanged,
inputItemMetaChanged,
p_ext );
}
input_item_Release( old );
}
p_ext->p_sys->p_item = p_item ? input_item_Hold(p_item) : NULL;
if( p_ext->p_sys->i_capabilities & EXT_INPUT_LISTENER )
{
PushCommandUnique( p_ext, CMD_SET_INPUT );
}
if( p_ext->p_sys->p_item &&
p_ext->p_sys->i_capabilities & EXT_META_LISTENER )
{
vlc_event_attach( &p_item->event_manager,
vlc_InputItemMetaChanged,
inputItemMetaChanged,
p_ext );
}
vlc_mutex_unlock( &p_ext->p_sys->running_lock );
break;
}
case EXTENSION_PLAYING_CHANGED:
{
p_ext = va_arg( args, extension_t* );
assert( p_ext->psz_name != NULL );
i = va_arg( args, int );
if( p_ext->p_sys->i_capabilities & EXT_PLAYING_LISTENER )
{
PushCommand( p_ext, CMD_PLAYING_CHANGED, i );
}
break;
}
case EXTENSION_META_CHANGED:
{
p_ext = va_arg( args, extension_t* );
PushCommand( p_ext, CMD_UPDATE_META );
break;
}
default:
msg_Warn( p_mgr, "Control '%d' not yet implemented in Extension",
i_control );
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
int lua_ExtensionActivate( extensions_manager_t *p_mgr, extension_t *p_ext )
{
assert( p_mgr != NULL && p_ext != NULL );
return lua_ExecuteFunction( p_mgr, p_ext, "activate", LUA_END );
}
int lua_ExtensionDeactivate( extensions_manager_t *p_mgr, extension_t *p_ext )
{
assert( p_mgr != NULL && p_ext != NULL );
if( p_ext->p_sys->b_activated == false )
return VLC_SUCCESS;
vlclua_fd_interrupt( &p_ext->p_sys->dtable );
if( p_ext->p_sys->p_item )
{
if( p_ext->p_sys->i_capabilities & EXT_META_LISTENER )
vlc_event_detach( &p_ext->p_sys->p_item->event_manager,
vlc_InputItemMetaChanged,
inputItemMetaChanged,
p_ext );
input_item_Release(p_ext->p_sys->p_item);
p_ext->p_sys->p_item = NULL;
}
int i_ret = lua_ExecuteFunction( p_mgr, p_ext, "deactivate", LUA_END );
if ( p_ext->p_sys->L == NULL )
return VLC_EGENERIC;
lua_close( p_ext->p_sys->L );
p_ext->p_sys->L = NULL;
return i_ret;
}
int lua_ExtensionWidgetClick( extensions_manager_t *p_mgr,
extension_t *p_ext,
extension_widget_t *p_widget )
{
if( !p_ext->p_sys->L )
return VLC_SUCCESS;
lua_State *L = GetLuaState( p_mgr, p_ext );
lua_pushlightuserdata( L, p_widget );
lua_gettable( L, LUA_REGISTRYINDEX );
return lua_ExecuteFunction( p_mgr, p_ext, NULL, LUA_END );
}
static int GetMenuEntries( extensions_manager_t *p_mgr, extension_t *p_ext,
char ***pppsz_titles, uint16_t **ppi_ids )
{
assert( *pppsz_titles == NULL );
assert( *ppi_ids == NULL );
vlc_mutex_lock( &p_ext->p_sys->command_lock );
if( p_ext->p_sys->b_activated == false || p_ext->p_sys->b_exiting == true )
{
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
msg_Dbg( p_mgr, "Can't get menu of an unactivated/dying extension!" );
return VLC_EGENERIC;
}
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
vlc_mutex_lock( &p_ext->p_sys->running_lock );
int i_ret = VLC_EGENERIC;
lua_State *L = GetLuaState( p_mgr, p_ext );
if( ( p_ext->p_sys->i_capabilities & EXT_HAS_MENU ) == 0 )
{
msg_Dbg( p_mgr, "can't get a menu from an extension without menu!" );
goto exit;
}
lua_getglobal( L, "menu" );
if( !lua_isfunction( L, -1 ) )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function menu() not found", p_ext->psz_name );
goto exit;
}
if( lua_pcall( L, 0, 1, 0 ) )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function menu(): %s", p_ext->psz_name,
lua_tostring( L, lua_gettop( L ) ) );
goto exit;
}
if( lua_gettop( L ) )
{
if( lua_istable( L, -1 ) )
{
size_t i_size = lua_objlen( L, -1 );
*pppsz_titles = ( char** ) calloc( i_size+1, sizeof( char* ) );
*ppi_ids = ( uint16_t* ) calloc( i_size+1, sizeof( uint16_t ) );
size_t i_idx = 0;
lua_pushnil( L );
while( lua_next( L, -2 ) != 0 )
{
assert( i_idx < i_size );
if( (!lua_isstring( L, -1 )) || (!lua_isnumber( L, -2 )) )
{
msg_Warn( p_mgr, "In script %s, an entry in "
"the menu table is invalid!", p_ext->psz_name );
goto exit;
}
(*pppsz_titles)[ i_idx ] = strdup( luaL_checkstring( L, -1 ) );
(*ppi_ids)[ i_idx ] = luaL_checkinteger( L, -2 ) & 0xFFFF;
i_idx++;
lua_pop( L, 1 );
}
}
else
{
msg_Warn( p_mgr, "Function menu() in script %s "
"did not return a table", p_ext->psz_name );
goto exit;
}
}
else
{
msg_Warn( p_mgr, "Script %s went completely foobar", p_ext->psz_name );
goto exit;
}
i_ret = VLC_SUCCESS;
exit:
vlc_mutex_unlock( &p_ext->p_sys->running_lock );
if( i_ret != VLC_SUCCESS )
{
msg_Dbg( p_mgr, "Something went wrong in %s (%s:%d)",
__func__, __FILE__, __LINE__ );
}
return i_ret;
}
static lua_State* GetLuaState( extensions_manager_t *p_mgr,
extension_t *p_ext )
{
assert( p_ext != NULL );
lua_State *L = p_ext->p_sys->L;
if( !L )
{
L = luaL_newstate();
if( !L )
{
msg_Err( p_mgr, "Could not create new Lua State" );
return NULL;
}
vlclua_set_this( L, p_mgr );
intf_thread_t *intf = (intf_thread_t *) vlc_object_parent(p_mgr);
vlc_playlist_t *playlist = vlc_intf_GetMainPlaylist(intf);
vlclua_set_playlist_internal(L, playlist);
vlclua_extension_set( L, p_ext );
luaL_openlibs( L );
luaL_register_namespace( L, "vlc", p_reg );
luaopen_msg( L );
luaopen_config( L );
luaopen_dialog( L, p_ext );
luaopen_input( L );
luaopen_msg( L );
if( vlclua_fd_init( L, &p_ext->p_sys->dtable ) )
{
lua_close( L );
return NULL;
}
luaopen_object( L );
luaopen_osd( L );
luaopen_playlist( L );
luaopen_stream( L );
luaopen_strings( L );
luaopen_variables( L );
luaopen_video( L );
luaopen_vlm( L );
luaopen_volume( L );
luaopen_xml( L );
luaopen_vlcio( L );
luaopen_errno( L );
luaopen_rand( L );
#if defined(_WIN32) && !VLC_WINSTORE_APP
luaopen_win( L );
#endif
lua_getglobal( L, "vlc" );
lua_pushcfunction( L, vlclua_extension_deactivate );
lua_setfield( L, -2, "deactivate" );
lua_pushcfunction( L, vlclua_extension_keep_alive );
lua_setfield( L, -2, "keep_alive" );
if( !strncmp( p_ext->psz_name, "zip://", 6 ) )
{
lua_register( L, "require", &vlclua_extension_require );
}
else
{
if( vlclua_add_modules_path( L, p_ext->psz_name ) )
{
msg_Warn( p_mgr, "Error while setting the module "
"search path for %s", p_ext->psz_name );
vlclua_fd_cleanup( &p_ext->p_sys->dtable );
lua_close( L );
return NULL;
}
}
if( vlclua_dofile( VLC_OBJECT( p_mgr ), L, p_ext->psz_name ) )
{
msg_Warn( p_mgr, "Error loading script %s: %s", p_ext->psz_name,
lua_tostring( L, lua_gettop( L ) ) );
vlclua_fd_cleanup( &p_ext->p_sys->dtable );
lua_close( L );
return NULL;
}
p_ext->p_sys->L = L;
}
return L;
}
int lua_ExecuteFunction( extensions_manager_t *p_mgr, extension_t *p_ext,
const char *psz_function, ... )
{
va_list args;
va_start( args, psz_function );
int i_ret = lua_ExecuteFunctionVa( p_mgr, p_ext, psz_function, args );
va_end( args );
return i_ret;
}
int lua_ExecuteFunctionVa( extensions_manager_t *p_mgr, extension_t *p_ext,
const char *psz_function, va_list args )
{
int i_ret = VLC_SUCCESS;
int i_args = 0;
assert( p_mgr != NULL );
assert( p_ext != NULL );
lua_State *L = GetLuaState( p_mgr, p_ext );
if( !L )
return -1;
if( psz_function )
lua_getglobal( L, psz_function );
if( !lua_isfunction( L, -1 ) )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function %s() not found", p_ext->psz_name, psz_function );
lua_pop( L, 1 );
goto exit;
}
lua_datatype_e type = LUA_END;
while( ( type = va_arg( args, int ) ) != LUA_END )
{
if( type == LUA_NUM )
{
lua_pushnumber( L , va_arg( args, int ) );
}
else if( type == LUA_TEXT )
{
lua_pushstring( L , va_arg( args, char* ) );
}
else
{
msg_Warn( p_mgr, "Undefined argument type %d to lua function %s"
"from script %s", type, psz_function, p_ext->psz_name );
if( i_args > 0 )
lua_pop( L, i_args );
goto exit;
}
i_args ++;
}
if( lua_pcall( L, i_args, 1, 0 ) )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function %s(): %s", p_ext->psz_name, psz_function,
lua_tostring( L, lua_gettop( L ) ) );
i_ret = VLC_EGENERIC;
}
i_ret |= lua_DialogFlush( L );
exit:
return i_ret;
}
static inline int TriggerMenu( extension_t *p_ext, int i_id )
{
return PushCommand( p_ext, CMD_TRIGGERMENU, i_id );
}
int lua_ExtensionTriggerMenu( extensions_manager_t *p_mgr,
extension_t *p_ext, int id )
{
int i_ret = VLC_SUCCESS;
lua_State *L = GetLuaState( p_mgr, p_ext );
if( !L )
return VLC_EGENERIC;
luaopen_dialog( L, p_ext );
lua_getglobal( L, "trigger_menu" );
if( !lua_isfunction( L, -1 ) )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function trigger_menu() not found", p_ext->psz_name );
return VLC_EGENERIC;
}
lua_pushinteger( L, id );
if( lua_pcall( L, 1, 1, 0 ) != 0 )
{
msg_Warn( p_mgr, "Error while running script %s, "
"function trigger_menu(): %s", p_ext->psz_name,
lua_tostring( L, lua_gettop( L ) ) );
i_ret = VLC_EGENERIC;
}
i_ret |= lua_DialogFlush( L );
if( i_ret < VLC_SUCCESS )
{
msg_Dbg( p_mgr, "Something went wrong in %s (%s:%d)",
__func__, __FILE__, __LINE__ );
}
return i_ret;
}
static int TriggerExtension( extensions_manager_t *p_mgr,
extension_t *p_ext )
{
int i_ret = lua_ExecuteFunction( p_mgr, p_ext, "trigger", LUA_END );
if( p_ext->p_sys->L )
{
vlclua_fd_cleanup( &p_ext->p_sys->dtable );
lua_close( p_ext->p_sys->L );
}
p_ext->p_sys->L = NULL;
return i_ret;
}
void vlclua_extension_set( lua_State *L, extension_t *p_ext )
{
lua_pushlightuserdata( L, vlclua_extension_set );
lua_pushlightuserdata( L, p_ext );
lua_rawset( L, LUA_REGISTRYINDEX );
}
extension_t *vlclua_extension_get( lua_State *L )
{
lua_pushlightuserdata( L, vlclua_extension_set );
lua_rawget( L, LUA_REGISTRYINDEX );
extension_t *p_ext = (extension_t*) lua_topointer( L, -1 );
lua_pop( L, 1 );
return p_ext;
}
int vlclua_extension_deactivate( lua_State *L )
{
extension_t *p_ext = vlclua_extension_get( L );
vlc_mutex_lock( &p_ext->p_sys->command_lock );
bool b_ret = QueueDeactivateCommand( p_ext );
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
return ( b_ret == true ) ? 1 : 0;
}
int vlclua_extension_keep_alive( lua_State *L )
{
extension_t *p_ext = vlclua_extension_get( L );
vlc_mutex_lock( &p_ext->p_sys->command_lock );
if( p_ext->p_sys->p_progress_id != NULL )
{
vlc_dialog_release( p_ext->p_sys->p_mgr, p_ext->p_sys->p_progress_id );
p_ext->p_sys->p_progress_id = NULL;
}
vlc_timer_schedule( p_ext->p_sys->timer, false, WATCH_TIMER_PERIOD,
VLC_TIMER_FIRE_ONCE );
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
return 1;
}
static int vlclua_extension_dialog_callback( vlc_object_t *p_this,
char const *psz_var,
vlc_value_t oldval,
vlc_value_t newval,
void *p_data )
{
( void ) psz_var;
( void ) oldval;
( void ) p_data;
extension_dialog_command_t *command = newval.p_address;
assert( command != NULL );
assert( command->p_dlg != NULL);
extension_t *p_ext = command->p_dlg->p_sys;
assert( p_ext != NULL );
extension_widget_t *p_widget = command->p_data;
switch( command->event )
{
case EXTENSION_EVENT_CLICK:
assert( p_widget != NULL );
PushCommandUnique( p_ext, CMD_CLICK, p_widget );
break;
case EXTENSION_EVENT_CLOSE:
PushCommandUnique( p_ext, CMD_CLOSE );
break;
default:
msg_Dbg( p_this, "Received unknown UI event %d, discarded",
command->event );
break;
}
return VLC_SUCCESS;
}
static void inputItemMetaChanged( const vlc_event_t *p_event,
void *data )
{
assert( p_event && p_event->type == vlc_InputItemMetaChanged );
extension_t *p_ext = ( extension_t* ) data;
assert( p_ext != NULL );
PushCommandUnique( p_ext, CMD_UPDATE_META );
}
static void WatchTimerCallback( void *data )
{
extension_t *p_ext = data;
extensions_manager_t *p_mgr = p_ext->p_sys->p_mgr;
vlc_mutex_lock( &p_ext->p_sys->command_lock );
for( struct command_t *cmd = p_ext->p_sys->command;
cmd != NULL;
cmd = cmd->next )
if( cmd->i_command == CMD_DEACTIVATE )
{ 
if( p_ext->p_sys->p_progress_id != NULL )
{
vlc_dialog_release( p_mgr, p_ext->p_sys->p_progress_id );
p_ext->p_sys->p_progress_id = NULL;
}
KillExtension( p_mgr, p_ext );
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
return;
}
if( p_ext->p_sys->p_progress_id == NULL )
{
p_ext->p_sys->p_progress_id =
vlc_dialog_display_progress( p_mgr, true, 0.0,
_( "Yes" ),
_( "Extension not responding!" ),
_( "Extension '%s' does not respond.\n"
"Do you want to kill it now? " ),
p_ext->psz_title );
if( p_ext->p_sys->p_progress_id == NULL )
{
KillExtension( p_mgr, p_ext );
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
return;
}
vlc_timer_schedule( p_ext->p_sys->timer, false, VLC_TICK_FROM_MS(100),
VLC_TIMER_FIRE_ONCE );
}
else
{
if( vlc_dialog_is_cancelled( p_mgr, p_ext->p_sys->p_progress_id ) )
{
vlc_dialog_release( p_mgr, p_ext->p_sys->p_progress_id );
p_ext->p_sys->p_progress_id = NULL;
KillExtension( p_mgr, p_ext );
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
return;
}
vlc_timer_schedule( p_ext->p_sys->timer, false, VLC_TICK_FROM_MS(100),
VLC_TIMER_FIRE_ONCE );
}
vlc_mutex_unlock( &p_ext->p_sys->command_lock );
}
