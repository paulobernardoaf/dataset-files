#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vlc_common.h>
#include <vlc_access.h>
#include "vlc.h"
#include "libs.h"
struct vlclua_playlist
{
lua_State *L;
char *filename;
char *access;
const char *path;
};
static int vlclua_demux_peek( lua_State *L )
{
stream_t *s = (stream_t *)vlclua_get_this(L);
int n = luaL_checkinteger( L, 1 );
const uint8_t *p_peek;
ssize_t val = vlc_stream_Peek(s->s, &p_peek, n);
if (val > 0)
lua_pushlstring(L, (const char *)p_peek, val);
else
lua_pushnil( L );
return 1;
}
static int vlclua_demux_read( lua_State *L )
{
stream_t *s = (stream_t *)vlclua_get_this(L);
int n = luaL_checkinteger( L, 1 );
char *buf = malloc(n);
if (buf != NULL)
{
ssize_t val = vlc_stream_Read(s->s, buf, n);
if (val > 0)
lua_pushlstring(L, buf, val);
else
lua_pushnil( L );
free(buf);
}
else
lua_pushnil( L );
return 1;
}
static int vlclua_demux_readline( lua_State *L )
{
stream_t *s = (stream_t *)vlclua_get_this(L);
char *line = vlc_stream_ReadLine(s->s);
if (line != NULL)
{
lua_pushstring(L, line);
free(line);
}
else
lua_pushnil( L );
return 1;
}
static const luaL_Reg p_reg[] =
{
{ "peek", vlclua_demux_peek },
{ NULL, NULL }
};
static const luaL_Reg p_reg_parse[] =
{
{ "read", vlclua_demux_read },
{ "readline", vlclua_demux_readline },
{ NULL, NULL }
};
static int probe_luascript(vlc_object_t *obj, const char *filename,
const luabatch_context_t *ctx)
{
stream_t *s = (stream_t *)obj;
struct vlclua_playlist *sys = s->p_sys;
lua_State *L = luaL_newstate();
if( !L )
return VLC_ENOMEM;
sys->L = L;
luaL_openlibs( L ); 
vlclua_set_this(L, s);
luaL_register_namespace( L, "vlc", p_reg );
luaopen_msg( L );
luaopen_strings( L );
luaopen_stream( L );
luaopen_variables( L );
luaopen_xml( L );
if (sys->path != NULL)
lua_pushstring(L, sys->path);
else
lua_pushnil(L);
lua_setfield( L, -2, "path" );
if (sys->access != NULL)
lua_pushstring(L, sys->access);
else
lua_pushnil(L);
lua_setfield( L, -2, "access" );
lua_pop( L, 1 );
if (vlclua_add_modules_path(L, filename))
{
msg_Warn(s, "error setting the module search path for %s", filename);
goto error;
}
if (vlclua_dofile(VLC_OBJECT(s), L, filename))
{
msg_Warn(s, "error loading script %s: %s", filename,
lua_tostring(L, lua_gettop(L)));
goto error;
}
lua_getglobal( L, "probe" );
if( !lua_isfunction( L, -1 ) )
{
msg_Warn(s, "error running script %s: function %s(): %s",
filename, "probe", "not found");
goto error;
}
if( lua_pcall( L, 0, 1, 0 ) )
{
msg_Warn(s, "error running script %s: function %s(): %s",
filename, "probe", lua_tostring(L, lua_gettop(L)));
goto error;
}
if( lua_gettop( L ) )
{
if( lua_toboolean( L, 1 ) )
{
msg_Dbg(s, "Lua playlist script %s's "
"probe() function was successful", filename );
lua_pop( L, 1 );
sys->filename = strdup(filename);
return VLC_SUCCESS;
}
}
(void) ctx;
error:
lua_pop( L, 1 );
lua_close(sys->L);
return VLC_EGENERIC;
}
static int ReadDir(stream_t *s, input_item_node_t *node)
{
struct vlclua_playlist *sys = s->p_sys;
lua_State *L = sys->L;
luaL_register_namespace( L, "vlc", p_reg_parse );
lua_getglobal( L, "parse" );
if( !lua_isfunction( L, -1 ) )
{
msg_Warn(s, "error running script %s: function %s(): %s",
sys->filename, "parse", "not found");
return VLC_ENOITEM;
}
if( lua_pcall( L, 0, 1, 0 ) )
{
msg_Warn(s, "error running script %s: function %s(): %s",
sys->filename, "parse", lua_tostring(L, lua_gettop(L)));
return VLC_ENOITEM;
}
if (!lua_gettop(L))
{
msg_Err(s, "script went completely foobar");
return VLC_ENOITEM;
}
if (!lua_istable(L, -1))
{
msg_Warn(s, "Playlist should be a table.");
return VLC_ENOITEM;
}
lua_pushnil(L);
while (lua_next(L, -2))
{
input_item_t *item = vlclua_read_input_item(VLC_OBJECT(s), L);
if (item != NULL)
{
char *url = input_item_GetURL(item);
if (url == NULL && s->psz_url != NULL)
input_item_SetURL(item, s->psz_url);
free(url);
input_item_node_AppendItem(node, item);
input_item_Release(item);
}
lua_pop(L, 1);
}
return VLC_SUCCESS;
}
int Import_LuaPlaylist(vlc_object_t *obj)
{
if( lua_Disabled( obj ) )
return VLC_EGENERIC;
stream_t *s = (stream_t *)obj;
if( s->s->pf_readdir != NULL )
return VLC_EGENERIC;
struct vlclua_playlist *sys = malloc(sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
s->p_sys = sys;
sys->access = NULL;
sys->path = NULL;
if (s->psz_url != NULL)
{ 
const char *p = strstr(s->psz_url, "://");
if (p != NULL)
{
sys->access = strndup(s->psz_url, p - s->psz_url);
sys->path = p + 3;
}
}
int ret = vlclua_scripts_batch_execute(VLC_OBJECT(s), "playlist",
probe_luascript, NULL);
if (ret != VLC_SUCCESS)
{
free(sys->access);
free(sys);
return ret;
}
s->pf_readdir = ReadDir;
s->pf_control = access_vaDirectoryControlHelper;
return VLC_SUCCESS;
}
void Close_LuaPlaylist(vlc_object_t *obj)
{
stream_t *s = (stream_t *)obj;
struct vlclua_playlist *sys = s->p_sys;
free(sys->filename);
assert(sys->L != NULL);
lua_close(sys->L);
free(sys->access);
free(sys);
}
