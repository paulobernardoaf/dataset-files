void vlclua_set_object( lua_State *, void *id, void *value );
void *vlclua_get_object( lua_State *, void *id );
vlc_object_t * vlclua_get_this( lua_State * );
int vlclua_push_ret( lua_State *, int );
