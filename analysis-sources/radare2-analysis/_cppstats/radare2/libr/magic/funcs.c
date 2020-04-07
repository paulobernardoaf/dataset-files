#include <r_userconf.h>
#if !USE_LIB_MAGIC
#include "file.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#if defined(HAVE_WCHAR_H)
#include <wchar.h>
#endif
static int file_vprintf(RMagic *ms, const char *fmt, va_list ap) {
va_list ap2;
char cbuf[4096];
char *buf, *newstr;
va_copy (ap2, ap);
int len = vsnprintf (cbuf, sizeof (cbuf), fmt, ap2);
va_end (ap2);
if (len < 0) {
goto out;
}
if (len > sizeof (cbuf)) {
buf = malloc (len + 1);
va_copy (ap2, ap);
(void)vsnprintf (buf, len + 1, fmt, ap2);
va_end (ap2);
} else {
int nullbyte = len;
if (nullbyte > 0 && nullbyte == sizeof (cbuf)) {
nullbyte--;
}
cbuf[nullbyte] = 0;
buf = strdup (cbuf);
}
if (!buf) {
return -1;
}
int buflen = len;
if (ms->o.buf) {
int obuflen = strlen (ms->o.buf);
len = obuflen + buflen + 1;
newstr = malloc (len);
if (!newstr) {
free (buf);
return -1;
}
memcpy (newstr, ms->o.buf, obuflen);
memcpy (newstr + obuflen, buf, buflen);
newstr[len - 1] = 0;
free (buf);
free (ms->o.buf);
if (len < 0) {
free (newstr);
goto out;
}
buf = newstr;
}
ms->o.buf = buf;
return 0;
out:
file_error (ms, errno, "vasprintf failed");
return -1;
}
int file_printf(RMagic *ms, const char *fmt, ...) {
va_list ap;
int ret;
va_start (ap, fmt);
ret = file_vprintf (ms, fmt, ap);
va_end (ap);
return ret;
}
static void file_error_core(RMagic *ms, int error, const char *f, va_list va, ut32 lineno) {
if (!ms || ms->haderr) {
return;
}
if (lineno != 0) {
free(ms->o.buf);
ms->o.buf = NULL;
(void)file_printf (ms, "line %u: ", lineno);
}
file_vprintf (ms, f, va);
if (error > 0) {
(void)file_printf (ms, " (%s)", strerror (error));
}
ms->haderr++;
ms->error = error;
}
void file_error(RMagic *ms, int error, const char *f, ...) {
va_list va;
va_start (va, f);
file_error_core (ms, error, f, va, 0);
va_end (va);
}
void file_magerror(RMagic *ms, const char *f, ...) {
va_list va;
va_start (va, f);
file_error_core (ms, 0, f, va, ms->line);
va_end (va);
}
void file_oomem(RMagic *ms, size_t len) {
file_error (ms, errno, "cannot allocate %zu bytes", len);
}
void file_badseek(RMagic *ms) {
file_error (ms, errno, "error seeking");
}
void file_badread(RMagic *ms) {
file_error (ms, errno, "error reading");
}
int file_buffer(RMagic *ms, int fd, const char *inname, const void *buf, size_t nb) {
int mime, m = 0;
if (!ms) {
return -1;
}
mime = ms->flags & R_MAGIC_MIME;
if (nb == 0) {
if ((!mime || (mime & R_MAGIC_MIME_TYPE)) &&
file_printf (ms, mime ? "application/x-empty" : "empty") == -1) {
return -1;
}
return 1;
} else if (nb == 1) {
if ((!mime || (mime & R_MAGIC_MIME_TYPE)) &&
file_printf (ms, mime ? "application/octet-stream" : "very short file (no magic)") == -1) {
return -1;
}
return 1;
}
#if 0
if ((ms->flags & R_MAGIC_NO_CHECK_COMPRESS) != 0 ||
(m = file_zmagic(ms, fd, inname, buf, nb)) == 0) {
#endif
if ((ms->flags & R_MAGIC_NO_CHECK_TAR) != 0 ||
(m = file_is_tar(ms, buf, nb)) == 0) {
if ((ms->flags & R_MAGIC_NO_CHECK_SOFT) != 0 ||
(m = file_softmagic(ms, buf, nb, BINTEST)) == 0) {
if ((ms->flags & R_MAGIC_NO_CHECK_ASCII) != 0 ||
(m = file_ascmagic(ms, buf, nb)) == 0) {
if ((!mime || (mime & R_MAGIC_MIME_TYPE))) {
file_printf (ms, "application/octet-stream");
return -1;
}
m = 1;
}
}
}
#if 0
}
#endif
return m;
}
int file_reset(RMagic *ms) {
if (!ms) {
return 0;
}
free (ms->o.buf);
ms->o.buf = NULL;
ms->haderr = 0;
ms->error = -1;
if (!ms->mlist) {
file_error (ms, 0, "no magic files loaded! ");
return -1;
}
return 0;
}
#define OCTALIFY(n, o) (void)(*(n)++ = '\\', *(n)++ = (((ut32)*(o) >> 6) & 3) + '0', *(n)++ = (((ut32)*(o) >> 3) & 7) + '0', *(n)++ = (((ut32)*(o) >> 0) & 7) + '0', (o)++)
const char *file_getbuffer(RMagic *ms) {
char *pbuf, *op, *np;
size_t psize, len;
if (ms->haderr) {
return NULL;
}
if (ms->flags & R_MAGIC_RAW) {
return ms->o.buf;
}
if (!ms->o.buf) {
eprintf ("ms->o.buf = NULL\n");
return NULL;
}
len = strlen (ms->o.buf);
if (len > (SIZE_MAX - 1) / 4) {
file_oomem (ms, len);
return NULL;
}
psize = len * 4 + 1;
if (!(pbuf = realloc (ms->o.pbuf, psize))) {
file_oomem (ms, psize);
return NULL;
}
ms->o.pbuf = pbuf;
#if 1
{
mbstate_t state;
wchar_t nextchar;
int mb_conv = 1;
size_t bytesconsumed;
char *eop;
(void)memset(&state, 0, sizeof(mbstate_t));
np = ms->o.pbuf;
op = ms->o.buf;
eop = op + len;
while (op < eop) {
bytesconsumed = mbrtowc(&nextchar, op,
(size_t)(eop - op), &state);
if (bytesconsumed == (size_t)(-1) ||
bytesconsumed == (size_t)(-2)) {
mb_conv = 0;
break;
}
if (iswprint(nextchar)) {
(void)memcpy(np, op, bytesconsumed);
op += bytesconsumed;
np += bytesconsumed;
} else {
while (bytesconsumed-- > 0) {
OCTALIFY (np, op);
}
}
}
*np = '\0';
if (mb_conv != 0) {
return ms->o.pbuf;
}
}
#endif
for (np = ms->o.pbuf, op = ms->o.buf; *op; op++) {
if (isprint ((ut8)*op)) {
*np++ = *op; 
} else {
OCTALIFY (np, op);
}
}
*np = '\0';
return ms->o.pbuf;
}
int file_check_mem(RMagic *ms, unsigned int level) {
if (level >= ms->c.len) {
size_t len = (ms->c.len += 20) * sizeof (*ms->c.li);
ms->c.li = (!ms->c.li) ? malloc (len) :
realloc (ms->c.li, len);
if (!ms->c.li) {
file_oomem (ms, len);
return -1;
}
}
ms->c.li[level].got_match = 0;
ms->c.li[level].last_match = 0;
ms->c.li[level].last_cond = COND_NONE;
return 0;
}
#endif
