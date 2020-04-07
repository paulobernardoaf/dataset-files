






















#if !defined(VLC_VARIABLES_H)
#define VLC_VARIABLES_H 1












#define VLC_VAR_TYPE 0x00ff
#define VLC_VAR_CLASS 0x00f0
#define VLC_VAR_FLAGS 0xff00






#define VLC_VAR_VOID 0x0010
#define VLC_VAR_BOOL 0x0020
#define VLC_VAR_INTEGER 0x0030
#define VLC_VAR_STRING 0x0040
#define VLC_VAR_FLOAT 0x0050
#define VLC_VAR_ADDRESS 0x0070
#define VLC_VAR_COORDS 0x00A0







#define VLC_VAR_HASCHOICE 0x0100

#define VLC_VAR_ISCOMMAND 0x2000




#define VLC_VAR_DOINHERIT 0x8000










#define VLC_VAR_SETSTEP 0x0012






#define VLC_VAR_SETVALUE 0x0013

#define VLC_VAR_SETTEXT 0x0014
#define VLC_VAR_GETTEXT 0x0015

#define VLC_VAR_GETMIN 0x0016
#define VLC_VAR_GETMAX 0x0017
#define VLC_VAR_GETSTEP 0x0018

#define VLC_VAR_ADDCHOICE 0x0020
#define VLC_VAR_DELCHOICE 0x0021
#define VLC_VAR_CLEARCHOICES 0x0022
#define VLC_VAR_GETCHOICES 0x0024

#define VLC_VAR_CHOICESCOUNT 0x0026
#define VLC_VAR_SETMINMAX 0x0027








enum vlc_var_atomic_op {
VLC_VAR_BOOL_TOGGLE, 
VLC_VAR_INTEGER_ADD, 
VLC_VAR_INTEGER_OR, 
VLC_VAR_INTEGER_NAND,
};













VLC_API int var_Create(vlc_object_t *obj, const char *name, int type);










VLC_API void var_Destroy(vlc_object_t *obj, const char *name);








VLC_API int var_Change(vlc_object_t *obj, const char *name, int action, ...);









VLC_API int var_Type(vlc_object_t *obj, const char *name) VLC_USED;








VLC_API int var_Set(vlc_object_t *obj, const char *name, vlc_value_t val);








VLC_API int var_Get(vlc_object_t *obj, const char *name, vlc_value_t *valp);

VLC_API int var_SetChecked( vlc_object_t *, const char *, int, vlc_value_t );
VLC_API int var_GetChecked( vlc_object_t *, const char *, int, vlc_value_t * );
















VLC_API int var_GetAndSet(vlc_object_t *obj, const char *name, int op,
vlc_value_t *value);








VLC_API int var_Inherit( vlc_object_t *, const char *, int, vlc_value_t * );
































VLC_API void var_AddCallback(vlc_object_t *obj, const char *name,
vlc_callback_t callback, void *opaque);







VLC_API void var_DelCallback(vlc_object_t *obj, const char *name,
vlc_callback_t callback, void *opaque);











VLC_API void var_TriggerCallback(vlc_object_t *obj, const char *name);









VLC_API void var_AddListCallback( vlc_object_t *, const char *, vlc_list_callback_t, void * );






VLC_API void var_DelListCallback( vlc_object_t *, const char *, vlc_list_callback_t, void * );












static inline int var_SetInteger( vlc_object_t *p_obj, const char *psz_name,
int64_t i )
{
vlc_value_t val;
val.i_int = i;
return var_SetChecked( p_obj, psz_name, VLC_VAR_INTEGER, val );
}








static inline int var_SetBool( vlc_object_t *p_obj, const char *psz_name, bool b )
{
vlc_value_t val;
val.b_bool = b;
return var_SetChecked( p_obj, psz_name, VLC_VAR_BOOL, val );
}

static inline int var_SetCoords( vlc_object_t *obj, const char *name,
int32_t x, int32_t y )
{
vlc_value_t val;
val.coords.x = x;
val.coords.y = y;
return var_SetChecked (obj, name, VLC_VAR_COORDS, val);
}








static inline int var_SetFloat( vlc_object_t *p_obj, const char *psz_name, float f )
{
vlc_value_t val;
val.f_float = f;
return var_SetChecked( p_obj, psz_name, VLC_VAR_FLOAT, val );
}








static inline int var_SetString( vlc_object_t *p_obj, const char *psz_name, const char *psz_string )
{
vlc_value_t val;
val.psz_string = (char *)psz_string;
return var_SetChecked( p_obj, psz_name, VLC_VAR_STRING, val );
}








static inline
int var_SetAddress( vlc_object_t *p_obj, const char *psz_name, void *ptr )
{
vlc_value_t val;
val.p_address = ptr;
return var_SetChecked( p_obj, psz_name, VLC_VAR_ADDRESS, val );
}







VLC_USED
static inline int64_t var_GetInteger( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val;
if( !var_GetChecked( p_obj, psz_name, VLC_VAR_INTEGER, &val ) )
return val.i_int;
else
return 0;
}







VLC_USED
static inline bool var_GetBool( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val; val.b_bool = false;

if( !var_GetChecked( p_obj, psz_name, VLC_VAR_BOOL, &val ) )
return val.b_bool;
else
return false;
}

static inline void var_GetCoords( vlc_object_t *obj, const char *name,
int32_t *px, int32_t *py )
{
vlc_value_t val;

if (likely(!var_GetChecked (obj, name, VLC_VAR_COORDS, &val)))
{
*px = val.coords.x;
*py = val.coords.y;
}
else
*px = *py = 0;
}







VLC_USED
static inline float var_GetFloat( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val; val.f_float = 0.0;
if( !var_GetChecked( p_obj, psz_name, VLC_VAR_FLOAT, &val ) )
return val.f_float;
else
return 0.0;
}







VLC_USED VLC_MALLOC
static inline char *var_GetString( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val; val.psz_string = NULL;
if( var_GetChecked( p_obj, psz_name, VLC_VAR_STRING, &val ) )
return NULL;
else
return val.psz_string;
}

VLC_USED VLC_MALLOC
static inline char *var_GetNonEmptyString( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val;
if( var_GetChecked( p_obj, psz_name, VLC_VAR_STRING, &val ) )
return NULL;
if( val.psz_string && *val.psz_string )
return val.psz_string;
free( val.psz_string );
return NULL;
}

VLC_USED
static inline void *var_GetAddress( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val;
if( var_GetChecked( p_obj, psz_name, VLC_VAR_ADDRESS, &val ) )
return NULL;
else
return val.p_address;
}






static inline int64_t var_IncInteger( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val;
val.i_int = 1;
if( var_GetAndSet( p_obj, psz_name, VLC_VAR_INTEGER_ADD, &val ) )
return 0;
return val.i_int;
}






static inline int64_t var_DecInteger( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val;
val.i_int = -1;
if( var_GetAndSet( p_obj, psz_name, VLC_VAR_INTEGER_ADD, &val ) )
return 0;
return val.i_int;
}

static inline uint64_t var_OrInteger( vlc_object_t *obj, const char *name,
unsigned v )
{
vlc_value_t val;
val.i_int = v;
if( var_GetAndSet( obj, name, VLC_VAR_INTEGER_OR, &val ) )
return 0;
return val.i_int;
}

static inline uint64_t var_NAndInteger( vlc_object_t *obj, const char *name,
unsigned v )
{
vlc_value_t val;
val.i_int = v;
if( var_GetAndSet( obj, name, VLC_VAR_INTEGER_NAND, &val ) )
return 0;
return val.i_int;
}







VLC_USED
static inline int64_t var_CreateGetInteger( vlc_object_t *p_obj, const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_INTEGER | VLC_VAR_DOINHERIT );
return var_GetInteger( p_obj, psz_name );
}







VLC_USED
static inline bool var_CreateGetBool( vlc_object_t *p_obj, const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_BOOL | VLC_VAR_DOINHERIT );
return var_GetBool( p_obj, psz_name );
}







VLC_USED
static inline float var_CreateGetFloat( vlc_object_t *p_obj, const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_FLOAT | VLC_VAR_DOINHERIT );
return var_GetFloat( p_obj, psz_name );
}







VLC_USED VLC_MALLOC
static inline char *var_CreateGetString( vlc_object_t *p_obj,
const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_STRING | VLC_VAR_DOINHERIT );
return var_GetString( p_obj, psz_name );
}

VLC_USED VLC_MALLOC
static inline char *var_CreateGetNonEmptyString( vlc_object_t *p_obj,
const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_STRING | VLC_VAR_DOINHERIT );
return var_GetNonEmptyString( p_obj, psz_name );
}







VLC_USED
static inline void *var_CreateGetAddress( vlc_object_t *p_obj,
const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_ADDRESS | VLC_VAR_DOINHERIT );
return var_GetAddress( p_obj, psz_name );
}







VLC_USED
static inline int64_t var_CreateGetIntegerCommand( vlc_object_t *p_obj, const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_INTEGER | VLC_VAR_DOINHERIT
| VLC_VAR_ISCOMMAND );
return var_GetInteger( p_obj, psz_name );
}







VLC_USED
static inline bool var_CreateGetBoolCommand( vlc_object_t *p_obj, const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_BOOL | VLC_VAR_DOINHERIT
| VLC_VAR_ISCOMMAND );
return var_GetBool( p_obj, psz_name );
}







VLC_USED
static inline float var_CreateGetFloatCommand( vlc_object_t *p_obj, const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_FLOAT | VLC_VAR_DOINHERIT
| VLC_VAR_ISCOMMAND );
return var_GetFloat( p_obj, psz_name );
}







VLC_USED VLC_MALLOC
static inline char *var_CreateGetStringCommand( vlc_object_t *p_obj,
const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_STRING | VLC_VAR_DOINHERIT
| VLC_VAR_ISCOMMAND );
return var_GetString( p_obj, psz_name );
}

VLC_USED VLC_MALLOC
static inline char *var_CreateGetNonEmptyStringCommand( vlc_object_t *p_obj,
const char *psz_name )
{
var_Create( p_obj, psz_name, VLC_VAR_STRING | VLC_VAR_DOINHERIT
| VLC_VAR_ISCOMMAND );
return var_GetNonEmptyString( p_obj, psz_name );
}

VLC_USED
static inline int var_CountChoices( vlc_object_t *p_obj, const char *psz_name )
{
size_t count;
if( var_Change( p_obj, psz_name, VLC_VAR_CHOICESCOUNT, &count ) )
return 0;
return count;
}

static inline bool var_ToggleBool( vlc_object_t *p_obj, const char *psz_name )
{
vlc_value_t val;
if( var_GetAndSet( p_obj, psz_name, VLC_VAR_BOOL_TOGGLE, &val ) )
return false;
return val.b_bool;
}

VLC_USED
static inline bool var_InheritBool( vlc_object_t *obj, const char *name )
{
vlc_value_t val;

if( var_Inherit( obj, name, VLC_VAR_BOOL, &val ) )
val.b_bool = false;
return val.b_bool;
}

VLC_USED
static inline int64_t var_InheritInteger( vlc_object_t *obj, const char *name )
{
vlc_value_t val;

if( var_Inherit( obj, name, VLC_VAR_INTEGER, &val ) )
val.i_int = 0;
return val.i_int;
}

VLC_USED
static inline float var_InheritFloat( vlc_object_t *obj, const char *name )
{
vlc_value_t val;

if( var_Inherit( obj, name, VLC_VAR_FLOAT, &val ) )
val.f_float = 0.;
return val.f_float;
}

VLC_USED VLC_MALLOC
static inline char *var_InheritString( vlc_object_t *obj, const char *name )
{
vlc_value_t val;

if( var_Inherit( obj, name, VLC_VAR_STRING, &val ) )
val.psz_string = NULL;
else if( val.psz_string && !*val.psz_string )
{
free( val.psz_string );
val.psz_string = NULL;
}
return val.psz_string;
}

VLC_USED
static inline void *var_InheritAddress( vlc_object_t *obj, const char *name )
{
vlc_value_t val;

if( var_Inherit( obj, name, VLC_VAR_ADDRESS, &val ) )
val.p_address = NULL;
return val.p_address;
}



















VLC_API int var_InheritURational(vlc_object_t *obj, unsigned *num,
unsigned *den, const char *name);



















VLC_API int var_LocationParse(vlc_object_t *, const char *mrl, const char *prefix);

#if !defined(DOC)
#define var_Create(a,b,c) var_Create(VLC_OBJECT(a), b, c)
#define var_Destroy(a,b) var_Destroy(VLC_OBJECT(a), b)
#define var_Change(a,b,...) var_Change(VLC_OBJECT(a), b, __VA_ARGS__)
#define var_Type(a,b) var_Type(VLC_OBJECT(a), b)
#define var_Set(a,b,c) var_Set(VLC_OBJECT(a), b, c)
#define var_Get(a,b,c) var_Get(VLC_OBJECT(a), b, c)
#define var_SetChecked(o,n,t,v) var_SetChecked(VLC_OBJECT(o), n, t, v)
#define var_GetChecked(o,n,t,v) var_GetChecked(VLC_OBJECT(o), n, t, v)

#define var_AddCallback(a,b,c,d) var_AddCallback(VLC_OBJECT(a), b, c, d)
#define var_DelCallback(a,b,c,d) var_DelCallback(VLC_OBJECT(a), b, c, d)
#define var_TriggerCallback(a,b) var_TriggerCallback(VLC_OBJECT(a), b)
#define var_AddListCallback(a,b,c,d) var_AddListCallback(VLC_OBJECT(a), b, c, d)

#define var_DelListCallback(a,b,c,d) var_DelListCallback(VLC_OBJECT(a), b, c, d)


#define var_SetInteger(a,b,c) var_SetInteger(VLC_OBJECT(a), b, c)
#define var_SetBool(a,b,c) var_SetBool(VLC_OBJECT(a), b, c)
#define var_SetCoords(o,n,x,y) var_SetCoords(VLC_OBJECT(o), n, x, y)
#define var_SetFloat(a,b,c) var_SetFloat(VLC_OBJECT(a), b, c)
#define var_SetString(a,b,c) var_SetString(VLC_OBJECT(a), b, c)
#define var_SetAddress(o, n, p) var_SetAddress(VLC_OBJECT(o), n, p)

#define var_GetCoords(o,n,x,y) var_GetCoords(VLC_OBJECT(o), n, x, y)

#define var_IncInteger(a,b) var_IncInteger(VLC_OBJECT(a), b)
#define var_DecInteger(a,b) var_DecInteger(VLC_OBJECT(a), b)
#define var_OrInteger(a,b,c) var_OrInteger(VLC_OBJECT(a), b, c)
#define var_NAndInteger(a,b,c) var_NAndInteger(VLC_OBJECT(a), b, c)

#define var_CreateGetInteger(a,b) var_CreateGetInteger(VLC_OBJECT(a), b)
#define var_CreateGetBool(a,b) var_CreateGetBool(VLC_OBJECT(a), b)
#define var_CreateGetFloat(a,b) var_CreateGetFloat(VLC_OBJECT(a), b)
#define var_CreateGetString(a,b) var_CreateGetString(VLC_OBJECT(a), b)
#define var_CreateGetNonEmptyString(a,b) var_CreateGetNonEmptyString(VLC_OBJECT(a), b)

#define var_CreateGetAddress(a,b) var_CreateGetAddress( VLC_OBJECT(a), b)

#define var_CreateGetIntegerCommand(a,b) var_CreateGetIntegerCommand( VLC_OBJECT(a),b)
#define var_CreateGetBoolCommand(a,b) var_CreateGetBoolCommand( VLC_OBJECT(a),b)
#define var_CreateGetFloatCommand(a,b) var_CreateGetFloatCommand( VLC_OBJECT(a),b)
#define var_CreateGetStringCommand(a,b) var_CreateGetStringCommand( VLC_OBJECT(a),b)
#define var_CreateGetNonEmptyStringCommand(a,b) var_CreateGetNonEmptyStringCommand( VLC_OBJECT(a),b)

#define var_CountChoices(a,b) var_CountChoices(VLC_OBJECT(a),b)
#define var_ToggleBool(a,b) var_ToggleBool(VLC_OBJECT(a),b )

#define var_InheritBool(o, n) var_InheritBool(VLC_OBJECT(o), n)
#define var_InheritInteger(o, n) var_InheritInteger(VLC_OBJECT(o), n)
#define var_InheritFloat(o, n) var_InheritFloat(VLC_OBJECT(o), n)
#define var_InheritString(o, n) var_InheritString(VLC_OBJECT(o), n)
#define var_InheritAddress(o, n) var_InheritAddress(VLC_OBJECT(o), n)
#define var_InheritURational(a,b,c,d) var_InheritURational(VLC_OBJECT(a), b, c, d)

#define var_GetInteger(a,b) var_GetInteger(VLC_OBJECT(a),b)
#define var_GetBool(a,b) var_GetBool(VLC_OBJECT(a),b)
#define var_GetFloat(a,b) var_GetFloat(VLC_OBJECT(a),b)
#define var_GetString(a,b) var_GetString(VLC_OBJECT(a),b)
#define var_GetNonEmptyString(a,b) var_GetNonEmptyString( VLC_OBJECT(a),b)
#define var_GetAddress(a,b) var_GetAddress(VLC_OBJECT(a),b)

#define var_LocationParse(o, m, p) var_LocationParse(VLC_OBJECT(o), m, p)
#endif




#endif 
