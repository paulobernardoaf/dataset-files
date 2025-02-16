
























#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_dialog.h>
#include <vlc_extensions.h>

#include "../vlc.h"
#include "../libs.h"

#include "assert.h"






static int vlclua_dialog_create( lua_State *L );
static int vlclua_dialog_delete( lua_State *L );
static int vlclua_dialog_show( lua_State *L );
static int vlclua_dialog_hide( lua_State *L );
static int vlclua_dialog_set_title( lua_State *L );
static int vlclua_dialog_update( lua_State *L );
static void lua_SetDialogUpdate( lua_State *L, int flag );
static int lua_GetDialogUpdate( lua_State *L );
int lua_DialogFlush( lua_State *L );

static int vlclua_dialog_add_button( lua_State *L );
static int vlclua_dialog_add_label( lua_State *L );
static int vlclua_dialog_add_html( lua_State *L );
static int vlclua_dialog_add_text_inner( lua_State *L, int );
static inline int vlclua_dialog_add_text_input( lua_State *L )
{
return vlclua_dialog_add_text_inner( L, EXTENSION_WIDGET_TEXT_FIELD );
}
static inline int vlclua_dialog_add_password( lua_State *L )
{
return vlclua_dialog_add_text_inner( L, EXTENSION_WIDGET_PASSWORD );
}
static inline int vlclua_dialog_add_html( lua_State *L )
{
return vlclua_dialog_add_text_inner( L, EXTENSION_WIDGET_HTML );
}
static int vlclua_dialog_add_check_box( lua_State *L );
static int vlclua_dialog_add_list( lua_State *L );
static int vlclua_dialog_add_dropdown( lua_State *L );
static int vlclua_dialog_add_image( lua_State *L );
static int vlclua_dialog_add_spin_icon( lua_State *L );
static int vlclua_create_widget_inner( lua_State *L, int i_args,
extension_widget_t *p_widget);

static int vlclua_dialog_delete_widget( lua_State *L );


static int vlclua_widget_set_text( lua_State *L );
static int vlclua_widget_get_text( lua_State *L );
static int vlclua_widget_set_checked( lua_State *L );
static int vlclua_widget_get_checked( lua_State *L );
static int vlclua_widget_add_value( lua_State *L );
static int vlclua_widget_get_value( lua_State *L );
static int vlclua_widget_clear( lua_State *L );
static int vlclua_widget_get_selection( lua_State *L );
static int vlclua_widget_animate( lua_State *L );
static int vlclua_widget_stop( lua_State *L );


static void AddWidget( extension_dialog_t *p_dialog,
extension_widget_t *p_widget );
static int DeleteWidget( extension_dialog_t *p_dialog,
extension_widget_t *p_widget );

static const luaL_Reg vlclua_dialog_reg[] = {
{ "show", vlclua_dialog_show },
{ "hide", vlclua_dialog_hide },
{ "delete", vlclua_dialog_delete },
{ "set_title", vlclua_dialog_set_title },
{ "update", vlclua_dialog_update },

{ "add_button", vlclua_dialog_add_button },
{ "add_label", vlclua_dialog_add_label },
{ "add_html", vlclua_dialog_add_html },
{ "add_text_input", vlclua_dialog_add_text_input },
{ "add_password", vlclua_dialog_add_password },
{ "add_check_box", vlclua_dialog_add_check_box },
{ "add_dropdown", vlclua_dialog_add_dropdown },
{ "add_list", vlclua_dialog_add_list },
{ "add_image", vlclua_dialog_add_image },
{ "add_spin_icon", vlclua_dialog_add_spin_icon },

{ "del_widget", vlclua_dialog_delete_widget },
{ NULL, NULL }
};

static const luaL_Reg vlclua_widget_reg[] = {
{ "set_text", vlclua_widget_set_text },
{ "get_text", vlclua_widget_get_text },
{ "set_checked", vlclua_widget_set_checked },
{ "get_checked", vlclua_widget_get_checked },
{ "add_value", vlclua_widget_add_value },
{ "get_value", vlclua_widget_get_value },
{ "clear", vlclua_widget_clear },
{ "get_selection", vlclua_widget_get_selection },
{ "animate", vlclua_widget_animate },
{ "stop", vlclua_widget_stop },
{ NULL, NULL }
};


static const char key_opaque = 'A',
key_update = 'B';







void luaopen_dialog( lua_State *L, void *opaque )
{
lua_getglobal( L, "vlc" );
lua_pushcfunction( L, vlclua_dialog_create );
lua_setfield( L, -2, "dialog" );




lua_pushlightuserdata( L, (void*) &key_opaque );
lua_pushlightuserdata( L, opaque );
lua_settable( L, LUA_REGISTRYINDEX );


lua_SetDialogUpdate( L, 0 );
}

static int vlclua_dialog_create( lua_State *L )
{
if( !lua_isstring( L, 1 ) )
return luaL_error( L, "vlc.dialog() usage: (title)" );
const char *psz_title = luaL_checkstring( L, 1 );

vlc_object_t *p_this = vlclua_get_this( L );

extension_dialog_t *p_dlg = calloc( 1, sizeof( extension_dialog_t ) );
if( !p_dlg )
return 0; 

lua_getglobal( L, "vlc" );
lua_getfield( L, -1, "__dialog" );
if( lua_topointer( L, lua_gettop( L ) ) != NULL )
{
free( p_dlg );
return luaL_error( L, "Only one dialog allowed per extension!" );
}

p_dlg->p_object = p_this;
p_dlg->psz_title = strdup( psz_title );
p_dlg->b_kill = false;
ARRAY_INIT( p_dlg->widgets );


lua_pushlightuserdata( L, (void*) &key_opaque );
lua_gettable( L, LUA_REGISTRYINDEX );
p_dlg->p_sys = (void*) lua_topointer( L, -1 ); 
lua_pop( L, 1 );

vlc_mutex_init( &p_dlg->lock );
vlc_cond_init( &p_dlg->cond );




lua_getglobal( L, "vlc" );
lua_pushlightuserdata( L, p_dlg );
lua_setfield( L, -2, "__dialog" );
lua_pop( L, 1 );

extension_dialog_t **pp_dlg = lua_newuserdata( L, sizeof( extension_dialog_t* ) );
*pp_dlg = p_dlg;

if( luaL_newmetatable( L, "dialog" ) )
{
lua_newtable( L );
luaL_register( L, NULL, vlclua_dialog_reg );
lua_setfield( L, -2, "__index" );
lua_pushcfunction( L, vlclua_dialog_delete );
lua_setfield( L, -2, "__gc" );
}

lua_setmetatable( L, -2 );

msg_Dbg( p_this, "Creating dialog '%s'", psz_title );
lua_SetDialogUpdate( L, 0 );

return 1;
}

static int vlclua_dialog_delete( lua_State *L )
{
vlc_object_t *p_mgr = vlclua_get_this( L );


extension_dialog_t **pp_dlg =
(extension_dialog_t**) luaL_checkudata( L, 1, "dialog" );

if( !pp_dlg || !*pp_dlg )
return luaL_error( L, "Can't get pointer to dialog" );

extension_dialog_t *p_dlg = *pp_dlg;
*pp_dlg = NULL;


lua_getglobal( L, "vlc" );
lua_pushnil( L );
lua_setfield( L, -2, "__dialog" );

assert( !p_dlg->b_kill );


msg_Dbg( p_mgr, "Deleting dialog '%s'", p_dlg->psz_title );
p_dlg->b_kill = true;
lua_SetDialogUpdate( L, 0 ); 
vlc_ext_dialog_update( p_mgr, p_dlg );




msg_Dbg( p_mgr, "Waiting for the dialog to be deleted..." );
vlc_mutex_lock( &p_dlg->lock );
while( p_dlg->p_sys_intf != NULL )
{
vlc_cond_wait( &p_dlg->cond, &p_dlg->lock );
}
vlc_mutex_unlock( &p_dlg->lock );

free( p_dlg->psz_title );
p_dlg->psz_title = NULL;


extension_widget_t *p_widget;
ARRAY_FOREACH( p_widget, p_dlg->widgets )
{
if( !p_widget )
continue;
free( p_widget->psz_text );


struct extension_widget_value_t *p_value, *p_next;
for( p_value = p_widget->p_values; p_value != NULL; p_value = p_next )
{
p_next = p_value->p_next;
free( p_value->psz_text );
free( p_value );
}
free( p_widget );
}

ARRAY_RESET( p_dlg->widgets );


free( p_dlg );

return 1;
}


static int vlclua_dialog_show( lua_State *L )
{
extension_dialog_t **pp_dlg =
(extension_dialog_t**) luaL_checkudata( L, 1, "dialog" );
if( !pp_dlg || !*pp_dlg )
return luaL_error( L, "Can't get pointer to dialog" );
extension_dialog_t *p_dlg = *pp_dlg;

p_dlg->b_hide = false;
lua_SetDialogUpdate( L, 1 );

return 1;
}


static int vlclua_dialog_hide( lua_State *L )
{
extension_dialog_t **pp_dlg =
(extension_dialog_t**) luaL_checkudata( L, 1, "dialog" );
if( !pp_dlg || !*pp_dlg )
return luaL_error( L, "Can't get pointer to dialog" );
extension_dialog_t *p_dlg = *pp_dlg;

p_dlg->b_hide = true;
lua_SetDialogUpdate( L, 1 );

return 1;
}



static int vlclua_dialog_set_title( lua_State *L )
{
extension_dialog_t **pp_dlg =
(extension_dialog_t**) luaL_checkudata( L, 1, "dialog" );
if( !pp_dlg || !*pp_dlg )
return luaL_error( L, "Can't get pointer to dialog" );
extension_dialog_t *p_dlg = *pp_dlg;

vlc_mutex_lock( &p_dlg->lock );

const char *psz_title = luaL_checkstring( L, 2 );
free( p_dlg->psz_title );
p_dlg->psz_title = strdup( psz_title );

vlc_mutex_unlock( &p_dlg->lock );

lua_SetDialogUpdate( L, 1 );

return 1;
}


static int vlclua_dialog_update( lua_State *L )
{
vlc_object_t *p_mgr = vlclua_get_this( L );

extension_dialog_t **pp_dlg =
(extension_dialog_t**) luaL_checkudata( L, 1, "dialog" );
if( !pp_dlg || !*pp_dlg )
return luaL_error( L, "Can't get pointer to dialog" );
extension_dialog_t *p_dlg = *pp_dlg;


vlc_ext_dialog_update( p_mgr, p_dlg );


lua_SetDialogUpdate( L, 0 );

return 1;
}

static void lua_SetDialogUpdate( lua_State *L, int flag )
{

lua_pushlightuserdata( L, (void*) &key_update );
lua_pushinteger( L, flag );
lua_settable( L, LUA_REGISTRYINDEX );
}

static int lua_GetDialogUpdate( lua_State *L )
{

lua_pushlightuserdata( L, (void*) &key_update );
lua_gettable( L, LUA_REGISTRYINDEX );
return luaL_checkinteger( L, -1 );
}







int lua_DialogFlush( lua_State *L )
{
lua_getglobal( L, "vlc" );
lua_getfield( L, -1, "__dialog" );
extension_dialog_t *p_dlg = ( extension_dialog_t* )lua_topointer( L, -1 );

if( !p_dlg )
return VLC_SUCCESS;

int i_ret = VLC_SUCCESS;
if( lua_GetDialogUpdate( L ) )
{
i_ret = vlc_ext_dialog_update( vlclua_get_this( L ), p_dlg );
lua_SetDialogUpdate( L, 0 );
}

return i_ret;
}






static int vlclua_dialog_add_button( lua_State *L )
{

if( !lua_isstring( L, 2 ) || !lua_isfunction( L, 3 ) )
return luaL_error( L, "dialog:add_button usage: (text, func)" );

extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = EXTENSION_WIDGET_BUTTON;
p_widget->psz_text = strdup( luaL_checkstring( L, 2 ) );
lua_settop( L, 10 );
lua_pushlightuserdata( L, p_widget );
lua_pushvalue( L, 3 );
lua_settable( L, LUA_REGISTRYINDEX );
p_widget->p_sys = NULL;

return vlclua_create_widget_inner( L, 2, p_widget );
}






static int vlclua_dialog_add_label( lua_State *L )
{

if( !lua_isstring( L, 2 ) )
return luaL_error( L, "dialog:add_label usage: (text)" );
extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = EXTENSION_WIDGET_LABEL;
p_widget->psz_text = strdup( luaL_checkstring( L, 2 ) );

return vlclua_create_widget_inner( L, 1, p_widget );
}






static int vlclua_dialog_add_text_inner( lua_State *L, int i_type )
{

if( !lua_isstring( L, 2 ) && !lua_isnil( L, 2 ) )
return luaL_error( L, "dialog:add_text_input usage: (text = nil)" );

extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = i_type;
if( !lua_isnil( L, 2 ) )
p_widget->psz_text = strdup( luaL_checkstring( L, 2 ) );

return vlclua_create_widget_inner( L, 1, p_widget );
}






static int vlclua_dialog_add_check_box( lua_State *L )
{

if( !lua_isstring( L, 2 ) )
return luaL_error( L, "dialog:add_check_box usage: (text, checked)" );

extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = EXTENSION_WIDGET_CHECK_BOX;
p_widget->psz_text = strdup( luaL_checkstring( L, 2 ) );
p_widget->b_checked = lua_toboolean( L, 3 );

return vlclua_create_widget_inner( L, 2, p_widget );
}







static int vlclua_dialog_add_dropdown( lua_State *L )
{
extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = EXTENSION_WIDGET_DROPDOWN;

return vlclua_create_widget_inner( L, 0, p_widget );
}






static int vlclua_dialog_add_list( lua_State *L )
{
extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = EXTENSION_WIDGET_LIST;

return vlclua_create_widget_inner( L, 0, p_widget );
}






static int vlclua_dialog_add_image( lua_State *L )
{

if( !lua_isstring( L, 2 ) )
return luaL_error( L, "dialog:add_image usage: (filename)" );

extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = EXTENSION_WIDGET_IMAGE;
p_widget->psz_text = strdup( luaL_checkstring( L, 2 ) );

return vlclua_create_widget_inner( L, 1, p_widget );
}






static int vlclua_dialog_add_spin_icon( lua_State *L )
{

if( !lua_isstring( L, 2 ) )
return luaL_error( L, "dialog:add_image usage: (filename)" );

extension_widget_t *p_widget = calloc( 1, sizeof( extension_widget_t ) );
p_widget->type = EXTENSION_WIDGET_SPIN_ICON;

return vlclua_create_widget_inner( L, 0, p_widget );
}







static int vlclua_create_widget_inner( lua_State *L, int i_args,
extension_widget_t *p_widget )
{
int arg = i_args + 2;


extension_dialog_t **pp_dlg =
(extension_dialog_t**) luaL_checkudata( L, 1, "dialog" );
if( !pp_dlg || !*pp_dlg )
return luaL_error( L, "Can't get pointer to dialog" );
extension_dialog_t *p_dlg = *pp_dlg;


p_widget->p_dialog = p_dlg;


if( lua_isnumber( L, arg ) )
p_widget->i_column = luaL_checkinteger( L, arg );
else goto end_of_args;
if( lua_isnumber( L, ++arg ) )
p_widget->i_row = luaL_checkinteger( L, arg );
else goto end_of_args;
if( lua_isnumber( L, ++arg ) )
p_widget->i_horiz_span = luaL_checkinteger( L, arg );
else goto end_of_args;
if( lua_isnumber( L, ++arg ) )
p_widget->i_vert_span = luaL_checkinteger( L, arg );
else goto end_of_args;
if( lua_isnumber( L, ++arg ) )
p_widget->i_width = luaL_checkinteger( L, arg );
else goto end_of_args;
if( lua_isnumber( L, ++arg ) )
p_widget->i_height = luaL_checkinteger( L, arg );
else goto end_of_args;

end_of_args:
vlc_mutex_lock( &p_dlg->lock );


AddWidget( p_dlg, p_widget );

vlc_mutex_unlock( &p_dlg->lock );


extension_widget_t **pp_widget = lua_newuserdata( L, sizeof( extension_widget_t* ) );
*pp_widget = p_widget;
if( luaL_newmetatable( L, "widget" ) )
{
lua_newtable( L );
luaL_register( L, NULL, vlclua_widget_reg );
lua_setfield( L, -2, "__index" );
}
lua_setmetatable( L, -2 );

lua_SetDialogUpdate( L, 1 );

return 1;
}

static int vlclua_widget_set_text( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;


if( !lua_isstring( L, 2 ) )
return luaL_error( L, "widget:set_text usage: (text)" );


switch( p_widget->type )
{
case EXTENSION_WIDGET_LABEL:
case EXTENSION_WIDGET_BUTTON:
case EXTENSION_WIDGET_HTML:
case EXTENSION_WIDGET_TEXT_FIELD:
case EXTENSION_WIDGET_PASSWORD:
case EXTENSION_WIDGET_DROPDOWN:
case EXTENSION_WIDGET_CHECK_BOX:
break;
case EXTENSION_WIDGET_LIST:
case EXTENSION_WIDGET_IMAGE:
default:
return luaL_error( L, "method set_text not valid for this widget" );
}

vlc_mutex_lock( &p_widget->p_dialog->lock );


p_widget->b_update = true;
free( p_widget->psz_text );
p_widget->psz_text = strdup( luaL_checkstring( L, 2 ) );

vlc_mutex_unlock( &p_widget->p_dialog->lock );

lua_SetDialogUpdate( L, 1 );

return 1;
}

static int vlclua_widget_get_text( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;


switch( p_widget->type )
{
case EXTENSION_WIDGET_LABEL:
case EXTENSION_WIDGET_BUTTON:
case EXTENSION_WIDGET_HTML:
case EXTENSION_WIDGET_TEXT_FIELD:
case EXTENSION_WIDGET_PASSWORD:
case EXTENSION_WIDGET_DROPDOWN:
case EXTENSION_WIDGET_CHECK_BOX:
break;
case EXTENSION_WIDGET_LIST:
case EXTENSION_WIDGET_IMAGE:
default:
return luaL_error( L, "method get_text not valid for this widget" );
}

extension_dialog_t *p_dlg = p_widget->p_dialog;
vlc_mutex_lock( &p_dlg->lock );

char *psz_text = NULL;
if( p_widget->psz_text )
psz_text = strdup( p_widget->psz_text );
vlc_mutex_unlock( &p_dlg->lock );

lua_pushstring( L, psz_text );

free( psz_text );
return 1;
}

static int vlclua_widget_get_checked( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_CHECK_BOX )
return luaL_error( L, "method get_checked not valid for this widget" );

vlc_mutex_lock( &p_widget->p_dialog->lock );
lua_pushboolean( L, p_widget->b_checked );
vlc_mutex_unlock( &p_widget->p_dialog->lock );

return 1;
}

static int vlclua_widget_add_value( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_DROPDOWN
&& p_widget->type != EXTENSION_WIDGET_LIST )
return luaL_error( L, "method add_value not valid for this widget" );

if( !lua_isstring( L, 2 ) )
return luaL_error( L, "widget:add_value usage: (text, id = 0)" );

struct extension_widget_value_t *p_value,
*p_new_value = calloc( 1, sizeof( struct extension_widget_value_t ) );
p_new_value->psz_text = strdup( luaL_checkstring( L, 2 ) );
p_new_value->i_id = lua_tointeger( L, 3 );

vlc_mutex_lock( &p_widget->p_dialog->lock );

if( !p_widget->p_values )
{
p_widget->p_values = p_new_value;
if( p_widget->type == EXTENSION_WIDGET_DROPDOWN )
p_new_value->b_selected = true;
}
else
{
for( p_value = p_widget->p_values;
p_value->p_next != NULL;
p_value = p_value->p_next )
{ }
p_value->p_next = p_new_value;
}

p_widget->b_update = true;
vlc_mutex_unlock( &p_widget->p_dialog->lock );

lua_SetDialogUpdate( L, 1 );

return 1;
}

static int vlclua_widget_get_value( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_DROPDOWN )
return luaL_error( L, "method get_value not valid for this widget" );

vlc_mutex_lock( &p_widget->p_dialog->lock );

struct extension_widget_value_t *p_value;
for( p_value = p_widget->p_values;
p_value != NULL;
p_value = p_value->p_next )
{
if( p_value->b_selected )
{
lua_pushinteger( L, p_value->i_id );
lua_pushstring( L, p_value->psz_text );
vlc_mutex_unlock( &p_widget->p_dialog->lock );
return 2;
}
}

vlc_mutex_unlock( &p_widget->p_dialog->lock );

lua_pushinteger( L, -1 );
lua_pushnil( L );
return 2;
}

static int vlclua_widget_clear( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_DROPDOWN
&& p_widget->type != EXTENSION_WIDGET_LIST )
return luaL_error( L, "method clear not valid for this widget" );

struct extension_widget_value_t *p_value, *p_next;

vlc_mutex_lock( &p_widget->p_dialog->lock );

for( p_value = p_widget->p_values;
p_value != NULL;
p_value = p_next )
{
p_next = p_value->p_next;
free( p_value->psz_text );
free( p_value );
}

p_widget->p_values = NULL;
p_widget->b_update = true;

vlc_mutex_unlock( &p_widget->p_dialog->lock );

lua_SetDialogUpdate( L, 1 );

return 1;
}

static int vlclua_widget_get_selection( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_LIST )
return luaL_error( L, "method get_selection not valid for this widget" );


lua_newtable( L );

vlc_mutex_lock( &p_widget->p_dialog->lock );

struct extension_widget_value_t *p_value;
for( p_value = p_widget->p_values;
p_value != NULL;
p_value = p_value->p_next )
{
if( p_value->b_selected )
{
lua_pushinteger( L, p_value->i_id );
lua_pushstring( L, p_value->psz_text );
lua_settable( L, -3 );
}
}

vlc_mutex_unlock( &p_widget->p_dialog->lock );

return 1;
}

static int vlclua_widget_set_checked( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_CHECK_BOX )
return luaL_error( L, "method set_checked not valid for this widget" );


if( !lua_isboolean( L, 2 ) )
return luaL_error( L, "widget:set_checked usage: (bool)" );

vlc_mutex_lock( &p_widget->p_dialog->lock );

bool b_old_check = p_widget->b_checked;
p_widget->b_checked = lua_toboolean( L, 2 );

vlc_mutex_unlock( &p_widget->p_dialog->lock );

if( b_old_check != p_widget->b_checked )
{

p_widget->b_update = true;
lua_SetDialogUpdate( L, 1 );
}

return 1;
}

static int vlclua_widget_animate( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_SPIN_ICON )
return luaL_error( L, "method animate not valid for this widget" );


vlc_mutex_lock( &p_widget->p_dialog->lock );
if( !lua_isnumber( L, 2 ) )
p_widget->i_spin_loops = -1;
else
p_widget->i_spin_loops = lua_tointeger( L, 2 );
vlc_mutex_unlock( &p_widget->p_dialog->lock );


p_widget->b_update = true;
lua_SetDialogUpdate( L, 1 );

return 1;
}

static int vlclua_widget_stop( lua_State *L )
{

extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 1, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;

if( p_widget->type != EXTENSION_WIDGET_SPIN_ICON )
return luaL_error( L, "method stop not valid for this widget" );

vlc_mutex_lock( &p_widget->p_dialog->lock );

bool b_needs_update = p_widget->i_spin_loops != 0;
p_widget->i_spin_loops = 0;

vlc_mutex_unlock( &p_widget->p_dialog->lock );

if( b_needs_update )
{

p_widget->b_update = true;
lua_SetDialogUpdate( L, 1 );
}

return 1;
}






static int vlclua_dialog_delete_widget( lua_State *L )
{

extension_dialog_t **pp_dlg =
(extension_dialog_t**) luaL_checkudata( L, 1, "dialog" );
if( !pp_dlg || !*pp_dlg )
return luaL_error( L, "Can't get pointer to dialog" );
extension_dialog_t *p_dlg = *pp_dlg;


if( !lua_isuserdata( L, 2 ) )
return luaL_error( L, "Argument to del_widget is not a widget" );


extension_widget_t **pp_widget =
(extension_widget_t **) luaL_checkudata( L, 2, "widget" );
if( !pp_widget || !*pp_widget )
return luaL_error( L, "Can't get pointer to widget" );
extension_widget_t *p_widget = *pp_widget;


*pp_widget = NULL;
if( p_widget->type == EXTENSION_WIDGET_BUTTON )
{

lua_pushlightuserdata( L, p_widget );
lua_pushnil( L );
lua_settable( L, LUA_REGISTRYINDEX );
}

vlc_object_t *p_mgr = vlclua_get_this( L );

p_widget->b_kill = true;

lua_SetDialogUpdate( L, 0 ); 
int i_ret = vlc_ext_dialog_update( p_mgr, p_dlg );

if( i_ret != VLC_SUCCESS )
{
return luaL_error( L, "Could not delete widget" );
}

vlc_mutex_lock( &p_dlg->lock );





while( p_widget->p_sys_intf != NULL && !p_dlg->b_kill
&& p_dlg->p_sys_intf != NULL )
{
vlc_cond_wait( &p_dlg->cond, &p_dlg->lock );
}

i_ret = DeleteWidget( p_dlg, p_widget );

vlc_mutex_unlock( &p_dlg->lock );

if( i_ret != VLC_SUCCESS )
{
return luaL_error( L, "Could not remove widget from list" );
}

return 1;
}












static void AddWidget( extension_dialog_t *p_dialog,
extension_widget_t *p_widget )
{
ARRAY_APPEND( p_dialog->widgets, p_widget );
}






static int DeleteWidget( extension_dialog_t *p_dialog,
extension_widget_t *p_widget )
{
int pos = -1;
bool found = false;
extension_widget_t *p_iter;
ARRAY_FOREACH( p_iter, p_dialog->widgets )
{
pos++;
if( p_iter == p_widget )
{
found = true;
break;
}
}

if( !found )
return VLC_EGENERIC;

ARRAY_REMOVE( p_dialog->widgets, pos );


free( p_widget->p_sys );
struct extension_widget_value_t *p_value = p_widget->p_values;
while( p_value )
{
free( p_value->psz_text );
struct extension_widget_value_t *old = p_value;
p_value = p_value->p_next;
free( old );
}
free( p_widget->psz_text );
free( p_widget );

return VLC_SUCCESS;
}
