#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nvim/func_attr.h"
#include "nvim/types.h"
#define ARRAY_DICT_INIT {.size = 0, .capacity = 0, .items = NULL}
#define STRING_INIT {.data = NULL, .size = 0}
#define OBJECT_INIT { .type = kObjectTypeNil }
#define ERROR_INIT { .type = kErrorTypeNone, .msg = NULL }
#define REMOTE_TYPE(type) typedef handle_T type
#define ERROR_SET(e) ((e)->type != kErrorTypeNone)
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#define ArrayOf(...) Array
#define DictionaryOf(...) Dictionary
#endif
typedef enum {
kErrorTypeNone = -1,
kErrorTypeException,
kErrorTypeValidation
} ErrorType;
typedef enum {
kMessageTypeUnknown = -1,
kMessageTypeRequest = 0,
kMessageTypeResponse = 1,
kMessageTypeNotification = 2,
} MessageType;
#define INTERNAL_CALL_MASK (((uint64_t)1) << (sizeof(uint64_t) * 8 - 1))
#define VIML_INTERNAL_CALL INTERNAL_CALL_MASK
#define LUA_INTERNAL_CALL (VIML_INTERNAL_CALL + 1)
static inline bool is_internal_call(const uint64_t channel_id)
REAL_FATTR_ALWAYS_INLINE REAL_FATTR_CONST;
static inline bool is_internal_call(const uint64_t channel_id)
{
return !!(channel_id & INTERNAL_CALL_MASK);
}
typedef struct {
ErrorType type;
char *msg;
} Error;
typedef bool Boolean;
typedef int64_t Integer;
typedef double Float;
#define API_INTEGER_MAX INT64_MAX
#define API_INTEGER_MIN INT64_MIN
typedef struct {
char *data;
size_t size;
} String;
REMOTE_TYPE(Buffer);
REMOTE_TYPE(Window);
REMOTE_TYPE(Tabpage);
typedef struct object Object;
typedef struct {
Object *items;
size_t size, capacity;
} Array;
typedef struct key_value_pair KeyValuePair;
typedef struct {
KeyValuePair *items;
size_t size, capacity;
} Dictionary;
typedef enum {
kObjectTypeNil = 0,
kObjectTypeBoolean,
kObjectTypeInteger,
kObjectTypeFloat,
kObjectTypeString,
kObjectTypeArray,
kObjectTypeDictionary,
kObjectTypeLuaRef,
kObjectTypeBuffer,
kObjectTypeWindow,
kObjectTypeTabpage,
} ObjectType;
struct object {
ObjectType type;
union {
Boolean boolean;
Integer integer;
Float floating;
String string;
Array array;
Dictionary dictionary;
LuaRef luaref;
} data;
};
struct key_value_pair {
String key;
Object value;
};
