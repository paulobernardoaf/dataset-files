




#include "gzguts.h"


local int gz_load OF((gz_statep, unsigned char *, unsigned, unsigned *));
local int gz_avail OF((gz_statep));
local int gz_look OF((gz_statep));
local int gz_decomp OF((gz_statep));
local int gz_fetch OF((gz_statep));
local int gz_skip OF((gz_statep, z_off64_t));
local z_size_t gz_read OF((gz_statep, voidp, z_size_t));





local int gz_load(state, buf, len, have)
gz_statep state;
unsigned char *buf;
unsigned len;
unsigned *have;
{
int ret;
unsigned get, max = ((unsigned)-1 >> 2) + 1;

*have = 0;
do {
get = len - *have;
if (get > max)
get = max;
ret = read(state->fd, buf + *have, get);
if (ret <= 0)
break;
*have += (unsigned)ret;
} while (*have < len);
if (ret < 0) {
gz_error(state, Z_ERRNO, zstrerror());
return -1;
}
if (ret == 0)
state->eof = 1;
return 0;
}








local int gz_avail(state)
gz_statep state;
{
unsigned got;
z_streamp strm = &(state->strm);

if (state->err != Z_OK && state->err != Z_BUF_ERROR)
return -1;
if (state->eof == 0) {
if (strm->avail_in) { 
unsigned char *p = state->in;
unsigned const char *q = strm->next_in;
unsigned n = strm->avail_in;
do {
*p++ = *q++;
} while (--n);
}
if (gz_load(state, state->in + strm->avail_in,
state->size - strm->avail_in, &got) == -1)
return -1;
strm->avail_in += got;
strm->next_in = state->in;
}
return 0;
}










local int gz_look(state)
gz_statep state;
{
z_streamp strm = &(state->strm);


if (state->size == 0) {

state->in = (unsigned char *)malloc(state->want);
state->out = (unsigned char *)malloc(state->want << 1);
if (state->in == NULL || state->out == NULL) {
free(state->out);
free(state->in);
gz_error(state, Z_MEM_ERROR, "out of memory");
return -1;
}
state->size = state->want;


state->strm.zalloc = Z_NULL;
state->strm.zfree = Z_NULL;
state->strm.opaque = Z_NULL;
state->strm.avail_in = 0;
state->strm.next_in = Z_NULL;
if (inflateInit2(&(state->strm), 15 + 16) != Z_OK) { 
free(state->out);
free(state->in);
state->size = 0;
gz_error(state, Z_MEM_ERROR, "out of memory");
return -1;
}
}


if (strm->avail_in < 2) {
if (gz_avail(state) == -1)
return -1;
if (strm->avail_in == 0)
return 0;
}








if (strm->avail_in > 1 &&
strm->next_in[0] == 31 && strm->next_in[1] == 139) {
inflateReset(strm);
state->how = GZIP;
state->direct = 0;
return 0;
}



if (state->direct == 0) {
strm->avail_in = 0;
state->eof = 1;
state->x.have = 0;
return 0;
}




state->x.next = state->out;
if (strm->avail_in) {
memcpy(state->x.next, strm->next_in, strm->avail_in);
state->x.have = strm->avail_in;
strm->avail_in = 0;
}
state->how = COPY;
state->direct = 1;
return 0;
}






local int gz_decomp(state)
gz_statep state;
{
int ret = Z_OK;
unsigned had;
z_streamp strm = &(state->strm);


had = strm->avail_out;
do {

if (strm->avail_in == 0 && gz_avail(state) == -1)
return -1;
if (strm->avail_in == 0) {
gz_error(state, Z_BUF_ERROR, "unexpected end of file");
break;
}


ret = inflate(strm, Z_NO_FLUSH);
if (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
gz_error(state, Z_STREAM_ERROR,
"internal error: inflate stream corrupt");
return -1;
}
if (ret == Z_MEM_ERROR) {
gz_error(state, Z_MEM_ERROR, "out of memory");
return -1;
}
if (ret == Z_DATA_ERROR) { 
gz_error(state, Z_DATA_ERROR,
strm->msg == NULL ? "compressed data error" : strm->msg);
return -1;
}
} while (strm->avail_out && ret != Z_STREAM_END);


state->x.have = had - strm->avail_out;
state->x.next = strm->next_out - state->x.have;


if (ret == Z_STREAM_END)
state->how = LOOK;


return 0;
}







local int gz_fetch(state)
gz_statep state;
{
z_streamp strm = &(state->strm);

do {
switch(state->how) {
case LOOK: 
if (gz_look(state) == -1)
return -1;
if (state->how == LOOK)
return 0;
break;
case COPY: 
if (gz_load(state, state->out, state->size << 1, &(state->x.have))
== -1)
return -1;
state->x.next = state->out;
return 0;
case GZIP: 
strm->avail_out = state->size << 1;
strm->next_out = state->out;
if (gz_decomp(state) == -1)
return -1;
}
} while (state->x.have == 0 && (!state->eof || strm->avail_in));
return 0;
}


local int gz_skip(state, len)
gz_statep state;
z_off64_t len;
{
unsigned n;


while (len)

if (state->x.have) {
n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > len ?
(unsigned)len : state->x.have;
state->x.have -= n;
state->x.next += n;
state->x.pos += n;
len -= n;
}


else if (state->eof && state->strm.avail_in == 0)
break;


else {

if (gz_fetch(state) == -1)
return -1;
}
return 0;
}





local z_size_t gz_read(state, buf, len)
gz_statep state;
voidp buf;
z_size_t len;
{
z_size_t got;
unsigned n;


if (len == 0)
return 0;


if (state->seek) {
state->seek = 0;
if (gz_skip(state, state->skip) == -1)
return 0;
}


got = 0;
do {

n = -1;
if (n > len)
n = len;


if (state->x.have) {
if (state->x.have < n)
n = state->x.have;
memcpy(buf, state->x.next, n);
state->x.next += n;
state->x.have -= n;
}


else if (state->eof && state->strm.avail_in == 0) {
state->past = 1; 
break;
}



else if (state->how == LOOK || n < (state->size << 1)) {

if (gz_fetch(state) == -1)
return 0;
continue; 


}


else if (state->how == COPY) { 
if (gz_load(state, (unsigned char *)buf, n, &n) == -1)
return 0;
}


else { 
state->strm.avail_out = n;
state->strm.next_out = (unsigned char *)buf;
if (gz_decomp(state) == -1)
return 0;
n = state->x.have;
state->x.have = 0;
}


len -= n;
buf = (char *)buf + n;
got += n;
state->x.pos += n;
} while (len);


return got;
}


int ZEXPORT gzread(file, buf, len)
gzFile file;
voidp buf;
unsigned len;
{
gz_statep state;


if (file == NULL)
return -1;
state = (gz_statep)file;


if (state->mode != GZ_READ ||
(state->err != Z_OK && state->err != Z_BUF_ERROR))
return -1;



if ((int)len < 0) {
gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
return -1;
}


len = gz_read(state, buf, len);


if (len == 0 && state->err != Z_OK && state->err != Z_BUF_ERROR)
return -1;


return (int)len;
}


z_size_t ZEXPORT gzfread(buf, size, nitems, file)
voidp buf;
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


if (state->mode != GZ_READ ||
(state->err != Z_OK && state->err != Z_BUF_ERROR))
return 0;


len = nitems * size;
if (size && len / size != nitems) {
gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
return 0;
}


return len ? gz_read(state, buf, len) / size : 0;
}


#if defined(Z_PREFIX_SET)
#undef z_gzgetc
#else
#undef gzgetc
#endif
int ZEXPORT gzgetc(file)
gzFile file;
{
int ret;
unsigned char buf[1];
gz_statep state;


if (file == NULL)
return -1;
state = (gz_statep)file;


if (state->mode != GZ_READ ||
(state->err != Z_OK && state->err != Z_BUF_ERROR))
return -1;


if (state->x.have) {
state->x.have--;
state->x.pos++;
return *(state->x.next)++;
}


ret = gz_read(state, buf, 1);
return ret < 1 ? -1 : buf[0];
}

int ZEXPORT gzgetc_(file)
gzFile file;
{
return gzgetc(file);
}


int ZEXPORT gzungetc(c, file)
int c;
gzFile file;
{
gz_statep state;


if (file == NULL)
return -1;
state = (gz_statep)file;


if (state->mode != GZ_READ ||
(state->err != Z_OK && state->err != Z_BUF_ERROR))
return -1;


if (state->seek) {
state->seek = 0;
if (gz_skip(state, state->skip) == -1)
return -1;
}


if (c < 0)
return -1;


if (state->x.have == 0) {
state->x.have = 1;
state->x.next = state->out + (state->size << 1) - 1;
state->x.next[0] = (unsigned char)c;
state->x.pos--;
state->past = 0;
return c;
}


if (state->x.have == (state->size << 1)) {
gz_error(state, Z_DATA_ERROR, "out of room to push characters");
return -1;
}


if (state->x.next == state->out) {
unsigned char *src = state->out + state->x.have;
unsigned char *dest = state->out + (state->size << 1);
while (src > state->out)
*--dest = *--src;
state->x.next = dest;
}
state->x.have++;
state->x.next--;
state->x.next[0] = (unsigned char)c;
state->x.pos--;
state->past = 0;
return c;
}


char * ZEXPORT gzgets(file, buf, len)
gzFile file;
char *buf;
int len;
{
unsigned left, n;
char *str;
unsigned char *eol;
gz_statep state;


if (file == NULL || buf == NULL || len < 1)
return NULL;
state = (gz_statep)file;


if (state->mode != GZ_READ ||
(state->err != Z_OK && state->err != Z_BUF_ERROR))
return NULL;


if (state->seek) {
state->seek = 0;
if (gz_skip(state, state->skip) == -1)
return NULL;
}




str = buf;
left = (unsigned)len - 1;
if (left) do {

if (state->x.have == 0 && gz_fetch(state) == -1)
return NULL; 
if (state->x.have == 0) { 
state->past = 1; 
break; 
}


n = state->x.have > left ? left : state->x.have;
eol = (unsigned char *)memchr(state->x.next, '\n', n);
if (eol != NULL)
n = (unsigned)(eol - state->x.next) + 1;


memcpy(buf, state->x.next, n);
state->x.have -= n;
state->x.next += n;
state->x.pos += n;
left -= n;
buf += n;
} while (left && eol == NULL);


if (buf == str)
return NULL;
buf[0] = 0;
return str;
}


int ZEXPORT gzdirect(file)
gzFile file;
{
gz_statep state;


if (file == NULL)
return 0;
state = (gz_statep)file;



if (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
(void)gz_look(state);


return state->direct;
}


int ZEXPORT gzclose_r(file)
gzFile file;
{
int ret, err;
gz_statep state;


if (file == NULL)
return Z_STREAM_ERROR;
state = (gz_statep)file;


if (state->mode != GZ_READ)
return Z_STREAM_ERROR;


if (state->size) {
inflateEnd(&(state->strm));
free(state->out);
free(state->in);
}
err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
gz_error(state, Z_OK, NULL);
free(state->path);
ret = close(state->fd);
free(state);
return ret ? Z_ERRNO : err;
}
