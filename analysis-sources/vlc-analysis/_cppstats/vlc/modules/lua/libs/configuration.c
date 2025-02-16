#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "../vlc.h"
#include "../libs.h"
static int vlclua_config_get( lua_State *L )
{
const char *psz_name = luaL_checkstring( L, 1 );
switch( config_GetType( psz_name ) )
{
case VLC_VAR_STRING:
{
char *psz = config_GetPsz( psz_name );
lua_pushstring( L, psz );
free( psz );
break;
}
case VLC_VAR_INTEGER:
lua_pushinteger( L, config_GetInt( psz_name ) );
break;
case VLC_VAR_BOOL:
lua_pushboolean( L, config_GetInt( psz_name ) );
break;
case VLC_VAR_FLOAT:
lua_pushnumber( L, config_GetFloat( psz_name ) );
break;
default:
return vlclua_error( L );
}
return 1;
}
static int vlclua_config_set( lua_State *L )
{
const char *psz_name = luaL_checkstring( L, 1 );
switch( config_GetType( psz_name ) )
{
case VLC_VAR_STRING:
config_PutPsz( psz_name, luaL_checkstring( L, 2 ) );
break;
case VLC_VAR_INTEGER:
config_PutInt( psz_name, luaL_checkinteger( L, 2 ) );
break;
case VLC_VAR_BOOL:
config_PutInt( psz_name, luaL_checkboolean( L, 2 ) );
break;
case VLC_VAR_FLOAT:
config_PutFloat( psz_name, luaL_checknumber( L, 2 ) );
break;
default:
return vlclua_error( L );
}
return 0;
}
static int vlclua_datadir( lua_State *L )
{
char *psz_data = config_GetSysPath(VLC_PKG_DATA_DIR, NULL);
lua_pushstring( L, psz_data );
free( psz_data );
return 1;
}
static int vlclua_userdatadir( lua_State *L )
{
char *dir = config_GetUserDir( VLC_USERDATA_DIR );
lua_pushstring( L, dir );
free( dir );
return 1;
}
static int vlclua_homedir( lua_State *L )
{
char *home = config_GetUserDir( VLC_HOME_DIR );
lua_pushstring( L, home );
free( home );
return 1;
}
static int vlclua_configdir( lua_State *L )
{
char *dir = config_GetUserDir( VLC_CONFIG_DIR );
lua_pushstring( L, dir );
free( dir );
return 1;
}
static int vlclua_cachedir( lua_State *L )
{
char *dir = config_GetUserDir( VLC_CACHE_DIR );
lua_pushstring( L, dir );
free( dir );
return 1;
}
static int vlclua_datadir_list( lua_State *L )
{
const char *psz_dirname = luaL_checkstring( L, 1 );
char **ppsz_dir_list = NULL;
int i = 1;
if( vlclua_dir_list( psz_dirname, &ppsz_dir_list )
!= VLC_SUCCESS )
return 0;
lua_newtable( L );
for( char **ppsz_dir = ppsz_dir_list; *ppsz_dir; ppsz_dir++ )
{
lua_pushstring( L, *ppsz_dir );
lua_rawseti( L, -2, i );
i ++;
}
vlclua_dir_list_free( ppsz_dir_list );
return 1;
}
static const luaL_Reg vlclua_config_reg[] = {
{ "get", vlclua_config_get },
{ "set", vlclua_config_set },
{ "datadir", vlclua_datadir },
{ "userdatadir", vlclua_userdatadir },
{ "homedir", vlclua_homedir },
{ "configdir", vlclua_configdir },
{ "cachedir", vlclua_cachedir },
{ "datadir_list", vlclua_datadir_list },
{ NULL, NULL }
};
void luaopen_config( lua_State *L )
{
lua_newtable( L );
luaL_register( L, NULL, vlclua_config_reg );
lua_setfield( L, -2, "config" );
}
