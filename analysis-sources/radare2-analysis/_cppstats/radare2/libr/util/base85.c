#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <r_types.h>
static int getc_nospace(FILE *f) {
int c;
while (isspace (c = getc (f))) {
;
}
return c;
}
static void putc_wrap(char c, int wrap, int *len) {
if (wrap && *len >= wrap) {
putchar ('\n');
*len = 0;
}
putchar(c);
(*len)++;
}
static void encode_tuple(unsigned long tuple, int count, int wrap, int *plen, int y_abbr) {
int i, lim;
char out[5];
if (tuple == 0 && count == 4) {
putc_wrap('z', wrap, plen);
}
else if (tuple == 0x20202020 && count == 4 && y_abbr) {
putc_wrap('y', wrap, plen);
}
else {
for (i = 0; i < 5; i++) {
out[i] = tuple % 85 + '!';
tuple /= 85;
}
lim = 4 - count;
for (i = 4; i >= lim; i--) {
putc_wrap(out[i], wrap, plen);
}
}
}
R_API void r_base85_decode_tuple(unsigned long tuple, int count) {
int i;
for (i = 1; i < count; i++) {
putchar(tuple >> ((4 - i) * 8));
}
}
R_API void r_base85_encode(FILE *fp, int delims, int wrap, int y_abbr) {
int c, count = 0, len = 0;
unsigned long tuple = 0;
if (delims) {
putc_wrap ('<', wrap, &len);
putc_wrap ('~', wrap, &len);
}
for (;;) {
c = getc(fp);
if (c != EOF) {
tuple |= c << ((3 - count++) * 8);
if (count < 4) {
continue;
}
} else if (count == 0) {
break;
}
encode_tuple(tuple, count, wrap, &len, y_abbr);
if (c == EOF) {
break;
}
tuple = 0;
count = 0;
}
if (delims) {
putc_wrap ('~', wrap, &len);
putc_wrap ('>', wrap, &len);
}
}
R_API bool r_base85_decode(FILE *fp, int delims, int ignore_garbage) {
int c, count = 0, end = 0;
unsigned long tuple = 0, pows[] = {85*85*85*85, 85*85*85, 85*85, 85, 1};
while (delims) {
c = getc_nospace (fp);
if (c == '<') {
c = getc_nospace (fp);
if (c == '~') {
break;
}
ungetc (c, fp);
} else if (c == EOF) {
eprintf ("ascii85: missing <~");
return false;
}
}
for (;;) {
c = getc_nospace(fp);
if (c == 'z' && count == 0) {
r_base85_decode_tuple (0, 5);
continue;
}
if (c == 'y' && count == 0) {
r_base85_decode_tuple (0x20202020, 5);
continue;
}
if (c == '~' && delims) {
c = getc_nospace (fp);
if (c != '>') {
eprintf ("ascii85: ~ without >\n");
return false;
}
c = EOF;
end = 1;
}
if (c == EOF) {
if (delims && !end) {
eprintf ("ascii85: missing ~>");
return false;
}
if (count > 0) {
tuple += pows[count-1];
r_base85_decode_tuple(tuple, count);
}
break;
}
if (c < '!' || c > 'u') {
if (ignore_garbage) {
continue;
}
eprintf ("ascii85: invalid character '%c'\n", c);
return false;
}
tuple += (c - '!') * pows[count++];
if (count == 5) {
r_base85_decode_tuple(tuple, count);
tuple = 0;
count = 0;
}
}
return true;
}
