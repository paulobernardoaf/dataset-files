#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_services_discovery.h>
#include "vlc.h"
#include "libs.h"
static void *Run( void * );
static int DoSearch( services_discovery_t *p_sd, const char *psz_query );
static int FillDescriptor( services_discovery_t *, services_discovery_descriptor_t * );
static int Control( services_discovery_t *p_sd, int i_command, va_list args );
static const char *vlclua_sd_description( vlc_object_t *obj, lua_State *L,
const char *filename )
{
lua_getglobal( L, "descriptor" );
if( !lua_isfunction( L, -1 ) )
{
msg_Warn( obj, "No 'descriptor' function in '%s'", filename );
lua_pop( L, 1 );
return NULL;
}
if( lua_pcall( L, 0, 1, 0 ) )
{
msg_Warn( obj, "Error while running script %s, "
"function descriptor(): %s", filename,
lua_tostring( L, -1 ) );
lua_pop( L, 1 );
return NULL;
}
lua_getfield( L, -1, "title" );
if ( !lua_isstring( L, -1 ) )
{
msg_Warn( obj, "'descriptor' function in '%s' returned no title",
filename );
lua_pop( L, 2 );
return NULL;
}
return lua_tostring( L, -1 );
}
int vlclua_probe_sd( vlc_object_t *obj, const char *name )
{
vlc_probe_t *probe = (vlc_probe_t *)obj;
char *filename = vlclua_find_file( "sd", name );
if( filename == NULL )
{
msg_Err( probe, "Couldn't probe lua services discovery script \"%s\".",
name );
return VLC_PROBE_CONTINUE;
}
lua_State *L = luaL_newstate();
if( !L )
{
msg_Err( probe, "Could not create new Lua State" );
free( filename );
return VLC_ENOMEM;
}
luaL_openlibs( L );
if( vlclua_add_modules_path( L, filename ) )
{
msg_Err( probe, "Error while setting the module search path for %s",
filename );
lua_close( L );
free( filename );
return VLC_ENOMEM;
}
if( vlclua_dofile( obj, L, filename ) )
{
msg_Err( probe, "Error loading script %s: %s", filename,
lua_tostring( L, -1 ) );
lua_close( L );
free( filename );
return VLC_PROBE_CONTINUE;
}
const char *description = vlclua_sd_description( obj, L, filename );
if( description == NULL )
description = name;
int r = VLC_ENOMEM;
char *name_esc = config_StringEscape( name );
char *chain;
if( asprintf( &chain, "lua{sd='%s'}", name_esc ) != -1 )
{
r = vlc_sd_probe_Add( probe, chain, description, SD_CAT_INTERNET );
free( chain );
}
free( name_esc );
lua_close( L );
free( filename );
return r;
}
static const char * const ppsz_sd_options[] = { "sd", NULL };
typedef struct
{
lua_State *L;
char *psz_filename;
vlc_thread_t thread;
vlc_mutex_t lock;
vlc_cond_t cond;
char **ppsz_query;
int i_query;
} services_discovery_sys_t;
static const luaL_Reg p_reg[] = { { NULL, NULL } };
int Open_LuaSD( vlc_object_t *p_this )
{
if( lua_Disabled( p_this ) )
return VLC_EGENERIC;
services_discovery_t *p_sd = ( services_discovery_t * )p_this;
services_discovery_sys_t *p_sys;
lua_State *L = NULL;
char *psz_name;
if( !( p_sys = malloc( sizeof( services_discovery_sys_t ) ) ) )
return VLC_ENOMEM;
if( !strcmp( p_sd->psz_name, "lua" ) ||
!strcmp( p_sd->psz_name, "luasd" ) )
{
config_ChainParse( p_sd, "lua-", ppsz_sd_options, p_sd->p_cfg );
psz_name = var_GetString( p_sd, "lua-sd" );
}
else
{
psz_name = strdup(p_sd->psz_name);
}
p_sd->p_sys = p_sys;
p_sd->pf_control = Control;
p_sys->psz_filename = vlclua_find_file( "sd", psz_name );
if( !p_sys->psz_filename )
{
msg_Err( p_sd, "Couldn't find lua services discovery script \"%s\".",
psz_name );
free( psz_name );
goto error;
}
free( psz_name );
L = luaL_newstate();
if( !L )
{
msg_Err( p_sd, "Could not create new Lua State" );
goto error;
}
vlclua_set_this( L, p_sd );
luaL_openlibs( L );
luaL_register_namespace( L, "vlc", p_reg );
luaopen_input( L );
luaopen_msg( L );
luaopen_object( L );
luaopen_sd_sd( L );
luaopen_strings( L );
luaopen_variables( L );
luaopen_stream( L );
luaopen_gettext( L );
luaopen_xml( L );
lua_pop( L, 1 );
if( vlclua_add_modules_path( L, p_sys->psz_filename ) )
{
msg_Warn( p_sd, "Error while setting the module search path for %s",
p_sys->psz_filename );
goto error;
}
if( vlclua_dofile( VLC_OBJECT(p_sd), L, p_sys->psz_filename ) )
{
msg_Err( p_sd, "Error loading script %s: %s", p_sys->psz_filename,
lua_tostring( L, lua_gettop( L ) ) );
lua_pop( L, 1 );
goto error;
}
p_sd->description = vlclua_sd_description( VLC_OBJECT(p_sd), L,
p_sys->psz_filename );
if( p_sd->description == NULL )
p_sd->description = p_sd->psz_name;
p_sys->L = L;
vlc_mutex_init( &p_sys->lock );
vlc_cond_init( &p_sys->cond );
TAB_INIT( p_sys->i_query, p_sys->ppsz_query );
if( vlc_clone( &p_sys->thread, Run, p_sd, VLC_THREAD_PRIORITY_LOW ) )
{
TAB_CLEAN( p_sys->i_query, p_sys->ppsz_query );
goto error;
}
return VLC_SUCCESS;
error:
if( L )
lua_close( L );
free( p_sys->psz_filename );
free( p_sys );
return VLC_EGENERIC;
}
void Close_LuaSD( vlc_object_t *p_this )
{
services_discovery_t *p_sd = ( services_discovery_t * )p_this;
services_discovery_sys_t *p_sys = p_sd->p_sys;
vlc_cancel( p_sys->thread );
vlc_join( p_sys->thread, NULL );
for( int i = 0; i < p_sys->i_query; i++ )
free( p_sys->ppsz_query[i] );
TAB_CLEAN( p_sys->i_query, p_sys->ppsz_query );
free( p_sys->psz_filename );
lua_close( p_sys->L );
free( p_sys );
}
static void* Run( void *data )
{
services_discovery_t *p_sd = ( services_discovery_t * )data;
services_discovery_sys_t *p_sys = p_sd->p_sys;
lua_State *L = p_sys->L;
int cancel = vlc_savecancel();
lua_getglobal( L, "main" );
if( !lua_isfunction( L, lua_gettop( L ) ) || lua_pcall( L, 0, 1, 0 ) )
{
msg_Err( p_sd, "Error while running script %s, "
"function main(): %s", p_sys->psz_filename,
lua_tostring( L, lua_gettop( L ) ) );
lua_pop( L, 1 );
vlc_restorecancel( cancel );
return NULL;
}
msg_Dbg( p_sd, "LuaSD script loaded: %s", p_sys->psz_filename );
lua_gc( L, LUA_GCCOLLECT, 0 );
vlc_restorecancel( cancel );
vlc_mutex_lock( &p_sys->lock );
mutex_cleanup_push( &p_sys->lock );
for( ;; )
{
if( !p_sys->i_query )
{
vlc_cond_wait( &p_sys->cond, &p_sys->lock );
continue;
}
char *psz_query = p_sys->ppsz_query[p_sys->i_query - 1];
TAB_ERASE(p_sys->i_query, p_sys->ppsz_query, p_sys->i_query - 1);
vlc_mutex_unlock( &p_sys->lock );
cancel = vlc_savecancel();
DoSearch( p_sd, psz_query );
free( psz_query );
lua_gc( L, LUA_GCCOLLECT, 0 );
vlc_restorecancel( cancel );
vlc_mutex_lock( &p_sys->lock );
}
vlc_cleanup_pop();
vlc_assert_unreachable();
}
static int Control( services_discovery_t *p_sd, int i_command, va_list args )
{
services_discovery_sys_t *p_sys = p_sd->p_sys;
switch( i_command )
{
case SD_CMD_SEARCH:
{
const char *psz_query = va_arg( args, const char * );
vlc_mutex_lock( &p_sys->lock );
TAB_APPEND( p_sys->i_query, p_sys->ppsz_query, strdup( psz_query ) );
vlc_cond_signal( &p_sys->cond );
vlc_mutex_unlock( &p_sys->lock );
break;
}
case SD_CMD_DESCRIPTOR:
{
services_discovery_descriptor_t *p_desc = va_arg( args,
services_discovery_descriptor_t * );
return FillDescriptor( p_sd, p_desc );
}
}
return VLC_SUCCESS;
}
static int DoSearch( services_discovery_t *p_sd, const char *psz_query )
{
services_discovery_sys_t *p_sys = p_sd->p_sys;
lua_State *L = p_sys->L;
lua_getglobal( L, "search" );
if( !lua_isfunction( L, lua_gettop( L ) ) )
{
msg_Err( p_sd, "The script '%s' does not define any 'search' function",
p_sys->psz_filename );
lua_pop( L, 1 );
return VLC_EGENERIC;
}
lua_pushstring( L, psz_query );
if( lua_pcall( L, 1, 0, 0 ) )
{
msg_Err( p_sd, "Error while running the script '%s': %s",
p_sys->psz_filename, lua_tostring( L, lua_gettop( L ) ) );
lua_pop( L, 1 );
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static const char *const ppsz_capabilities[] = {
"search",
NULL
};
static int FillDescriptor( services_discovery_t *p_sd,
services_discovery_descriptor_t *p_desc )
{
services_discovery_sys_t *p_sys = p_sd->p_sys;
int i_ret = VLC_EGENERIC;
lua_State *L = luaL_newstate();
if( vlclua_dofile( VLC_OBJECT(p_sd), L, p_sys->psz_filename ) )
{
msg_Err( p_sd, "Error loading script %s: %s", p_sys->psz_filename,
lua_tostring( L, -1 ) );
goto end;
}
lua_getglobal( L, "descriptor" );
if( !lua_isfunction( L, -1 ) || lua_pcall( L, 0, 1, 0 ) )
{
msg_Warn( p_sd, "Error getting the descriptor in '%s': %s",
p_sys->psz_filename, lua_tostring( L, -1 ) );
goto end;
}
lua_getfield( L, -1, "short_description" );
p_desc->psz_short_desc = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "icon" );
p_desc->psz_icon_url = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "url" );
p_desc->psz_url = luaL_strdupornull( L, -1 );
lua_pop( L, 1 );
lua_getfield( L, -1, "capabilities" );
p_desc->i_capabilities = 0;
if( lua_istable( L, -1 ) )
{
lua_pushnil( L );
while( lua_next( L, -2 ) != 0 )
{
const char *psz_cap = luaL_checkstring( L, -1 );
int i_cap = 0;
const char *psz_iter;
for( psz_iter = *ppsz_capabilities; psz_iter;
psz_iter = ppsz_capabilities[ ++i_cap ] )
{
if( !strcmp( psz_iter, psz_cap ) )
{
p_desc->i_capabilities |= 1 << i_cap;
break;
}
}
lua_pop( L, 1 );
if( !psz_iter )
msg_Warn( p_sd, "Services discovery capability '%s' unknown in "
"script '%s'", psz_cap, p_sys->psz_filename );
}
}
lua_pop( L, 1 );
i_ret = VLC_SUCCESS;
end:
lua_close( L );
return i_ret;
}
