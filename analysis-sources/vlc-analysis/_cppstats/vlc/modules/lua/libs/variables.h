int vlclua_var_toggle_or_set( lua_State *, vlc_object_t *, const char * );
#define vlclua_var_toggle_or_set( a, b, c ) vlclua_var_toggle_or_set( a, VLC_OBJECT( b ), c )
