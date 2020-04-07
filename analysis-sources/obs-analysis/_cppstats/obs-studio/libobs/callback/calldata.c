#include <string.h>
#include "../util/bmem.h"
#include "../util/base.h"
#include "calldata.h"
static inline void cd_serialize(uint8_t **pos, void *ptr, size_t size)
{
memcpy(ptr, *pos, size);
*pos += size;
}
static inline size_t cd_serialize_size(uint8_t **pos)
{
size_t size = 0;
memcpy(&size, *pos, sizeof(size_t));
*pos += sizeof(size_t);
return size;
}
static inline const char *cd_serialize_string(uint8_t **pos)
{
size_t size = cd_serialize_size(pos);
const char *str = (const char *)*pos;
*pos += size;
return (size != 0) ? str : NULL;
}
static bool cd_getparam(const calldata_t *data, const char *name, uint8_t **pos)
{
size_t name_size;
if (!data->size)
return false;
*pos = data->stack;
name_size = cd_serialize_size(pos);
while (name_size != 0) {
const char *param_name = (const char *)*pos;
size_t param_size;
*pos += name_size;
if (strcmp(param_name, name) == 0)
return true;
param_size = cd_serialize_size(pos);
*pos += param_size;
name_size = cd_serialize_size(pos);
}
*pos -= sizeof(size_t);
return false;
}
static inline void cd_copy_string(uint8_t **pos, const char *str, size_t len)
{
if (!len)
len = strlen(str) + 1;
memcpy(*pos, &len, sizeof(size_t));
*pos += sizeof(size_t);
memcpy(*pos, str, len);
*pos += len;
}
static inline void cd_copy_data(uint8_t **pos, const void *in, size_t size)
{
memcpy(*pos, &size, sizeof(size_t));
*pos += sizeof(size_t);
if (size) {
memcpy(*pos, in, size);
*pos += size;
}
}
static inline void cd_set_first_param(calldata_t *data, const char *name,
const void *in, size_t size)
{
uint8_t *pos;
size_t capacity;
size_t name_len = strlen(name) + 1;
capacity = sizeof(size_t) * 3 + name_len + size;
data->size = capacity;
if (capacity < 128)
capacity = 128;
data->capacity = capacity;
data->stack = bmalloc(capacity);
pos = data->stack;
cd_copy_string(&pos, name, name_len);
cd_copy_data(&pos, in, size);
memset(pos, 0, sizeof(size_t));
}
static inline bool cd_ensure_capacity(calldata_t *data, uint8_t **pos,
size_t new_size)
{
size_t offset;
size_t new_capacity;
if (new_size < data->capacity)
return true;
if (data->fixed) {
blog(LOG_ERROR, "Tried to go above fixed calldata stack size!");
return false;
}
offset = *pos - data->stack;
new_capacity = data->capacity * 2;
if (new_capacity < new_size)
new_capacity = new_size;
data->stack = brealloc(data->stack, new_capacity);
data->capacity = new_capacity;
*pos = data->stack + offset;
return true;
}
bool calldata_get_data(const calldata_t *data, const char *name, void *out,
size_t size)
{
uint8_t *pos;
size_t data_size;
if (!data || !name || !*name)
return false;
if (!cd_getparam(data, name, &pos))
return false;
data_size = cd_serialize_size(&pos);
if (data_size != size)
return false;
memcpy(out, pos, size);
return true;
}
void calldata_set_data(calldata_t *data, const char *name, const void *in,
size_t size)
{
uint8_t *pos = NULL;
if (!data || !name || !*name)
return;
if (!data->fixed && !data->stack) {
cd_set_first_param(data, name, in, size);
return;
}
if (cd_getparam(data, name, &pos)) {
size_t cur_size;
memcpy(&cur_size, pos, sizeof(size_t));
if (cur_size < size) {
size_t offset = size - cur_size;
size_t bytes = data->size;
if (!cd_ensure_capacity(data, &pos, bytes + offset))
return;
memmove(pos + offset, pos, bytes - (pos - data->stack));
data->size += offset;
} else if (cur_size > size) {
size_t offset = cur_size - size;
size_t bytes = data->size - offset;
memmove(pos, pos + offset, bytes - (pos - data->stack));
data->size -= offset;
}
cd_copy_data(&pos, in, size);
} else {
size_t name_len = strlen(name) + 1;
size_t offset = name_len + size + sizeof(size_t) * 2;
if (!cd_ensure_capacity(data, &pos, data->size + offset))
return;
data->size += offset;
cd_copy_string(&pos, name, 0);
cd_copy_data(&pos, in, size);
memset(pos, 0, sizeof(size_t));
}
}
bool calldata_get_string(const calldata_t *data, const char *name,
const char **str)
{
uint8_t *pos;
if (!data || !name || !*name)
return false;
if (!cd_getparam(data, name, &pos))
return false;
*str = cd_serialize_string(&pos);
return true;
}
