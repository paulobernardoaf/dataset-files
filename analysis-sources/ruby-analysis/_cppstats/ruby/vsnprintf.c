#include <sys/types.h>
#define u_long unsigned long
#define u_short unsigned short
#define u_int unsigned int
#if !defined(HAVE_STDARG_PROTOTYPES)
#if defined(__STDC__)
#define HAVE_STDARG_PROTOTYPES 1
#endif
#endif
#undef __P
#if defined(HAVE_STDARG_PROTOTYPES)
#include <stdarg.h>
#if !defined(__P)
#define __P(x) x
#endif
#else
#define __P(x) ()
#if !defined(const)
#define const
#endif
#include <varargs.h>
#endif
#if !defined(_BSD_VA_LIST_)
#define _BSD_VA_LIST_ va_list
#endif
#if defined(__STDC__)
#include <limits.h>
#else
#if !defined(LONG_MAX)
#if defined(HAVE_LIMITS_H)
#include <limits.h>
#else
#define LONG_MAX 2147483647
#endif
#endif
#endif
#if defined(__hpux) && !defined(__GNUC__) && !defined(__STDC__)
#define const
#endif
#if defined(sgi)
#undef __const
#define __const
#endif 
#include <stddef.h>
#if defined(__hpux) && !defined(__GNUC__) || defined(__DECC)
#include <string.h>
#endif
#if !defined(__CYGWIN32__) && defined(__hpux) && !defined(__GNUC__)
#include <stdlib.h>
#endif
#if !defined(NULL)
#define NULL 0
#endif
#if SIZEOF_LONG > SIZEOF_INT
#include <errno.h>
#endif
struct __sbuf {
unsigned char *_base;
size_t _size;
};
typedef struct __sFILE {
unsigned char *_p; 
#if 0
size_t _r; 
#endif
size_t _w; 
short _flags; 
short _file; 
struct __sbuf _bf; 
#if 0
size_t _lbfsize; 
#endif
int (*vwrite)();
const char *(*vextra)();
} FILE;
#define __SLBF 0x0001 
#define __SNBF 0x0002 
#define __SRD 0x0004 
#define __SWR 0x0008 
#define __SRW 0x0010 
#define __SEOF 0x0020 
#define __SERR 0x0040 
#define __SMBF 0x0080 
#define __SAPP 0x0100 
#define __SSTR 0x0200 
#define __SOPT 0x0400 
#define __SNPT 0x0800 
#define __SOFF 0x1000 
#define __SMOD 0x2000 
#define EOF (-1)
#define BSD__sfeof(p) (((p)->_flags & __SEOF) != 0)
#define BSD__sferror(p) (((p)->_flags & __SERR) != 0)
#define BSD__sclearerr(p) ((void)((p)->_flags &= ~(__SERR|__SEOF)))
#define BSD__sfileno(p) ((p)->_file)
#undef feof
#undef ferror
#undef clearerr
#define feof(p) BSD__sfeof(p)
#define ferror(p) BSD__sferror(p)
#define clearerr(p) BSD__sclearerr(p)
#if !defined(_ANSI_SOURCE)
#define fileno(p) BSD__sfileno(p)
#endif
struct __siov {
const void *iov_base;
size_t iov_len;
};
struct __suio {
struct __siov *uio_iov;
int uio_iovcnt;
size_t uio_resid;
};
static int
BSD__sfvwrite(register FILE *fp, register struct __suio *uio)
{
register size_t len;
register const char *p;
register struct __siov *iov;
register size_t w;
if ((len = uio->uio_resid) == 0)
return (0);
#if !defined(__hpux)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#define COPY(n) (void)memcpy((void *)fp->_p, (void *)p, (size_t)(n))
iov = uio->uio_iov;
p = iov->iov_base;
len = iov->iov_len;
iov++;
#define GETIOV(extra_work) while (len == 0) { extra_work; p = iov->iov_base; len = iov->iov_len; iov++; }
if (fp->_flags & __SNBF) {
} else if ((fp->_flags & __SLBF) == 0) {
do {
GETIOV(;);
w = fp->_w;
if (fp->_flags & __SSTR) {
if (len < w)
w = len;
COPY(w); 
fp->_w -= w;
fp->_p += w;
w = len; 
} else {
}
p += w;
len -= w;
} while ((uio->uio_resid -= w) != 0);
} else {
}
return (0);
}
static int
BSD__sprint(FILE *fp, register struct __suio *uio)
{
register int err;
if (uio->uio_resid == 0) {
uio->uio_iovcnt = 0;
return (0);
}
err = (*fp->vwrite)(fp, uio);
uio->uio_resid = 0;
uio->uio_iovcnt = 0;
return (err);
}
static int
BSD__sbprintf(register FILE *fp, const char *fmt, va_list ap)
{
return 0;
}
#define to_digit(c) ((c) - '0')
#define is_digit(c) ((unsigned)to_digit(c) <= 9)
#define to_char(n) (char)((n) + '0')
#if defined(_HAVE_SANE_QUAD_)
static char *
BSD__uqtoa(register u_quad_t val, char *endp, int base, int octzero, const char *xdigs)
{
register char *cp = endp;
register quad_t sval;
switch (base) {
case 10:
if (val < 10) { 
*--cp = to_char(val);
return (cp);
}
if (val > LLONG_MAX) {
*--cp = to_char(val % 10);
sval = val / 10;
} else
sval = val;
do {
*--cp = to_char(sval % 10);
sval /= 10;
} while (sval != 0);
break;
case 8:
do {
*--cp = to_char(val & 7);
val >>= 3;
} while (val);
if (octzero && *cp != '0')
*--cp = '0';
break;
case 16:
do {
*--cp = xdigs[val & 15];
val >>= 4;
} while (val);
break;
default: 
break; 
}
return (cp);
}
#endif 
static char *
BSD__ultoa(register u_long val, char *endp, int base, int octzero, const char *xdigs)
{
register char *cp = endp;
register long sval;
switch (base) {
case 10:
if (val < 10) { 
*--cp = to_char(val);
return (cp);
}
if (val > LONG_MAX) {
*--cp = to_char(val % 10);
sval = val / 10;
} else
sval = val;
do {
*--cp = to_char(sval % 10);
sval /= 10;
} while (sval != 0);
break;
case 8:
do {
*--cp = to_char(val & 7);
val >>= 3;
} while (val);
if (octzero && *cp != '0')
*--cp = '0';
break;
case 16:
do {
*--cp = xdigs[val & 15];
val >>= 4;
} while (val);
break;
default: 
break; 
}
return (cp);
}
#if defined(FLOATING_POINT)
#include <math.h>
#include <float.h>
#if !defined(MAXEXP)
#if DBL_MAX_10_EXP > -DBL_MIN_10_EXP
#define MAXEXP (DBL_MAX_10_EXP)
#else
#define MAXEXP (-DBL_MIN_10_EXP)
#endif
#endif
#if !defined(MAXFRACT)
#define MAXFRACT (MAXEXP*10/3)
#endif
#define BUF (MAXEXP+MAXFRACT+1) 
#define DEFPREC 6
static char *cvt(double, int, int, char *, int *, int, int *, char *);
static int exponent(char *, int, int);
#else 
#define BUF 68
#endif 
#if !defined(lower_hexdigits)
#define lower_hexdigits "0123456789abcdef"
#endif
#if !defined(upper_hexdigits)
#define upper_hexdigits "0123456789ABCDEF"
#endif
#define ALT 0x001 
#define HEXPREFIX 0x002 
#define LADJUST 0x004 
#define LONGDBL 0x008 
#define LONGINT 0x010 
#if defined(_HAVE_SANE_QUAD_)
#define QUADINT 0x020 
#endif 
#define SHORTINT 0x040 
#define ZEROPAD 0x080 
#define FPT 0x100 
ATTRIBUTE_NO_ADDRESS_SAFETY_ANALYSIS(static ssize_t BSD_vfprintf(FILE *fp, const char *fmt0, va_list ap));
static ssize_t
BSD_vfprintf(FILE *fp, const char *fmt0, va_list ap)
{
#if defined(PRI_EXTRA_MARK)
const int PRI_EXTRA_MARK_LEN = rb_strlen_lit(PRI_EXTRA_MARK);
#endif
register const char *fmt; 
register int ch; 
register int n; 
register const char *cp;
register struct __siov *iovp;
register int flags; 
ssize_t ret; 
int width; 
int prec; 
char sign; 
#if defined(FLOATING_POINT)
char softsign; 
double _double = 0; 
int expt; 
int expsize = 0; 
int ndig = 0; 
int fprec = 0; 
char expstr[7]; 
#endif
u_long MAYBE_UNUSED(ulval) = 0; 
#if defined(_HAVE_SANE_QUAD_)
u_quad_t MAYBE_UNUSED(uqval); 
#endif 
int base; 
int dprec; 
long fieldsz; 
long realsz; 
int size; 
const char *xdigs = 0; 
#define NIOV 8
struct __suio uio; 
struct __siov iov[NIOV];
char buf[BUF]; 
char ox[4]; 
char *const ebuf = buf + sizeof(buf);
#if SIZEOF_LONG > SIZEOF_INT
long ln;
#endif
#define PADSIZE 16 
static const char blanks[PADSIZE] =
{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
static const char zeroes[PADSIZE] =
{'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
#define PRINT(ptr, len) { iovp->iov_base = (ptr); iovp->iov_len = (len); uio.uio_resid += (len); iovp++; if (++uio.uio_iovcnt >= NIOV) { if (BSD__sprint(fp, &uio)) goto error; iovp = iov; } }
#define PAD(howmany, with) { if ((n = (howmany)) > 0) { while (n > PADSIZE) { PRINT((with), PADSIZE); n -= PADSIZE; } PRINT((with), n); } }
#if SIZEOF_LONG > SIZEOF_INT
#define PAD_L(howmany, with) { ln = (howmany); if ((long)((int)ln) != ln) { errno = ENOMEM; goto error; } if (ln > 0) PAD((int)ln, (with)); }
#else
#define PAD_L(howmany, with) PAD((howmany), (with))
#endif
#define FLUSH() { if (uio.uio_resid && BSD__sprint(fp, &uio)) goto error; uio.uio_iovcnt = 0; iovp = iov; }
#define SARG() (flags&LONGINT ? va_arg(ap, long) : flags&SHORTINT ? (long)(short)va_arg(ap, int) : (long)va_arg(ap, int))
#define UARG() (flags&LONGINT ? va_arg(ap, u_long) : flags&SHORTINT ? (u_long)(u_short)va_arg(ap, int) : (u_long)va_arg(ap, u_int))
if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
fp->_file >= 0)
return (BSD__sbprintf(fp, fmt0, ap));
fmt = fmt0;
uio.uio_iov = iovp = iov;
uio.uio_resid = 0;
uio.uio_iovcnt = 0;
ret = 0;
xdigs = 0;
for (;;) {
size_t nc;
for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
;
if ((nc = fmt - cp) != 0) {
PRINT(cp, nc);
ret += nc;
}
if (ch == '\0')
goto done;
fmt++; 
flags = 0;
dprec = 0;
width = 0;
prec = -1;
sign = '\0';
rflag: ch = *fmt++;
reswitch: switch (ch) {
case ' ':
if (!sign)
sign = ' ';
goto rflag;
case '#':
flags |= ALT;
goto rflag;
case '*':
if ((width = va_arg(ap, int)) >= 0)
goto rflag;
width = -width;
case '-':
flags |= LADJUST;
goto rflag;
case '+':
sign = '+';
goto rflag;
case '.':
if ((ch = *fmt++) == '*') {
n = va_arg(ap, int);
prec = n < 0 ? -1 : n;
goto rflag;
}
n = 0;
while (is_digit(ch)) {
n = 10 * n + to_digit(ch);
ch = *fmt++;
}
prec = n < 0 ? -1 : n;
goto reswitch;
case '0':
flags |= ZEROPAD;
goto rflag;
case '1': case '2': case '3': case '4':
case '5': case '6': case '7': case '8': case '9':
n = 0;
do {
n = 10 * n + to_digit(ch);
ch = *fmt++;
} while (is_digit(ch));
width = n;
goto reswitch;
#if defined(FLOATING_POINT)
case 'L':
flags |= LONGDBL;
goto rflag;
#endif
case 'h':
flags |= SHORTINT;
goto rflag;
#if SIZEOF_PTRDIFF_T == SIZEOF_LONG
case 't':
#endif
#if SIZEOF_SIZE_T == SIZEOF_LONG
case 'z':
#endif
case 'l':
#if defined(_HAVE_SANE_QUAD_)
if (*fmt == 'l') {
fmt++;
flags |= QUADINT;
} else {
flags |= LONGINT;
}
#else
flags |= LONGINT;
#endif
goto rflag;
#if defined(_HAVE_SANE_QUAD_)
#if SIZEOF_PTRDIFF_T == SIZEOF_LONG_LONG
case 't':
#endif
#if SIZEOF_SIZE_T == SIZEOF_LONG_LONG
case 'z':
#endif
case 'q':
flags |= QUADINT;
goto rflag;
#endif 
#if defined(_WIN32)
case 'I':
if (*fmt == '3' && *(fmt + 1) == '2') {
fmt += 2;
flags |= LONGINT;
}
#if defined(_HAVE_SANE_QUAD_)
else if (*fmt == '6' && *(fmt + 1) == '4') {
fmt += 2;
flags |= QUADINT;
}
#endif
else
#if defined(_HAVE_SANE_QUAD_) && SIZEOF_SIZE_T == SIZEOF_LONG_LONG
flags |= QUADINT;
#else
flags |= LONGINT;
#endif
goto rflag;
#endif
case 'c':
cp = buf;
*buf = (char)va_arg(ap, int);
size = 1;
sign = '\0';
break;
case 'i':
#if defined(_HAVE_SANE_QUAD_)
#define INTPTR_MASK (QUADINT|LONGINT|SHORTINT)
#else
#define INTPTR_MASK (LONGINT|SHORTINT)
#endif
#if defined _HAVE_SANE_QUAD_ && SIZEOF_VOIDP == SIZEOF_LONG_LONG
#define INTPTR_FLAG QUADINT
#elif SIZEOF_VOIDP == SIZEOF_LONG
#define INTPTR_FLAG LONGINT
#else
#define INTPTR_FLAG 0
#endif
#if defined(PRI_EXTRA_MARK)
#define IS_PRI_EXTRA_MARK(s) (PRI_EXTRA_MARK_LEN < 1 || (*(s) == PRI_EXTRA_MARK[0] && (PRI_EXTRA_MARK_LEN == 1 || strncmp((s)+1, &PRI_EXTRA_MARK[1], PRI_EXTRA_MARK_LEN-1) == 0)))
#else
#define PRI_EXTRA_MARK_LEN 0
#define IS_PRI_EXTRA_MARK(s) 1
#endif
if (fp->vextra && (flags & INTPTR_MASK) == INTPTR_FLAG &&
IS_PRI_EXTRA_MARK(fmt)) {
fmt += PRI_EXTRA_MARK_LEN;
FLUSH();
#if defined _HAVE_SANE_QUAD_ && SIZEOF_VOIDP == SIZEOF_LONG_LONG
uqval = va_arg(ap, u_quad_t);
cp = (*fp->vextra)(fp, sizeof(uqval), &uqval, &fieldsz, sign);
#else
ulval = va_arg(ap, u_long);
cp = (*fp->vextra)(fp, sizeof(ulval), &ulval, &fieldsz, sign);
#endif
sign = '\0';
if (!cp) goto error;
if (prec < 0) goto long_len;
size = fieldsz < prec ? (int)fieldsz : prec;
break;
}
goto decimal;
case 'D':
flags |= LONGINT;
case 'd':
decimal:
#if defined(_HAVE_SANE_QUAD_)
if (flags & QUADINT) {
uqval = va_arg(ap, quad_t);
if ((quad_t)uqval < 0) {
uqval = -(quad_t)uqval;
sign = '-';
}
} else
#endif 
{
ulval = SARG();
if ((long)ulval < 0) {
ulval = (u_long)(-(long)ulval);
sign = '-';
}
}
base = 10;
goto number;
#if defined(FLOATING_POINT)
case 'a':
case 'A':
if (prec > 0) {
flags |= ALT;
prec++;
fprec = prec;
}
goto fp_begin;
case 'e': 
case 'E':
if (prec != 0)
flags |= ALT;
prec = (prec == -1) ?
DEFPREC + 1 : (fprec = prec + 1);
goto fp_begin;
case 'f': 
if (prec != 0)
flags |= ALT;
case 'g':
case 'G':
if (prec == -1)
prec = DEFPREC;
else
fprec = prec;
fp_begin: _double = va_arg(ap, double);
if (isinf(_double)) {
if (_double < 0)
sign = '-';
cp = "Inf";
size = 3;
break;
}
if (isnan(_double)) {
cp = "NaN";
size = 3;
break;
}
flags |= FPT;
cp = cvt(_double, (prec < MAXFRACT ? prec : MAXFRACT), flags, &softsign,
&expt, ch, &ndig, buf);
if (ch == 'g' || ch == 'G') {
if (expt <= -4 || (expt > prec && expt > 1))
ch = (ch == 'g') ? 'e' : 'E';
else
ch = 'g';
}
if (ch == 'a' || ch == 'A') {
flags |= HEXPREFIX;
--expt;
expsize = exponent(expstr, expt, ch + 'p' - 'a');
ch += 'x' - 'a';
size = expsize + ndig;
if (ndig > 1 || flags & ALT)
++size; 
}
else if (ch <= 'e') { 
--expt;
expsize = exponent(expstr, expt, ch);
size = expsize + ndig;
if (ndig > 1 || flags & ALT)
++fprec, ++size;
} else if (ch == 'f') { 
if (expt > 0) {
size = expt;
if (prec || flags & ALT)
size += prec + 1;
} else if (!prec) { 
size = 1;
if (flags & ALT)
size += 1;
} else 
size = prec + 2;
} else if (expt >= ndig) { 
size = expt;
if (flags & ALT)
++size;
} else
size = ndig + (expt > 0 ?
1 : 2 - expt);
if (softsign)
sign = '-';
break;
#endif 
case 'n':
#if defined(_HAVE_SANE_QUAD_)
if (flags & QUADINT)
*va_arg(ap, quad_t *) = ret;
else if (flags & LONGINT)
#else 
if (flags & LONGINT)
#endif 
*va_arg(ap, long *) = ret;
else if (flags & SHORTINT)
*va_arg(ap, short *) = (short)ret;
else
*va_arg(ap, int *) = (int)ret;
continue; 
case 'O':
flags |= LONGINT;
case 'o':
#if defined(_HAVE_SANE_QUAD_)
if (flags & QUADINT)
uqval = va_arg(ap, u_quad_t);
else
#endif 
ulval = UARG();
base = 8;
goto nosign;
case 'p':
prec = (int)(sizeof(void*)*CHAR_BIT/4);
#if defined(_HAVE_LLP64_)
uqval = (u_quad_t)va_arg(ap, void *);
flags = (flags) | QUADINT | HEXPREFIX;
#else
ulval = (u_long)va_arg(ap, void *);
#if defined(_HAVE_SANE_QUAD_)
flags = (flags & ~QUADINT) | HEXPREFIX;
#else 
flags = (flags) | HEXPREFIX;
#endif 
#endif
base = 16;
xdigs = lower_hexdigits;
ch = 'x';
goto nosign;
case 's':
if ((cp = va_arg(ap, char *)) == NULL)
cp = "(null)";
if (prec >= 0) {
const char *p = (char *)memchr(cp, 0, prec);
if (p != NULL && (p - cp) < prec)
size = (int)(p - cp);
else
size = prec;
}
else {
fieldsz = strlen(cp);
goto long_len;
}
sign = '\0';
break;
case 'U':
flags |= LONGINT;
case 'u':
#if defined(_HAVE_SANE_QUAD_)
if (flags & QUADINT)
uqval = va_arg(ap, u_quad_t);
else
#endif 
ulval = UARG();
base = 10;
goto nosign;
case 'X':
xdigs = upper_hexdigits;
goto hex;
case 'x':
xdigs = lower_hexdigits;
hex:
#if defined(_HAVE_SANE_QUAD_)
if (flags & QUADINT)
uqval = va_arg(ap, u_quad_t);
else
#endif 
ulval = UARG();
base = 16;
if (flags & ALT &&
#if defined(_HAVE_SANE_QUAD_)
(flags & QUADINT ? uqval != 0 : ulval != 0)
#else 
ulval != 0
#endif 
)
flags |= HEXPREFIX;
nosign: sign = '\0';
number: if ((dprec = prec) >= 0)
flags &= ~ZEROPAD;
cp = ebuf;
#if defined(_HAVE_SANE_QUAD_)
if (flags & QUADINT) {
if (uqval != 0 || prec != 0)
cp = BSD__uqtoa(uqval, ebuf, base,
flags & ALT, xdigs);
} else
#else 
#endif 
{
if (ulval != 0 || prec != 0)
cp = BSD__ultoa(ulval, ebuf, base,
flags & ALT, xdigs);
}
size = (int)(ebuf - cp);
break;
default: 
if (ch == '\0')
goto done;
cp = buf;
*buf = ch;
size = 1;
sign = '\0';
break;
}
fieldsz = size;
long_len:
realsz = dprec > fieldsz ? dprec : fieldsz;
if (sign)
realsz++;
if (flags & HEXPREFIX)
realsz += 2;
if ((flags & (LADJUST|ZEROPAD)) == 0)
PAD_L(width - realsz, blanks);
if (sign) {
PRINT(&sign, 1);
}
if (flags & HEXPREFIX) {
ox[0] = '0';
ox[1] = ch;
PRINT(ox, 2);
}
if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
PAD_L(width - realsz, zeroes);
PAD_L(dprec - fieldsz, zeroes);
#if defined(FLOATING_POINT)
if ((flags & FPT) == 0) {
PRINT(cp, fieldsz);
} else { 
if (flags & HEXPREFIX) {
if (ndig > 1 || flags & ALT) {
ox[2] = *cp++;
ox[3] = '.';
PRINT(ox+2, 2);
if (ndig > 0) PRINT(cp, ndig-1);
} else 
PRINT(cp, 1);
PAD(fprec-ndig, zeroes);
PRINT(expstr, expsize);
}
else if (ch >= 'f') { 
if (_double == 0) {
if (ndig <= 1 &&
(flags & ALT) == 0) {
PRINT("0", 1);
} else {
PRINT("0.", 2);
PAD((ndig >= fprec ? ndig - 1 : fprec - (ch != 'f')),
zeroes);
}
} else if (expt == 0 && ndig == 0 && (flags & ALT) == 0) {
PRINT("0", 1);
} else if (expt <= 0) {
PRINT("0.", 2);
PAD(-expt, zeroes);
PRINT(cp, ndig);
if (flags & ALT)
PAD(fprec - ndig + (ch == 'f' ? expt : 0), zeroes);
} else if (expt >= ndig) {
PRINT(cp, ndig);
PAD(expt - ndig, zeroes);
if (flags & ALT)
PRINT(".", 1);
} else {
PRINT(cp, expt);
cp += expt;
PRINT(".", 1);
PRINT(cp, ndig-expt);
if (flags & ALT)
PAD(fprec - ndig + (ch == 'f' ? expt : 0), zeroes);
}
} else { 
if (ndig > 1 || flags & ALT) {
ox[0] = *cp++;
ox[1] = '.';
PRINT(ox, 2);
if (_double ) {
PRINT(cp, ndig-1);
} else 
PAD(ndig - 1, zeroes);
if (flags & ALT) PAD(fprec - ndig - 1, zeroes);
} else 
PRINT(cp, 1);
PRINT(expstr, expsize);
}
}
#else
PRINT(cp, fieldsz);
#endif
if (flags & LADJUST)
PAD_L(width - realsz, blanks);
ret += width > realsz ? width : realsz;
FLUSH(); 
}
done:
FLUSH();
error:
return (BSD__sferror(fp) ? EOF : ret);
}
#if defined(FLOATING_POINT)
extern char *BSD__dtoa(double, int, int, int *, int *, char **);
extern char *BSD__hdtoa(double, const char *, int, int *, int *, char **);
static char *
cvt(double value, int ndigits, int flags, char *sign, int *decpt, int ch, int *length, char *buf)
{
int mode, dsgn;
char *digits, *bp, *rve;
if (ch == 'f')
mode = 3;
else {
mode = 2;
}
if (value < 0) {
value = -value;
*sign = '-';
} else if (value == 0.0 && signbit(value)) {
*sign = '-';
} else {
*sign = '\000';
}
if (ch == 'a' || ch =='A') {
digits = BSD__hdtoa(value,
ch == 'a' ? lower_hexdigits : upper_hexdigits,
ndigits, decpt, &dsgn, &rve);
}
else {
digits = BSD__dtoa(value, mode, ndigits, decpt, &dsgn, &rve);
}
buf[0] = 0; 
memcpy(buf, digits, rve - digits);
xfree(digits);
rve = buf + (rve - digits);
digits = buf;
if (flags & ALT) { 
bp = digits + ndigits;
if (ch == 'f') {
if (*digits == '0' && value)
*decpt = -ndigits + 1;
bp += *decpt;
}
while (rve < bp)
*rve++ = '0';
}
*length = (int)(rve - digits);
return (digits);
}
static int
exponent(char *p0, int exp, int fmtch)
{
register char *p, *t;
char expbuf[2 + (MAXEXP < 1000 ? 3 : MAXEXP < 10000 ? 4 : 5)]; 
p = p0;
*p++ = fmtch;
if (exp < 0) {
exp = -exp;
*p++ = '-';
}
else
*p++ = '+';
t = expbuf + sizeof(expbuf);
if (exp > 9) {
do {
*--t = to_char(exp % 10);
} while ((exp /= 10) > 9);
*--t = to_char(exp);
for (; t < expbuf + sizeof(expbuf); *p++ = *t++);
}
else {
if (fmtch & 15) *p++ = '0'; 
*p++ = to_char(exp);
}
return (int)(p - p0);
}
#endif 
