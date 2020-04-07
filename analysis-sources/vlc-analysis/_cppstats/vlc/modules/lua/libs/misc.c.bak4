


























#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_meta.h>
#include <vlc_interface.h>
#include <vlc_actions.h>
#include <vlc_interrupt.h>
#include <vlc_rand.h>

#include "../vlc.h"
#include "../libs.h"
#include "misc.h"




void vlclua_set_object( lua_State *L, void *id, void *value )
{
lua_pushlightuserdata( L, id );
lua_pushlightuserdata( L, value );
lua_rawset( L, LUA_REGISTRYINDEX );
}

void *vlclua_get_object( lua_State *L, void *id )
{
lua_pushlightuserdata( L, id );
lua_rawget( L, LUA_REGISTRYINDEX );
const void *p = lua_topointer( L, -1 );
lua_pop( L, 1 );
return (void *)p;
}

#undef vlclua_set_this
void vlclua_set_this( lua_State *L, vlc_object_t *p_this )
{
vlclua_set_object( L, vlclua_set_this, p_this );
}

vlc_object_t * vlclua_get_this( lua_State *L )
{
return vlclua_get_object( L, vlclua_set_this );
}




int vlclua_push_ret( lua_State *L, int i_error )
{
lua_pushnumber( L, i_error );

int err;

switch( i_error )
{
case VLC_SUCCESS: err = 0; break;
case VLC_ENOMEM: err = ENOMEM; break;
case VLC_ETIMEOUT: err = ETIMEDOUT; break;
case VLC_EBADVAR: err = EINVAL; break;
case VLC_ENOMOD: err = ENOENT; break;
case VLC_ENOOBJ: err = ENOENT; break;
case VLC_ENOVAR: err = ENOENT; break;
case VLC_EGENERIC:
lua_pushstring( L, "generic error" );
return 2;
default:
lua_pushstring( L, "unknown error" );
return 2;
}

lua_pushstring( L, vlc_strerror_c(err) );
return 2;
}




static int vlclua_version( lua_State *L )
{
lua_pushstring( L, VERSION_MESSAGE );
return 1;
}




static int vlclua_copyright( lua_State *L )
{
lua_pushliteral( L, COPYRIGHT_MESSAGE );
return 1;
}




static int vlclua_license( lua_State *L )
{
lua_pushstring( L, LICENSE_MSG );
return 1;
}




static int vlclua_quit( lua_State *L )
{
vlc_object_t *p_this = vlclua_get_this( L );


libvlc_Quit( vlc_object_instance(p_this) );
return 0;
}

static int vlclua_mdate( lua_State *L )
{
lua_pushnumber( L, vlc_tick_now() );
return 1;
}

static int vlclua_mwait( lua_State *L )
{
double f = luaL_checknumber( L, 1 );

vlc_interrupt_t *oint = vlclua_set_interrupt( L );
int ret = vlc_mwait_i11e( llround(f) );

vlc_interrupt_set( oint );
if( ret )
return luaL_error( L, "Interrupted." );
return 0;
}

static int vlclua_action_id( lua_State *L )
{
vlc_action_id_t i_key = vlc_actions_get_id( luaL_checkstring( L, 1 ) );
if (i_key == 0)
return 0;
lua_pushnumber( L, i_key );
return 1;
}




static const luaL_Reg vlclua_misc_reg[] = {
{ "version", vlclua_version },
{ "copyright", vlclua_copyright },
{ "license", vlclua_license },

{ "action_id", vlclua_action_id },

{ "mdate", vlclua_mdate },
{ "mwait", vlclua_mwait },

{ "quit", vlclua_quit },

{ NULL, NULL }
};

void luaopen_misc( lua_State *L )
{
lua_newtable( L );
luaL_register( L, NULL, vlclua_misc_reg );
lua_setfield( L, -2, "misc" );
}
