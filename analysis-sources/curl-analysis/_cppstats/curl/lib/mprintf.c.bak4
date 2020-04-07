




































#include "curl_setup.h"
#include <curl/mprintf.h>

#include "curl_memory.h"

#include "memdebug.h"





#if defined(HAVE_LONGLONG)
#define LONG_LONG_TYPE long long
#define HAVE_LONG_LONG_TYPE
#else
#if defined(_MSC_VER) && (_MSC_VER >= 900) && (_INTEGRAL_MAX_BITS >= 64)
#define LONG_LONG_TYPE __int64
#define HAVE_LONG_LONG_TYPE
#else
#undef LONG_LONG_TYPE
#undef HAVE_LONG_LONG_TYPE
#endif
#endif





#if (defined(__BORLANDC__) && (__BORLANDC__ >= 0x520)) || (defined(__WATCOMC__) && defined(__386__)) || (defined(__POCC__) && defined(_MSC_VER)) || (defined(_WIN32_WCE)) || (defined(__MINGW32__)) || (defined(_MSC_VER) && (_MSC_VER >= 900) && (_INTEGRAL_MAX_BITS >= 64))





#define MP_HAVE_INT_EXTENSIONS
#endif





#if defined(HAVE_LONG_LONG_TYPE)
#define mp_intmax_t LONG_LONG_TYPE
#define mp_uintmax_t unsigned LONG_LONG_TYPE
#else
#define mp_intmax_t long
#define mp_uintmax_t unsigned long
#endif

#define BUFFSIZE 326 

#define MAX_PARAMETERS 128 

#if defined(__AMIGA__)
#undef FORMAT_INT
#endif


static const char lower_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";


static const char upper_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define OUTCHAR(x) do{ if(stream((unsigned char)(x), (FILE *)data) != -1) done++; else return done; } while(0)








typedef enum {
FORMAT_UNKNOWN = 0,
FORMAT_STRING,
FORMAT_PTR,
FORMAT_INT,
FORMAT_INTPTR,
FORMAT_LONG,
FORMAT_LONGLONG,
FORMAT_DOUBLE,
FORMAT_LONGDOUBLE,
FORMAT_WIDTH 
} FormatType;


enum {
FLAGS_NEW = 0,
FLAGS_SPACE = 1<<0,
FLAGS_SHOWSIGN = 1<<1,
FLAGS_LEFT = 1<<2,
FLAGS_ALT = 1<<3,
FLAGS_SHORT = 1<<4,
FLAGS_LONG = 1<<5,
FLAGS_LONGLONG = 1<<6,
FLAGS_LONGDOUBLE = 1<<7,
FLAGS_PAD_NIL = 1<<8,
FLAGS_UNSIGNED = 1<<9,
FLAGS_OCTAL = 1<<10,
FLAGS_HEX = 1<<11,
FLAGS_UPPER = 1<<12,
FLAGS_WIDTH = 1<<13, 
FLAGS_WIDTHPARAM = 1<<14, 
FLAGS_PREC = 1<<15, 
FLAGS_PRECPARAM = 1<<16, 
FLAGS_CHAR = 1<<17, 
FLAGS_FLOATE = 1<<18, 
FLAGS_FLOATG = 1<<19 
};

typedef struct {
FormatType type;
int flags;
long width; 
long precision; 
union {
char *str;
void *ptr;
union {
mp_intmax_t as_signed;
mp_uintmax_t as_unsigned;
} num;
double dnum;
} data;
} va_stack_t;

struct nsprintf {
char *buffer;
size_t length;
size_t max;
};

struct asprintf {
char *buffer; 
size_t len; 
size_t alloc; 
int fail; 

};

static long dprintf_DollarString(char *input, char **end)
{
int number = 0;
while(ISDIGIT(*input)) {
number *= 10;
number += *input-'0';
input++;
}
if(number && ('$'==*input++)) {
*end = input;
return number;
}
return 0;
}

static bool dprintf_IsQualifierNoDollar(const char *fmt)
{
#if defined(MP_HAVE_INT_EXTENSIONS)
if(!strncmp(fmt, "I32", 3) || !strncmp(fmt, "I64", 3)) {
return TRUE;
}
#endif

switch(*fmt) {
case '-': case '+': case ' ': case '#': case '.':
case '0': case '1': case '2': case '3': case '4':
case '5': case '6': case '7': case '8': case '9':
case 'h': case 'l': case 'L': case 'z': case 'q':
case '*': case 'O':
#if defined(MP_HAVE_INT_EXTENSIONS)
case 'I':
#endif
return TRUE;

default:
return FALSE;
}
}











static int dprintf_Pass1(const char *format, va_stack_t *vto, char **endpos,
va_list arglist)
{
char *fmt = (char *)format;
int param_num = 0;
long this_param;
long width;
long precision;
int flags;
long max_param = 0;
long i;

while(*fmt) {
if(*fmt++ == '%') {
if(*fmt == '%') {
fmt++;
continue; 
}

flags = FLAGS_NEW;



param_num++;

this_param = dprintf_DollarString(fmt, &fmt);
if(0 == this_param)

this_param = param_num;

if(this_param > max_param)
max_param = this_param;







width = 0;
precision = 0;



while(dprintf_IsQualifierNoDollar(fmt)) {
#if defined(MP_HAVE_INT_EXTENSIONS)
if(!strncmp(fmt, "I32", 3)) {
flags |= FLAGS_LONG;
fmt += 3;
}
else if(!strncmp(fmt, "I64", 3)) {
flags |= FLAGS_LONGLONG;
fmt += 3;
}
else
#endif

switch(*fmt++) {
case ' ':
flags |= FLAGS_SPACE;
break;
case '+':
flags |= FLAGS_SHOWSIGN;
break;
case '-':
flags |= FLAGS_LEFT;
flags &= ~FLAGS_PAD_NIL;
break;
case '#':
flags |= FLAGS_ALT;
break;
case '.':
if('*' == *fmt) {


flags |= FLAGS_PRECPARAM;
fmt++;
param_num++;

i = dprintf_DollarString(fmt, &fmt);
if(i)
precision = i;
else
precision = param_num;

if(precision > max_param)
max_param = precision;
}
else {
flags |= FLAGS_PREC;
precision = strtol(fmt, &fmt, 10);
}
break;
case 'h':
flags |= FLAGS_SHORT;
break;
#if defined(MP_HAVE_INT_EXTENSIONS)
case 'I':
#if (SIZEOF_CURL_OFF_T > SIZEOF_LONG)
flags |= FLAGS_LONGLONG;
#else
flags |= FLAGS_LONG;
#endif
break;
#endif
case 'l':
if(flags & FLAGS_LONG)
flags |= FLAGS_LONGLONG;
else
flags |= FLAGS_LONG;
break;
case 'L':
flags |= FLAGS_LONGDOUBLE;
break;
case 'q':
flags |= FLAGS_LONGLONG;
break;
case 'z':


#if (SIZEOF_SIZE_T > SIZEOF_LONG)
flags |= FLAGS_LONGLONG;
#else
flags |= FLAGS_LONG;
#endif
break;
case 'O':
#if (SIZEOF_CURL_OFF_T > SIZEOF_LONG)
flags |= FLAGS_LONGLONG;
#else
flags |= FLAGS_LONG;
#endif
break;
case '0':
if(!(flags & FLAGS_LEFT))
flags |= FLAGS_PAD_NIL;

case '1': case '2': case '3': case '4':
case '5': case '6': case '7': case '8': case '9':
flags |= FLAGS_WIDTH;
width = strtol(fmt-1, &fmt, 10);
break;
case '*': 
flags |= FLAGS_WIDTHPARAM;
param_num++;

i = dprintf_DollarString(fmt, &fmt);
if(i)
width = i;
else
width = param_num;
if(width > max_param)
max_param = width;
break;
default:
break;
}
} 



i = this_param - 1;

if((i < 0) || (i >= MAX_PARAMETERS))

return 1;

switch (*fmt) {
case 'S':
flags |= FLAGS_ALT;

case 's':
vto[i].type = FORMAT_STRING;
break;
case 'n':
vto[i].type = FORMAT_INTPTR;
break;
case 'p':
vto[i].type = FORMAT_PTR;
break;
case 'd': case 'i':
vto[i].type = FORMAT_INT;
break;
case 'u':
vto[i].type = FORMAT_INT;
flags |= FLAGS_UNSIGNED;
break;
case 'o':
vto[i].type = FORMAT_INT;
flags |= FLAGS_OCTAL;
break;
case 'x':
vto[i].type = FORMAT_INT;
flags |= FLAGS_HEX|FLAGS_UNSIGNED;
break;
case 'X':
vto[i].type = FORMAT_INT;
flags |= FLAGS_HEX|FLAGS_UPPER|FLAGS_UNSIGNED;
break;
case 'c':
vto[i].type = FORMAT_INT;
flags |= FLAGS_CHAR;
break;
case 'f':
vto[i].type = FORMAT_DOUBLE;
break;
case 'e':
vto[i].type = FORMAT_DOUBLE;
flags |= FLAGS_FLOATE;
break;
case 'E':
vto[i].type = FORMAT_DOUBLE;
flags |= FLAGS_FLOATE|FLAGS_UPPER;
break;
case 'g':
vto[i].type = FORMAT_DOUBLE;
flags |= FLAGS_FLOATG;
break;
case 'G':
vto[i].type = FORMAT_DOUBLE;
flags |= FLAGS_FLOATG|FLAGS_UPPER;
break;
default:
vto[i].type = FORMAT_UNKNOWN;
break;
} 

vto[i].flags = flags;
vto[i].width = width;
vto[i].precision = precision;

if(flags & FLAGS_WIDTHPARAM) {


long k = width - 1;
vto[i].width = k;
vto[k].type = FORMAT_WIDTH;
vto[k].flags = FLAGS_NEW;

vto[k].width = 0;
vto[k].precision = 0;
}
if(flags & FLAGS_PRECPARAM) {


long k = precision - 1;
vto[i].precision = k;
vto[k].type = FORMAT_WIDTH;
vto[k].flags = FLAGS_NEW;

vto[k].width = 0;
vto[k].precision = 0;
}
*endpos++ = fmt + 1; 
}
}


for(i = 0; i<max_param; i++) {


if(vto[i].flags & FLAGS_WIDTHPARAM) {
vto[vto[i].width].data.num.as_signed =
(mp_intmax_t)va_arg(arglist, int);
}
if(vto[i].flags & FLAGS_PRECPARAM) {
vto[vto[i].precision].data.num.as_signed =
(mp_intmax_t)va_arg(arglist, int);
}

switch(vto[i].type) {
case FORMAT_STRING:
vto[i].data.str = va_arg(arglist, char *);
break;

case FORMAT_INTPTR:
case FORMAT_UNKNOWN:
case FORMAT_PTR:
vto[i].data.ptr = va_arg(arglist, void *);
break;

case FORMAT_INT:
#if defined(HAVE_LONG_LONG_TYPE)
if((vto[i].flags & FLAGS_LONGLONG) && (vto[i].flags & FLAGS_UNSIGNED))
vto[i].data.num.as_unsigned =
(mp_uintmax_t)va_arg(arglist, mp_uintmax_t);
else if(vto[i].flags & FLAGS_LONGLONG)
vto[i].data.num.as_signed =
(mp_intmax_t)va_arg(arglist, mp_intmax_t);
else
#endif
{
if((vto[i].flags & FLAGS_LONG) && (vto[i].flags & FLAGS_UNSIGNED))
vto[i].data.num.as_unsigned =
(mp_uintmax_t)va_arg(arglist, unsigned long);
else if(vto[i].flags & FLAGS_LONG)
vto[i].data.num.as_signed =
(mp_intmax_t)va_arg(arglist, long);
else if(vto[i].flags & FLAGS_UNSIGNED)
vto[i].data.num.as_unsigned =
(mp_uintmax_t)va_arg(arglist, unsigned int);
else
vto[i].data.num.as_signed =
(mp_intmax_t)va_arg(arglist, int);
}
break;

case FORMAT_DOUBLE:
vto[i].data.dnum = va_arg(arglist, double);
break;

case FORMAT_WIDTH:



vto[i].type = FORMAT_INT;
break;

default:
break;
}
}

return 0;

}

static int dprintf_formatf(
void *data, 


int (*stream)(int, FILE *),
const char *format, 
va_list ap_save) 
{

const char *digits = lower_digits;


char *f;


int done = 0;

long param; 
long param_num = 0; 

va_stack_t vto[MAX_PARAMETERS];
char *endpos[MAX_PARAMETERS];
char **end;

char work[BUFFSIZE];

va_stack_t *p;




char *workend = &work[sizeof(work) - 2];


if(dprintf_Pass1(format, vto, endpos, ap_save))
return -1;

end = &endpos[0]; 


f = (char *)format;
while(*f != '\0') {

int is_alt;


long width;


long prec;


int is_neg;


unsigned long base;


mp_uintmax_t num;


mp_intmax_t signed_num;

char *w;

if(*f != '%') {


do {
OUTCHAR(*f);
} while(*++f && ('%' != *f));
continue;
}

++f;





if(*f == '%') {
++f;
OUTCHAR('%');
continue;
}



param = dprintf_DollarString(f, &f);

if(!param)
param = param_num;
else
--param;

param_num++; 


p = &vto[param];


if(p->flags & FLAGS_WIDTHPARAM) {
width = (long)vto[p->width].data.num.as_signed;
param_num++; 

if(width < 0) {


width = -width;
p->flags |= FLAGS_LEFT;
p->flags &= ~FLAGS_PAD_NIL;
}
}
else
width = p->width;


if(p->flags & FLAGS_PRECPARAM) {
prec = (long)vto[p->precision].data.num.as_signed;
param_num++; 

if(prec < 0)


prec = -1;
}
else if(p->flags & FLAGS_PREC)
prec = p->precision;
else
prec = -1;

is_alt = (p->flags & FLAGS_ALT) ? 1 : 0;

switch(p->type) {
case FORMAT_INT:
num = p->data.num.as_unsigned;
if(p->flags & FLAGS_CHAR) {

if(!(p->flags & FLAGS_LEFT))
while(--width > 0)
OUTCHAR(' ');
OUTCHAR((char) num);
if(p->flags & FLAGS_LEFT)
while(--width > 0)
OUTCHAR(' ');
break;
}
if(p->flags & FLAGS_OCTAL) {

base = 8;
goto unsigned_number;
}
else if(p->flags & FLAGS_HEX) {


digits = (p->flags & FLAGS_UPPER)? upper_digits : lower_digits;
base = 16;
goto unsigned_number;
}
else if(p->flags & FLAGS_UNSIGNED) {

base = 10;
goto unsigned_number;
}


base = 10;

is_neg = (p->data.num.as_signed < (mp_intmax_t)0) ? 1 : 0;
if(is_neg) {

signed_num = p->data.num.as_signed + (mp_intmax_t)1;
signed_num = -signed_num;
num = (mp_uintmax_t)signed_num;
num += (mp_uintmax_t)1;
}

goto number;

unsigned_number:

is_neg = 0;

number:



if(prec == -1)
prec = 1;


w = workend;
while(num > 0) {
*w-- = digits[num % base];
num /= base;
}
width -= (long)(workend - w);
prec -= (long)(workend - w);

if(is_alt && base == 8 && prec <= 0) {
*w-- = '0';
--width;
}

if(prec > 0) {
width -= prec;
while(prec-- > 0)
*w-- = '0';
}

if(is_alt && base == 16)
width -= 2;

if(is_neg || (p->flags & FLAGS_SHOWSIGN) || (p->flags & FLAGS_SPACE))
--width;

if(!(p->flags & FLAGS_LEFT) && !(p->flags & FLAGS_PAD_NIL))
while(width-- > 0)
OUTCHAR(' ');

if(is_neg)
OUTCHAR('-');
else if(p->flags & FLAGS_SHOWSIGN)
OUTCHAR('+');
else if(p->flags & FLAGS_SPACE)
OUTCHAR(' ');

if(is_alt && base == 16) {
OUTCHAR('0');
if(p->flags & FLAGS_UPPER)
OUTCHAR('X');
else
OUTCHAR('x');
}

if(!(p->flags & FLAGS_LEFT) && (p->flags & FLAGS_PAD_NIL))
while(width-- > 0)
OUTCHAR('0');


while(++w <= workend) {
OUTCHAR(*w);
}

if(p->flags & FLAGS_LEFT)
while(width-- > 0)
OUTCHAR(' ');
break;

case FORMAT_STRING:

{
static const char null[] = "(nil)";
const char *str;
size_t len;

str = (char *) p->data.str;
if(str == NULL) {

if(prec == -1 || prec >= (long) sizeof(null) - 1) {
str = null;
len = sizeof(null) - 1;

p->flags &= (~FLAGS_ALT);
}
else {
str = "";
len = 0;
}
}
else if(prec != -1)
len = (size_t)prec;
else
len = strlen(str);

width -= (len > LONG_MAX) ? LONG_MAX : (long)len;

if(p->flags & FLAGS_ALT)
OUTCHAR('"');

if(!(p->flags&FLAGS_LEFT))
while(width-- > 0)
OUTCHAR(' ');

for(; len && *str; len--)
OUTCHAR(*str++);
if(p->flags&FLAGS_LEFT)
while(width-- > 0)
OUTCHAR(' ');

if(p->flags & FLAGS_ALT)
OUTCHAR('"');
}
break;

case FORMAT_PTR:

{
void *ptr;
ptr = (void *) p->data.ptr;
if(ptr != NULL) {

base = 16;
digits = (p->flags & FLAGS_UPPER)? upper_digits : lower_digits;
is_alt = 1;
num = (size_t) ptr;
is_neg = 0;
goto number;
}
else {

static const char strnil[] = "(nil)";
const char *point;

width -= (long)(sizeof(strnil) - 1);
if(p->flags & FLAGS_LEFT)
while(width-- > 0)
OUTCHAR(' ');
for(point = strnil; *point != '\0'; ++point)
OUTCHAR(*point);
if(! (p->flags & FLAGS_LEFT))
while(width-- > 0)
OUTCHAR(' ');
}
}
break;

case FORMAT_DOUBLE:
{
char formatbuf[32]="%";
char *fptr = &formatbuf[1];
size_t left = sizeof(formatbuf)-strlen(formatbuf);
int len;

width = -1;
if(p->flags & FLAGS_WIDTH)
width = p->width;
else if(p->flags & FLAGS_WIDTHPARAM)
width = (long)vto[p->width].data.num.as_signed;

prec = -1;
if(p->flags & FLAGS_PREC)
prec = p->precision;
else if(p->flags & FLAGS_PRECPARAM)
prec = (long)vto[p->precision].data.num.as_signed;

if(p->flags & FLAGS_LEFT)
*fptr++ = '-';
if(p->flags & FLAGS_SHOWSIGN)
*fptr++ = '+';
if(p->flags & FLAGS_SPACE)
*fptr++ = ' ';
if(p->flags & FLAGS_ALT)
*fptr++ = '#';

*fptr = 0;

if(width >= 0) {
if(width >= (long)sizeof(work))
width = sizeof(work)-1;

len = curl_msnprintf(fptr, left, "%ld", width);
fptr += len;
left -= len;
}
if(prec >= 0) {


size_t maxprec = sizeof(work) - 2;
double val = p->data.dnum;
while(val >= 10.0) {
val /= 10;
maxprec--;
}

if(prec > (long)maxprec)
prec = (long)maxprec-1;

len = curl_msnprintf(fptr, left, ".%ld", prec);
fptr += len;
}
if(p->flags & FLAGS_LONG)
*fptr++ = 'l';

if(p->flags & FLAGS_FLOATE)
*fptr++ = (char)((p->flags & FLAGS_UPPER) ? 'E':'e');
else if(p->flags & FLAGS_FLOATG)
*fptr++ = (char)((p->flags & FLAGS_UPPER) ? 'G' : 'g');
else
*fptr++ = 'f';

*fptr = 0; 



(sprintf)(work, formatbuf, p->data.dnum);
DEBUGASSERT(strlen(work) <= sizeof(work));
for(fptr = work; *fptr; fptr++)
OUTCHAR(*fptr);
}
break;

case FORMAT_INTPTR:

#if defined(HAVE_LONG_LONG_TYPE)
if(p->flags & FLAGS_LONGLONG)
*(LONG_LONG_TYPE *) p->data.ptr = (LONG_LONG_TYPE)done;
else
#endif
if(p->flags & FLAGS_LONG)
*(long *) p->data.ptr = (long)done;
else if(!(p->flags & FLAGS_SHORT))
*(int *) p->data.ptr = (int)done;
else
*(short *) p->data.ptr = (short)done;
break;

default:
break;
}
f = *end++; 

}
return done;
}


static int addbyter(int output, FILE *data)
{
struct nsprintf *infop = (struct nsprintf *)data;
unsigned char outc = (unsigned char)output;

if(infop->length < infop->max) {

infop->buffer[0] = outc; 
infop->buffer++; 
infop->length++; 
return outc; 
}
return -1;
}

int curl_mvsnprintf(char *buffer, size_t maxlength, const char *format,
va_list ap_save)
{
int retcode;
struct nsprintf info;

info.buffer = buffer;
info.length = 0;
info.max = maxlength;

retcode = dprintf_formatf(&info, addbyter, format, ap_save);
if((retcode != -1) && info.max) {

if(info.max == info.length)

info.buffer[-1] = 0;
else
info.buffer[0] = 0;
}
return retcode;
}

int curl_msnprintf(char *buffer, size_t maxlength, const char *format, ...)
{
int retcode;
va_list ap_save; 
va_start(ap_save, format);
retcode = curl_mvsnprintf(buffer, maxlength, format, ap_save);
va_end(ap_save);
return retcode;
}


static int alloc_addbyter(int output, FILE *data)
{
struct asprintf *infop = (struct asprintf *)data;
unsigned char outc = (unsigned char)output;

if(!infop->buffer) {
infop->buffer = malloc(32);
if(!infop->buffer) {
infop->fail = 1;
return -1; 
}
infop->alloc = 32;
infop->len = 0;
}
else if(infop->len + 1 >= infop->alloc) {
char *newptr = NULL;
size_t newsize = infop->alloc*2;


if(newsize > infop->alloc)
newptr = realloc(infop->buffer, newsize);

if(!newptr) {
infop->fail = 1;
return -1; 
}
infop->buffer = newptr;
infop->alloc = newsize;
}

infop->buffer[ infop->len ] = outc;

infop->len++;

return outc; 
}

char *curl_maprintf(const char *format, ...)
{
va_list ap_save; 
int retcode;
struct asprintf info;

info.buffer = NULL;
info.len = 0;
info.alloc = 0;
info.fail = 0;

va_start(ap_save, format);
retcode = dprintf_formatf(&info, alloc_addbyter, format, ap_save);
va_end(ap_save);
if((-1 == retcode) || info.fail) {
if(info.alloc)
free(info.buffer);
return NULL;
}
if(info.alloc) {
info.buffer[info.len] = 0; 
return info.buffer;
}
return strdup("");
}

char *curl_mvaprintf(const char *format, va_list ap_save)
{
int retcode;
struct asprintf info;

info.buffer = NULL;
info.len = 0;
info.alloc = 0;
info.fail = 0;

retcode = dprintf_formatf(&info, alloc_addbyter, format, ap_save);
if((-1 == retcode) || info.fail) {
if(info.alloc)
free(info.buffer);
return NULL;
}

if(info.alloc) {
info.buffer[info.len] = 0; 
return info.buffer;
}
return strdup("");
}

static int storebuffer(int output, FILE *data)
{
char **buffer = (char **)data;
unsigned char outc = (unsigned char)output;
**buffer = outc;
(*buffer)++;
return outc; 
}

int curl_msprintf(char *buffer, const char *format, ...)
{
va_list ap_save; 
int retcode;
va_start(ap_save, format);
retcode = dprintf_formatf(&buffer, storebuffer, format, ap_save);
va_end(ap_save);
*buffer = 0; 
return retcode;
}

int curl_mprintf(const char *format, ...)
{
int retcode;
va_list ap_save; 
va_start(ap_save, format);

retcode = dprintf_formatf(stdout, fputc, format, ap_save);
va_end(ap_save);
return retcode;
}

int curl_mfprintf(FILE *whereto, const char *format, ...)
{
int retcode;
va_list ap_save; 
va_start(ap_save, format);
retcode = dprintf_formatf(whereto, fputc, format, ap_save);
va_end(ap_save);
return retcode;
}

int curl_mvsprintf(char *buffer, const char *format, va_list ap_save)
{
int retcode;
retcode = dprintf_formatf(&buffer, storebuffer, format, ap_save);
*buffer = 0; 
return retcode;
}

int curl_mvprintf(const char *format, va_list ap_save)
{
return dprintf_formatf(stdout, fputc, format, ap_save);
}

int curl_mvfprintf(FILE *whereto, const char *format, va_list ap_save)
{
return dprintf_formatf(whereto, fputc, format, ap_save);
}
