





















#if !defined(VLC_LUA_LIBS_H)
#define VLC_LUA_LIBS_H

#include "vlc.h"

void luaopen_config( lua_State * );
void luaopen_dialog( lua_State *, void * );
void luaopen_httpd( lua_State * );
void luaopen_input( lua_State * );
void luaopen_msg( lua_State * );
void luaopen_misc( lua_State * );
void luaopen_object( lua_State * );
void luaopen_osd( lua_State * );
void luaopen_playlist( lua_State * );
void luaopen_sd_sd( lua_State * );
void luaopen_sd_intf( lua_State * );
void luaopen_stream( lua_State * );
void luaopen_strings( lua_State * );
void luaopen_variables( lua_State * );
void luaopen_video( lua_State * );
void luaopen_vlm( lua_State * );
void luaopen_volume( lua_State * );
void luaopen_gettext( lua_State * );
void luaopen_input_item( lua_State *L, input_item_t *item );
void luaopen_xml( lua_State *L );
void luaopen_equalizer( lua_State *L );
void luaopen_vlcio( lua_State *L );
void luaopen_errno( lua_State *L );
void luaopen_rand( lua_State *L );
#if defined(_WIN32)
void luaopen_win( lua_State *L );
#endif

int vlclua_url_parse( lua_State *L );
int vlclua_input_item_get( lua_State *L, input_item_t *p_item );

#endif
