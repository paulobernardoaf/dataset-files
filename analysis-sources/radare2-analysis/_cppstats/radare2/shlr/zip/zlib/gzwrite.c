#include "gzguts.h"
local int gz_init OF((gz_statep));
local int gz_comp OF((gz_statep, int));
local int gz_zero OF((gz_statep, z_off64_t));
local z_size_t gz_write OF((gz_statep, voidpc, z_size_t));
local int gz_init(state)
gz_statep state;
{
int ret;
z_streamp strm = &(state->strm);
state->in = (unsigned char *)malloc(state->want << 1);
if (state->in == NULL) {
gz_error(state, Z_MEM_ERROR, "out of memory");
return -1;
}
if (!state->direct) {
state->out = (unsigned char *)malloc(state->want);
if (state->out == NULL) {
free(state->in);
gz_error(state, Z_MEM_ERROR, "out of memory");
return -1;
}
strm->zalloc = Z_NULL;
strm->zfree = Z_NULL;
strm->opaque = Z_NULL;
ret = deflateInit2(strm, state->level, Z_DEFLATED,
MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
if (ret != Z_OK) {
free(state->out);
free(state->in);
gz_error(state, Z_MEM_ERROR, "out of memory");
return -1;
}
strm->next_in = NULL;
}
state->size = state->want;
if (!state->direct) {
strm->avail_out = state->size;
strm->next_out = state->out;
state->x.next = strm->next_out;
}
return 0;
}
local int gz_comp(state, flush)
gz_statep state;
int flush;
{
int ret, writ;
unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
z_streamp strm = &(state->strm);
if (state->size == 0 && gz_init(state) == -1)
return -1;
if (state->direct) {
while (strm->avail_in) {
put = strm->avail_in > max ? max : strm->avail_in;
writ = write(state->fd, strm->next_in, put);
if (writ < 0) {
gz_error(state, Z_ERRNO, zstrerror());
return -1;
}
strm->avail_in -= (unsigned)writ;
strm->next_in += writ;
}
return 0;
}
ret = Z_OK;
do {
if (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
(flush != Z_FINISH || ret == Z_STREAM_END))) {
while (strm->next_out > state->x.next) {
put = strm->next_out - state->x.next > (int)max ? max :
(unsigned)(strm->next_out - state->x.next);
writ = write(state->fd, state->x.next, put);
if (writ < 0) {
gz_error(state, Z_ERRNO, zstrerror());
return -1;
}
state->x.next += writ;
}
if (strm->avail_out == 0) {
strm->avail_out = state->size;
strm->next_out = state->out;
state->x.next = state->out;
}
}
have = strm->avail_out;
ret = deflate(strm, flush);
if (ret == Z_STREAM_ERROR) {
gz_error(state, Z_STREAM_ERROR,
"internal error: deflate stream corrupt");
return -1;
}
have -= strm->avail_out;
} while (have);
if (flush == Z_FINISH)
deflateReset(strm);
return 0;
}
local int gz_zero(state, len)
gz_statep state;
z_off64_t len;
{
int first;
unsigned n;
z_streamp strm = &(state->strm);
if (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
return -1;
first = 1;
while (len) {
n = GT_OFF(state->size) || (z_off64_t)state->size > len ?
(unsigned)len : state->size;
if (first) {
memset(state->in, 0, n);
first = 0;
}
strm->avail_in = n;
strm->next_in = state->in;
state->x.pos += n;
if (gz_comp(state, Z_NO_FLUSH) == -1)
return -1;
len -= n;
}
return 0;
}
local z_size_t gz_write(state, buf, len)
gz_statep state;
voidpc buf;
z_size_t len;
{
z_size_t put = len;
if (len == 0)
return 0;
if (state->size == 0 && gz_init(state) == -1)
return 0;
if (state->seek) {
state->seek = 0;
if (gz_zero(state, state->skip) == -1)
return 0;
}
if (len < state->size) {
do {
unsigned have, copy;
if (state->strm.avail_in == 0)
state->strm.next_in = state->in;
have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
state->in);
copy = state->size - have;
if (copy > len)
copy = len;
memcpy(state->in + have, buf, copy);
state->strm.avail_in += copy;
state->x.pos += copy;
buf = (const char *)buf + copy;
len -= copy;
if (len && gz_comp(state, Z_NO_FLUSH) == -1)
return 0;
} while (len);
}
else {
if (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
return 0;
state->strm.next_in = (z_const Bytef *)buf;
do {
unsigned n = (unsigned)-1;
if (n > len)
n = len;
state->strm.avail_in = n;
state->x.pos += n;
if (gz_comp(state, Z_NO_FLUSH) == -1)
return 0;
len -= n;
} while (len);
}
return put;
}
int ZEXPORT gzwrite(file, buf, len)
gzFile file;
voidpc buf;
unsigned len;
{
gz_statep state;
if (file == NULL)
return 0;
state = (gz_statep)file;
if (state->mode != GZ_WRITE || state->err != Z_OK)
return 0;
if ((int)len < 0) {
gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
return 0;
}
return (int)gz_write(state, buf, len);
}
z_size_t ZEXPORT gzfwrite(buf, size, nitems, file)
voidpc buf;
z_size_t size;
z_size_t nitems;
gzFile file;
{
z_size_t len;
gz_statep state;
if (file == NULL || size < 1) {
return 0;
}
state = (gz_statep)file;
if (state->mode != GZ_WRITE || state->err != Z_OK)
return 0;
len = nitems * size;
if (size && len / size != nitems) {
gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
return 0;
}
return len ? gz_write(state, buf, len) / size : 0;
}
int ZEXPORT gzputc(file, c)
gzFile file;
int c;
{
unsigned have;
unsigned char buf[1];
gz_statep state;
z_streamp strm;
if (file == NULL)
return -1;
state = (gz_statep)file;
strm = &(state->strm);
if (state->mode != GZ_WRITE || state->err != Z_OK)
return -1;
if (state->seek) {
state->seek = 0;
if (gz_zero(state, state->skip) == -1)
return -1;
}
if (state->size) {
if (strm->avail_in == 0)
strm->next_in = state->in;
have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
if (have < state->size) {
state->in[have] = (unsigned char)c;
strm->avail_in++;
state->x.pos++;
return c & 0xff;
}
}
buf[0] = (unsigned char)c;
if (gz_write(state, buf, 1) != 1)
return -1;
return c & 0xff;
}
int ZEXPORT gzputs(file, str)
gzFile file;
const char *str;
{
int ret;
z_size_t len;
gz_statep state;
if (file == NULL)
return -1;
state = (gz_statep)file;
if (state->mode != GZ_WRITE || state->err != Z_OK)
return -1;
len = strlen(str);
ret = gz_write(state, str, len);
return ret == 0 && len != 0 ? -1 : ret;
}
#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>
int ZEXPORTVA gzvprintf(gzFile file, const char *format, va_list va)
{
int len;
unsigned left;
char *next;
gz_statep state;
z_streamp strm;
if (file == NULL)
return Z_STREAM_ERROR;
state = (gz_statep)file;
strm = &(state->strm);
if (state->mode != GZ_WRITE || state->err != Z_OK)
return Z_STREAM_ERROR;
if (state->size == 0 && gz_init(state) == -1)
return state->err;
if (state->seek) {
state->seek = 0;
if (gz_zero(state, state->skip) == -1)
return state->err;
}
if (strm->avail_in == 0)
strm->next_in = state->in;
next = (char *)(state->in + (strm->next_in - state->in) + strm->avail_in);
next[state->size - 1] = 0;
#if defined(NO_vsnprintf)
#if defined(HAS_vsprintf_void)
(void)vsprintf(next, format, va);
for (len = 0; len < state->size; len++)
if (next[len] == 0) break;
#else
len = vsprintf(next, format, va);
#endif
#else
#if defined(HAS_vsnprintf_void)
(void)vsnprintf(next, state->size, format, va);
len = strlen(next);
#else
len = vsnprintf(next, state->size, format, va);
#endif
#endif
if (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
return 0;
strm->avail_in += (unsigned)len;
state->x.pos += len;
if (strm->avail_in >= state->size) {
left = strm->avail_in - state->size;
strm->avail_in = state->size;
if (gz_comp(state, Z_NO_FLUSH) == -1)
return state->err;
memcpy(state->in, state->in + state->size, left);
strm->next_in = state->in;
strm->avail_in = left;
}
return len;
}
int ZEXPORTVA gzprintf(gzFile file, const char *format, ...)
{
va_list va;
int ret;
va_start(va, format);
ret = gzvprintf(file, format, va);
va_end(va);
return ret;
}
#else 
int ZEXPORTVA gzprintf (file, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
a11, a12, a13, a14, a15, a16, a17, a18, a19, a20)
gzFile file;
const char *format;
int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
{
unsigned len, left;
char *next;
gz_statep state;
z_streamp strm;
if (file == NULL)
return Z_STREAM_ERROR;
state = (gz_statep)file;
strm = &(state->strm);
if (sizeof(int) != sizeof(void *))
return Z_STREAM_ERROR;
if (state->mode != GZ_WRITE || state->err != Z_OK)
return Z_STREAM_ERROR;
if (state->size == 0 && gz_init(state) == -1)
return state->error;
if (state->seek) {
state->seek = 0;
if (gz_zero(state, state->skip) == -1)
return state->error;
}
if (strm->avail_in == 0)
strm->next_in = state->in;
next = (char *)(strm->next_in + strm->avail_in);
next[state->size - 1] = 0;
#if defined(NO_snprintf)
#if defined(HAS_sprintf_void)
sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
a13, a14, a15, a16, a17, a18, a19, a20);
for (len = 0; len < size; len++)
if (next[len] == 0)
break;
#else
len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
a12, a13, a14, a15, a16, a17, a18, a19, a20);
#endif
#else
#if defined(HAS_snprintf_void)
snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
len = strlen(next);
#else
len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#endif
#endif
if (len == 0 || len >= state->size || next[state->size - 1] != 0)
return 0;
strm->avail_in += len;
state->x.pos += len;
if (strm->avail_in >= state->size) {
left = strm->avail_in - state->size;
strm->avail_in = state->size;
if (gz_comp(state, Z_NO_FLUSH) == -1)
return state->err;
memcpy(state->in, state->in + state->size, left);
strm->next_in = state->in;
strm->avail_in = left;
}
return (int)len;
}
#endif
int ZEXPORT gzflush(file, flush)
gzFile file;
int flush;
{
gz_statep state;
if (file == NULL)
return Z_STREAM_ERROR;
state = (gz_statep)file;
if (state->mode != GZ_WRITE || state->err != Z_OK)
return Z_STREAM_ERROR;
if (flush < 0 || flush > Z_FINISH)
return Z_STREAM_ERROR;
if (state->seek) {
state->seek = 0;
if (gz_zero(state, state->skip) == -1)
return state->err;
}
(void)gz_comp(state, flush);
return state->err;
}
int ZEXPORT gzsetparams(file, level, strategy)
gzFile file;
int level;
int strategy;
{
gz_statep state;
z_streamp strm;
if (file == NULL)
return Z_STREAM_ERROR;
state = (gz_statep)file;
strm = &(state->strm);
if (state->mode != GZ_WRITE || state->err != Z_OK)
return Z_STREAM_ERROR;
if (level == state->level && strategy == state->strategy)
return Z_OK;
if (state->seek) {
state->seek = 0;
if (gz_zero(state, state->skip) == -1)
return state->err;
}
if (state->size) {
if (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
return state->err;
deflateParams(strm, level, strategy);
}
state->level = level;
state->strategy = strategy;
return Z_OK;
}
int ZEXPORT gzclose_w(file)
gzFile file;
{
int ret = Z_OK;
gz_statep state;
if (file == NULL)
return Z_STREAM_ERROR;
state = (gz_statep)file;
if (state->mode != GZ_WRITE)
return Z_STREAM_ERROR;
if (state->seek) {
state->seek = 0;
if (gz_zero(state, state->skip) == -1)
ret = state->err;
}
if (gz_comp(state, Z_FINISH) == -1)
ret = state->err;
if (state->size) {
if (!state->direct) {
(void)deflateEnd(&(state->strm));
free(state->out);
}
free(state->in);
}
gz_error(state, Z_OK, NULL);
free(state->path);
if (close(state->fd) == -1)
ret = Z_ERRNO;
free(state);
return ret;
}
