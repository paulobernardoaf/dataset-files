#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_vout.h>
#include <vlc_playlist.h>
#include <vlc_player.h>
#include "../vlc.h"
#include "../libs.h"
#include "input.h"
typedef struct vlclua_object {
void *object;
void (*release)(void *);
} vlclua_object_t;
static int vlclua_release_vlc_object(lua_State *L)
{
vlclua_object_t *p_luaobj = luaL_checkudata(L, 1, "vlc_object");
lua_pop(L, 1);
if (p_luaobj->release)
p_luaobj->release(p_luaobj->object);
return 0;
}
static int vlclua_push_vlc_object(lua_State *L, void *p_obj,
void (*release)(void *))
{
vlclua_object_t *udata =
(vlclua_object_t *)lua_newuserdata(L, sizeof (vlclua_object_t));
udata->object = p_obj;
udata->release = release;
if (luaL_newmetatable(L, "vlc_object"))
{
lua_pushliteral(L, "none of your business");
lua_setfield(L, -2, "__metatable");
lua_pushcfunction(L, vlclua_release_vlc_object);
lua_setfield(L, -2, "__gc");
}
lua_setmetatable(L, -2);
return 1;
}
static int vlclua_get_libvlc( lua_State *L )
{
libvlc_int_t *p_libvlc = vlc_object_instance(vlclua_get_this( L ));
vlclua_push_vlc_object(L, p_libvlc, NULL);
return 1;
}
static int vlclua_get_playlist( lua_State *L )
{
vlc_playlist_t *playlist = vlclua_get_playlist_internal(L);
if (playlist)
lua_pushlightuserdata(L, playlist);
else
lua_pushnil(L);
return 1;
}
static int vlclua_get_player( lua_State *L )
{
vlc_player_t *player = vlclua_get_player_internal(L);
if (player)
lua_pushlightuserdata(L, player);
else
lua_pushnil(L);
return 1;
}
static void do_vout_Release( void *vout )
{
vout_Release( vout );
}
static int vlclua_get_vout( lua_State *L )
{
vout_thread_t *vout = vlclua_get_vout_internal(L);
if (vout)
vlclua_push_vlc_object(L, vout, do_vout_Release);
else
lua_pushnil(L);
return 1;
}
static void do_aout_Release( void *aout )
{
aout_Release( aout );
}
static int vlclua_get_aout( lua_State *L )
{
audio_output_t *aout = vlclua_get_aout_internal(L);
if (aout)
vlclua_push_vlc_object(L, aout, do_aout_Release);
else
lua_pushnil(L);
return 1;
}
static const luaL_Reg vlclua_object_reg[] = {
{ "playlist", vlclua_get_playlist },
{ "player", vlclua_get_player },
{ "libvlc", vlclua_get_libvlc },
{ "vout", vlclua_get_vout},
{ "aout", vlclua_get_aout},
{ NULL, NULL }
};
void luaopen_object( lua_State *L )
{
lua_newtable( L );
luaL_register( L, NULL, vlclua_object_reg );
lua_setfield( L, -2, "object" );
}
