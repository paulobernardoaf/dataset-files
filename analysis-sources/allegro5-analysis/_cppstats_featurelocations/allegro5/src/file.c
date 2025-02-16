















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_file.h"




ALLEGRO_FILE *al_fopen(const char *path, const char *mode)
{
return al_fopen_interface(al_get_new_file_interface(), path, mode);
}




ALLEGRO_FILE *al_fopen_interface(const ALLEGRO_FILE_INTERFACE *drv,
const char *path, const char *mode)
{
ALLEGRO_FILE *f = NULL;

ASSERT(drv);
ASSERT(path);
ASSERT(mode);

if (drv->fi_fopen) {
f = al_malloc(sizeof(*f));
if (!f) {
al_set_errno(ENOMEM);
}
else {
f->vtable = drv;
f->userdata = drv->fi_fopen(path, mode);
f->ungetc_len = 0;
if (!f->userdata) {
al_free(f);
f = NULL;
}
}
}

return f;
}




ALLEGRO_FILE *al_create_file_handle(const ALLEGRO_FILE_INTERFACE *drv,
void *userdata)
{
ALLEGRO_FILE *f;

ASSERT(drv);

f = al_malloc(sizeof(*f));
if (!f) {
al_set_errno(ENOMEM);
}
else {
f->vtable = drv;
f->userdata = userdata;
f->ungetc_len = 0;
}

return f;
}




bool al_fclose(ALLEGRO_FILE *f)
{
if (f) {
bool ret = f->vtable->fi_fclose(f);
al_free(f);
return ret;
}

al_set_errno(EINVAL);
return false;
}




size_t al_fread(ALLEGRO_FILE *f, void *ptr, size_t size)
{
ASSERT(f);
ASSERT(ptr || size == 0);

if (f->ungetc_len) {
int bytes_ungetc = 0;
unsigned char *cptr = ptr;

while (f->ungetc_len > 0 && size > 0) {
*cptr++ = f->ungetc[--f->ungetc_len];
++bytes_ungetc;
--size;
}

return bytes_ungetc + f->vtable->fi_fread(f, cptr, size);
}
else {
return f->vtable->fi_fread(f, ptr, size);
}
}




size_t al_fwrite(ALLEGRO_FILE *f, const void *ptr, size_t size)
{
ASSERT(f);
ASSERT(ptr || size == 0);

f->ungetc_len = 0;
return f->vtable->fi_fwrite(f, ptr, size);
}




bool al_fflush(ALLEGRO_FILE *f)
{
ASSERT(f);

return f->vtable->fi_fflush(f);
}




int64_t al_ftell(ALLEGRO_FILE *f)
{
ASSERT(f);

return f->vtable->fi_ftell(f) - f->ungetc_len;
}




bool al_fseek(ALLEGRO_FILE *f, int64_t offset, int whence)
{
ASSERT(f);

ASSERT(
whence == ALLEGRO_SEEK_SET ||
whence == ALLEGRO_SEEK_CUR ||
whence == ALLEGRO_SEEK_END
);

if (f->ungetc_len) {
if (whence == ALLEGRO_SEEK_CUR) {
offset -= f->ungetc_len;
}
f->ungetc_len = 0;
}

return f->vtable->fi_fseek(f, offset, whence);
}




bool al_feof(ALLEGRO_FILE *f)
{
ASSERT(f);

return f->ungetc_len == 0 && f->vtable->fi_feof(f);
}




int al_ferror(ALLEGRO_FILE *f)
{
ASSERT(f);

return f->vtable->fi_ferror(f);
}




const char *al_ferrmsg(ALLEGRO_FILE *f)
{
const char *msg;

ASSERT(f);
msg = f->vtable->fi_ferrmsg(f);
ASSERT(msg);
return msg;
}




void al_fclearerr(ALLEGRO_FILE *f)
{
ASSERT(f);

f->vtable->fi_fclearerr(f);
}




int al_fgetc(ALLEGRO_FILE *f)
{
uint8_t c;
ASSERT(f);

if (al_fread(f, &c, 1) != 1) {
return EOF;
}

return c;
}




int al_fputc(ALLEGRO_FILE *f, int c)
{
uint8_t b = (c & 0xff);
ASSERT(f);

if (al_fwrite(f, &b, 1) != 1) {
return EOF;
}

return b;
}




int16_t al_fread16le(ALLEGRO_FILE *f)
{
unsigned char b[2];
ASSERT(f);

if (al_fread(f, b, 2) == 2) {
return (((int16_t)b[1] << 8) | (int16_t)b[0]);
}

return EOF;
}




int32_t al_fread32le(ALLEGRO_FILE *f)
{
unsigned char b[4];
ASSERT(f);

if (al_fread(f, b, 4) == 4) {
return (((int32_t)b[3] << 24) | ((int32_t)b[2] << 16) |
((int32_t)b[1] << 8) | (int32_t)b[0]);
}

return EOF;
}




size_t al_fwrite16le(ALLEGRO_FILE *f, int16_t w)
{
uint8_t b1, b2;
ASSERT(f);

b1 = (w & 0xFF00) >> 8;
b2 = w & 0x00FF;

if (al_fputc(f, b2) == b2) {
if (al_fputc(f, b1) == b1) {
return 2;
}
return 1;
}
return 0;
}




size_t al_fwrite32le(ALLEGRO_FILE *f, int32_t l)
{
uint8_t b1, b2, b3, b4;
ASSERT(f);

b1 = ((l & 0xFF000000L) >> 24);
b2 = ((l & 0x00FF0000L) >> 16);
b3 = ((l & 0x0000FF00L) >> 8);
b4 = l & 0x00FF;

if (al_fputc(f, b4) == b4) {
if (al_fputc(f, b3) == b3) {
if (al_fputc(f, b2) == b2) {
if (al_fputc(f, b1) == b1) {
return 4;
}
return 3;
}
return 2;
}
return 1;
}
return 0;
}




int16_t al_fread16be(ALLEGRO_FILE *f)
{
unsigned char b[2];
ASSERT(f);

if (al_fread(f, b, 2) == 2) {
return (((int16_t)b[0] << 8) | (int16_t)b[1]);
}

return EOF;
}




int32_t al_fread32be(ALLEGRO_FILE *f)
{
unsigned char b[4];
ASSERT(f);

if (al_fread(f, b, 4) == 4) {
return (((int32_t)b[0] << 24) | ((int32_t)b[1] << 16) |
((int32_t)b[2] << 8) | (int32_t)b[3]);
}

return EOF;
}




size_t al_fwrite16be(ALLEGRO_FILE *f, int16_t w)
{
uint8_t b1, b2;
ASSERT(f);

b1 = (w & 0xFF00) >> 8;
b2 = w & 0x00FF;

if (al_fputc(f, b1) == b1) {
if (al_fputc(f, b2) == b2) {
return 2;
}
return 1;
}
return 0;
}




size_t al_fwrite32be(ALLEGRO_FILE *f, int32_t l)
{
uint8_t b1, b2, b3, b4;
ASSERT(f);

b1 = ((l & 0xFF000000L) >> 24);
b2 = ((l & 0x00FF0000L) >> 16);
b3 = ((l & 0x0000FF00L) >> 8);
b4 = l & 0x00FF;

if (al_fputc(f, b1) == b1) {
if (al_fputc(f, b2) == b2) {
if (al_fputc(f, b3) == b3) {
if (al_fputc(f, b4) == b4) {
return 4;
}
return 3;
}
return 2;
}
return 1;
}
return 0;
}




char *al_fgets(ALLEGRO_FILE *f, char * const buf, size_t max)
{
char *p = buf;
int c;
ASSERT(f);
ASSERT(buf);


if (max == 0) {
return NULL;
}
if (max == 1) {
*buf = '\0';
return buf;
}


if ((c = al_fgetc(f)) == EOF) {
return NULL;
}


do {
*p++ = c;
max--;
if (max == 1 || c == '\n')
break;
c = al_fgetc(f);
} while (c != EOF);


if (c == EOF && al_ferror(f)) {
return NULL;
}


ASSERT(max >= 1);
*p = '\0';

return buf;
}




ALLEGRO_USTR *al_fget_ustr(ALLEGRO_FILE *f)
{
ALLEGRO_USTR *us;
char buf[128];

if (!al_fgets(f, buf, sizeof(buf))) {
return NULL;
}

us = al_ustr_new("");

do {
al_ustr_append_cstr(us, buf);
if (al_ustr_has_suffix_cstr(us, "\n"))
break;
} while (al_fgets(f, buf, sizeof(buf)));

return us;
}




int al_fputs(ALLEGRO_FILE *f, char const *p)
{
size_t n;
ASSERT(f);
ASSERT(p);

n = strlen(p);
if (al_fwrite(f, p, n) != n) {
return EOF;
}

return n;
}




int al_fungetc(ALLEGRO_FILE *f, int c)
{
ASSERT(f != NULL);

if (f->vtable->fi_fungetc) {
return f->vtable->fi_fungetc(f, c);
}
else {





if (f->ungetc_len == ALLEGRO_UNGETC_SIZE) {
return EOF;
}

f->ungetc[f->ungetc_len++] = (unsigned char) c;

return c;
}
}




int64_t al_fsize(ALLEGRO_FILE *f)
{
ASSERT(f != NULL);

return f->vtable->fi_fsize(f);
}




void *al_get_file_userdata(ALLEGRO_FILE *f)
{
ASSERT(f != NULL);

return f->userdata;
}




int al_vfprintf(ALLEGRO_FILE *pfile, const char *format, va_list args)
{
int rv = -1;
ALLEGRO_USTR *ustr = 0;
size_t size = 0;
bool success;

if (pfile != 0 && format != 0)
{
ustr = al_ustr_new("");
if (ustr)
{
success = al_ustr_vappendf(ustr, format, args);
if (success)
{
size = al_ustr_size(ustr);
if (size > 0)
{
rv = al_fwrite(pfile, (const void*)(al_cstr(ustr)), size);
if (rv != (int)size) {
rv = -1;
}
}
}
al_ustr_free(ustr);
}
}
return rv;
}




int al_fprintf(ALLEGRO_FILE *pfile, const char *format, ...)
{
int rv = -1;
va_list args;

if (pfile != 0 && format != 0)
{
va_start(args, format);
rv = al_vfprintf(pfile, format, args);
va_end(args);
}
return rv;
}



