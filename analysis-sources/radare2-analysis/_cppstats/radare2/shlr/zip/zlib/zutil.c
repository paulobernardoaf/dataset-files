#include "zutil.h"
#if !defined(Z_SOLO)
#include "gzguts.h"
#endif
z_const char * const z_errmsg[10] = {
(z_const char *)"need dictionary", 
(z_const char *)"stream end", 
(z_const char *)"", 
(z_const char *)"file error", 
(z_const char *)"stream error", 
(z_const char *)"data error", 
(z_const char *)"insufficient memory", 
(z_const char *)"buffer error", 
(z_const char *)"incompatible version",
(z_const char *)""
};
const char * ZEXPORT zlibVersion()
{
return ZLIB_VERSION;
}
uLong ZEXPORT zlibCompileFlags()
{
uLong flags;
flags = 0;
switch ((int)(sizeof(uInt))) {
case 2: break;
case 4: flags += 1; break;
case 8: flags += 2; break;
default: flags += 3;
}
switch ((int)(sizeof(uLong))) {
case 2: break;
case 4: flags += 1 << 2; break;
case 8: flags += 2 << 2; break;
default: flags += 3 << 2;
}
switch ((int)(sizeof(voidpf))) {
case 2: break;
case 4: flags += 1 << 4; break;
case 8: flags += 2 << 4; break;
default: flags += 3 << 4;
}
switch ((int)(sizeof(z_off_t))) {
case 2: break;
case 4: flags += 1 << 6; break;
case 8: flags += 2 << 6; break;
default: flags += 3 << 6;
}
#if defined(ZLIB_DEBUG)
flags += 1 << 8;
#endif
#if defined(ASMV) || defined(ASMINF)
flags += 1 << 9;
#endif
#if defined(ZLIB_WINAPI)
flags += 1 << 10;
#endif
#if defined(BUILDFIXED)
flags += 1 << 12;
#endif
#if defined(DYNAMIC_CRC_TABLE)
flags += 1 << 13;
#endif
#if defined(NO_GZCOMPRESS)
flags += 1L << 16;
#endif
#if defined(NO_GZIP)
flags += 1L << 17;
#endif
#if defined(PKZIP_BUG_WORKAROUND)
flags += 1L << 20;
#endif
#if defined(FASTEST)
flags += 1L << 21;
#endif
#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#if defined(NO_vsnprintf)
flags += 1L << 25;
#if defined(HAS_vsprintf_void)
flags += 1L << 26;
#endif
#else
#if defined(HAS_vsnprintf_void)
flags += 1L << 26;
#endif
#endif
#else
flags += 1L << 24;
#if defined(NO_snprintf)
flags += 1L << 25;
#if defined(HAS_sprintf_void)
flags += 1L << 26;
#endif
#else
#if defined(HAS_snprintf_void)
flags += 1L << 26;
#endif
#endif
#endif
return flags;
}
#if defined(ZLIB_DEBUG)
#include <stdlib.h>
#if !defined(verbose)
#define verbose 0
#endif
int ZLIB_INTERNAL z_verbose = verbose;
void ZLIB_INTERNAL z_error (m)
char *m;
{
fprintf(stderr, "%s\n", m);
exit(1);
}
#endif
const char * ZEXPORT zError(err)
int err;
{
return ERR_MSG(err);
}
#if defined(_WIN32_WCE)
int errno = 0;
#endif
#if !defined(HAVE_MEMCPY)
void ZLIB_INTERNAL zmemcpy(dest, source, len)
Bytef* dest;
const Bytef* source;
uInt len;
{
if (len == 0) return;
do {
*dest++ = *source++; 
} while (--len != 0);
}
int ZLIB_INTERNAL zmemcmp(s1, s2, len)
const Bytef* s1;
const Bytef* s2;
uInt len;
{
uInt j;
for (j = 0; j < len; j++) {
if (s1[j] != s2[j]) return 2*(s1[j] > s2[j])-1;
}
return 0;
}
void ZLIB_INTERNAL zmemzero(dest, len)
Bytef* dest;
uInt len;
{
if (len == 0) return;
do {
*dest++ = 0; 
} while (--len != 0);
}
#endif
#if !defined(Z_SOLO)
#if defined(SYS16BIT)
#if defined(__TURBOC__)
#define MY_ZCALLOC
#define MAX_PTR 10
local int next_ptr = 0;
typedef struct ptr_table_s {
voidpf org_ptr;
voidpf new_ptr;
} ptr_table;
local ptr_table table[MAX_PTR];
voidpf ZLIB_INTERNAL zcalloc (voidpf opaque, unsigned items, unsigned size)
{
voidpf buf;
ulg bsize = (ulg)items*size;
(void)opaque;
if (bsize < 65520L) {
buf = farmalloc(bsize);
if (*(ush*)&buf != 0) return buf;
} else {
buf = farmalloc(bsize + 16L);
}
if (buf == NULL || next_ptr >= MAX_PTR) return NULL;
table[next_ptr].org_ptr = buf;
*((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
*(ush*)&buf = 0;
table[next_ptr++].new_ptr = buf;
return buf;
}
void ZLIB_INTERNAL zcfree (voidpf opaque, voidpf ptr)
{
int n;
(void)opaque;
if (*(ush*)&ptr != 0) { 
farfree(ptr);
return;
}
for (n = 0; n < next_ptr; n++) {
if (ptr != table[n].new_ptr) continue;
farfree(table[n].org_ptr);
while (++n < next_ptr) {
table[n-1] = table[n];
}
next_ptr--;
return;
}
Assert(0, "zcfree: ptr not found");
}
#endif 
#if defined(M_I86)
#define MY_ZCALLOC
#if (!defined(_MSC_VER) || (_MSC_VER <= 600))
#define _halloc halloc
#define _hfree hfree
#endif
voidpf ZLIB_INTERNAL zcalloc (voidpf opaque, uInt items, uInt size)
{
(void)opaque;
return _halloc((long)items, size);
}
void ZLIB_INTERNAL zcfree (voidpf opaque, voidpf ptr)
{
(void)opaque;
_hfree(ptr);
}
#endif 
#endif 
#if !defined(MY_ZCALLOC)
#if !defined(STDC)
extern voidp malloc OF((uInt size));
extern voidp calloc OF((uInt items, uInt size));
extern void free OF((voidpf ptr));
#endif
voidpf ZLIB_INTERNAL zcalloc (opaque, items, size)
voidpf opaque;
unsigned items;
unsigned size;
{
(void)opaque;
return sizeof(uInt) > 2 ? (voidpf)malloc(items * size) :
(voidpf)calloc(items, size);
}
void ZLIB_INTERNAL zcfree (opaque, ptr)
voidpf opaque;
voidpf ptr;
{
(void)opaque;
free(ptr);
}
#endif 
#endif 
