#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>
#include "nvim/types.h"
#include "nvim/mbyte.h"
#include "nvim/ascii.h"
const uint8_t utf8len_tab_zero[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0, 
};
const uint8_t utf8len_tab[] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1, 
};
int utf_ptr2char(const char_u *const p)
{
if (p[0] < 0x80) { 
return p[0];
}
const uint8_t len = utf8len_tab_zero[p[0]];
if (len > 1 && (p[1] & 0xc0) == 0x80) {
if (len == 2) {
return ((p[0] & 0x1f) << 6) + (p[1] & 0x3f);
}
if ((p[2] & 0xc0) == 0x80) {
if (len == 3) {
return (((p[0] & 0x0f) << 12) + ((p[1] & 0x3f) << 6)
+ (p[2] & 0x3f));
}
if ((p[3] & 0xc0) == 0x80) {
if (len == 4) {
return (((p[0] & 0x07) << 18) + ((p[1] & 0x3f) << 12)
+ ((p[2] & 0x3f) << 6) + (p[3] & 0x3f));
}
if ((p[4] & 0xc0) == 0x80) {
if (len == 5) {
return (((p[0] & 0x03) << 24) + ((p[1] & 0x3f) << 18)
+ ((p[2] & 0x3f) << 12) + ((p[3] & 0x3f) << 6)
+ (p[4] & 0x3f));
}
if ((p[5] & 0xc0) == 0x80 && len == 6) {
return (((p[0] & 0x01) << 30) + ((p[1] & 0x3f) << 24)
+ ((p[2] & 0x3f) << 18) + ((p[3] & 0x3f) << 12)
+ ((p[4] & 0x3f) << 6) + (p[5] & 0x3f));
}
}
}
}
}
return p[0];
}
bool utf_composinglike(const char_u *p1, const char_u *p2)
{
return false;
}
char_u *string_convert(const vimconv_T *conv, char_u *data, size_t *size)
{
return NULL;
}
int utf_ptr2len_len(const char_u *p, int size)
{
int len;
int i;
int m;
len = utf8len_tab[*p];
if (len == 1)
return 1; 
if (len > size)
m = size; 
else
m = len;
for (i = 1; i < m; ++i)
if ((p[i] & 0xc0) != 0x80)
return 1;
return len;
}
int utfc_ptr2len_len(const char_u *p, int size)
{
int len;
int prevlen;
if (size < 1 || *p == NUL)
return 0;
if (p[0] < 0x80 && (size == 1 || p[1] < 0x80)) 
return 1;
len = utf_ptr2len_len(p, size);
if ((len == 1 && p[0] >= 0x80) || len > size)
return 1;
prevlen = 0;
while (len < size) {
int len_next_char;
if (p[len] < 0x80)
break;
len_next_char = utf_ptr2len_len(p + len, size - len);
if (len_next_char > size - len)
break;
if (!UTF_COMPOSINGLIKE(p + prevlen, p + len))
break;
prevlen = len;
len += len_next_char;
}
return len;
}
int utf_char2len(const int c)
{
if (c < 0x80) {
return 1;
} else if (c < 0x800) {
return 2;
} else if (c < 0x10000) {
return 3;
} else if (c < 0x200000) {
return 4;
} else if (c < 0x4000000) {
return 5;
} else {
return 6;
}
}
int utf_char2bytes(const int c, char_u *const buf)
{
if (c < 0x80) { 
buf[0] = c;
return 1;
} else if (c < 0x800) { 
buf[0] = 0xc0 + ((unsigned)c >> 6);
buf[1] = 0x80 + (c & 0x3f);
return 2;
} else if (c < 0x10000) { 
buf[0] = 0xe0 + ((unsigned)c >> 12);
buf[1] = 0x80 + (((unsigned)c >> 6) & 0x3f);
buf[2] = 0x80 + (c & 0x3f);
return 3;
} else if (c < 0x200000) { 
buf[0] = 0xf0 + ((unsigned)c >> 18);
buf[1] = 0x80 + (((unsigned)c >> 12) & 0x3f);
buf[2] = 0x80 + (((unsigned)c >> 6) & 0x3f);
buf[3] = 0x80 + (c & 0x3f);
return 4;
} else if (c < 0x4000000) { 
buf[0] = 0xf8 + ((unsigned)c >> 24);
buf[1] = 0x80 + (((unsigned)c >> 18) & 0x3f);
buf[2] = 0x80 + (((unsigned)c >> 12) & 0x3f);
buf[3] = 0x80 + (((unsigned)c >> 6) & 0x3f);
buf[4] = 0x80 + (c & 0x3f);
return 5;
} else { 
buf[0] = 0xfc + ((unsigned)c >> 30);
buf[1] = 0x80 + (((unsigned)c >> 24) & 0x3f);
buf[2] = 0x80 + (((unsigned)c >> 18) & 0x3f);
buf[3] = 0x80 + (((unsigned)c >> 12) & 0x3f);
buf[4] = 0x80 + (((unsigned)c >> 6) & 0x3f);
buf[5] = 0x80 + (c & 0x3f);
return 6;
}
}
int utf_ptr2len(const char_u *const p)
{
if (*p == NUL) {
return 0;
}
const int len = utf8len_tab[*p];
for (int i = 1; i < len; i++) {
if ((p[i] & 0xc0) != 0x80) {
return 1;
}
}
return len;
}
int utfc_ptr2len(const char_u *const p)
{
uint8_t b0 = (uint8_t)(*p);
if (b0 == NUL) {
return 0;
}
if (b0 < 0x80 && p[1] < 0x80) { 
return 1;
}
int len = utf_ptr2len(p);
if (len == 1 && b0 >= 0x80) {
return 1;
}
int prevlen = 0;
for (;;) {
if (p[len] < 0x80 || !UTF_COMPOSINGLIKE(p + prevlen, p + len)) {
return len;
}
prevlen = len;
len += utf_ptr2len(p + len);
}
}
void mb_copy_char(const char_u **fp, char_u **tp)
{
const size_t l = utfc_ptr2len(*fp);
memmove(*tp, *fp, (size_t)l);
*tp += l;
*fp += l;
}
