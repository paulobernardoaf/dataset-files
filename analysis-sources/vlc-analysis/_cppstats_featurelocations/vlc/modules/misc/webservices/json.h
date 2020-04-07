





























#if !defined(VLC_WEBSERVICES_JSON_H_)
#define VLC_WEBSERVICES_JSON_H_

#if !defined(json_char)
#define json_char char
#endif

#if !defined(json_int_t)
#include <inttypes.h>
#define json_int_t int64_t
#endif

#if defined(__cplusplus)

#include <string.h>

extern "C"
{

#endif

typedef struct
{
unsigned long max_memory;
int settings;

} json_settings;

#define json_relaxed_commas 1

typedef enum
{
json_none,
json_object,
json_array,
json_integer,
json_double,
json_string,
json_boolean,
json_null

} json_type;

extern const struct _json_value json_value_none;

typedef struct _json_value
{
struct _json_value * parent;

json_type type;

union
{
int boolean;
json_int_t integer;
double dbl;

struct
{
unsigned int length;
json_char * ptr; 

} string;

struct
{
unsigned int length;

struct
{
json_char * name;
struct _json_value * value;

} * values;

} object;

struct
{
unsigned int length;
struct _json_value ** values;

} array;

} u;

union
{
struct _json_value * next_alloc;
void * object_mem;

} _reserved;




#if defined(__cplusplus)

public:

inline _json_value ()
{ memset (this, 0, sizeof (_json_value));
}

inline const struct _json_value &operator [] (int index) const
{
if (type != json_array || index < 0
|| ((unsigned int) index) >= u.array.length)
{
return json_value_none;
}

return *u.array.values [index];
}

inline const struct _json_value &operator [] (const char * index) const
{
if (type != json_object)
return json_value_none;

for (unsigned int i = 0; i < u.object.length; ++ i)
if (!strcmp (u.object.values [i].name, index))
return *u.object.values [i].value;

return json_value_none;
}

inline operator const char * () const
{
switch (type)
{
case json_string:
return u.string.ptr;

default:
return "";
};
}

inline operator json_int_t () const
{
switch (type)
{
case json_integer:
return u.integer;

case json_double:
return (json_int_t) u.dbl;

default:
return 0;
};
}

inline operator bool () const
{
if (type != json_boolean)
return false;

return u.boolean != 0;
}

inline operator double () const
{
switch (type)
{
case json_integer:
return (double) u.integer;

case json_double:
return u.dbl;

default:
return 0;
};
}

#endif

} json_value;

json_value * json_parse
(const json_char * json);

json_value * json_parse_ex
(json_settings * settings, const json_char * json, char * error);

void json_value_free (json_value *);


#if defined(__cplusplus)
} 
#endif

#endif
