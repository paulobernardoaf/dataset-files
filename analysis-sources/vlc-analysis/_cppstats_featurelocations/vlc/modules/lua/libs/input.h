





















#if !defined(VLC_LUA_INPUT_H)
#define VLC_LUA_INPUT_H

#include "../vlc.h"

vlc_player_t *vlclua_get_player_internal(lua_State *);
vout_thread_t *vlclua_get_vout_internal(lua_State *L);
audio_output_t *vlclua_get_aout_internal(lua_State *L);

#endif
