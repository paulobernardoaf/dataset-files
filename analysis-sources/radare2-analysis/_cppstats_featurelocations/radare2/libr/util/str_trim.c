

#include "r_types.h"
#include "r_util.h"
#include "r_cons.h"
#include "r_bin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>


R_API void r_str_trim_path(char *s) {
char *src, *dst, *p;
int i = 0;
if (!s || !*s) {
return;
}
dst = src = s + 1;
while (*src) {
if (*(src - 1) == '/' && *src == '.' && *(src + 1) == '.') {
if (*(src + 2) == '/' || *(src + 2) == '\0') {
p = dst - 1;
while (s != p) {
if (*p == '/') {
if (i) {
dst = p + 1;
i = 0;
break;
}
i = 1;
}
p--;
}
if (s == p && *p == '/') {
dst = p + 1;
}
src = src + 2;
} else {
*dst = *src;
dst++;
}
} else if (*src == '/' && *(src + 1) == '.' && (*(src + 2) == '/' || *(src + 2) == '\0')) {
src++;
} else if (*src != '/' || *(src - 1) != '/') {
*dst = *src;
dst++;
}
src++;
}
if (dst > s + 1 && *(dst - 1) == '/') {
*(dst - 1) = 0;
} else {
*dst = 0;
}
}

R_API char* r_str_trim_lines(char *str) {
RList *list = r_str_split_list (str, "\n", 0);
char *s;
RListIter *iter;
RStrBuf *sb = r_strbuf_new ("");
r_list_foreach (list, iter, s) {

r_str_ansi_filter (s, NULL, NULL, -1);
r_str_trim (s);
if (*s) {
r_strbuf_appendf (sb, "%s\n", s);
}
}
r_list_free (list);
free (str);
return r_strbuf_drain (sb);
}

R_API char *r_str_trim_dup(const char *str) {
char *a = strdup (str);
r_str_trim (a);
return a;
}



R_API const char *r_str_trim_head_ro(const char *str) {
r_return_val_if_fail (str, NULL);
for (; *str && IS_WHITECHAR (*str); str++) {
;
}
return str;
}


R_API const char *r_str_trim_head_wp(const char *str) {
r_return_val_if_fail (str, NULL);
for (; *str && !IS_WHITESPACE (*str); str++) {
;
}
return str;
}



R_API void r_str_trim_head(char *str) {
char *p = (char *)r_str_trim_head_ro (str);
if (p) {
memmove (str, p, strlen (p) + 1);
}
}



R_API void r_str_trim_tail(char *str) {
r_return_if_fail (str);
size_t length = strlen (str);
while (length-- > 0) {
if (IS_WHITECHAR (str[length])) {
str[length] = '\0';
} else {
break;
}
}
}



R_API void r_str_trim(char *str) {
r_str_trim_head (str);
r_str_trim_tail (str);
}



R_API char *r_str_trim_nc(char *str) {
char *s = (char *)r_str_trim_head_ro (str);
r_str_trim_tail (s);
return s;
}


R_API int r_str_ansi_trim(char *str, int str_len, int n) {
r_return_val_if_fail (str, 0);
char ch, ch2;
int back = 0, i = 0, len = 0;

if (str_len < 0) {
str_len = strlen (str);
}
if (n >= str_len) {
str[str_len] = 0;
return str_len;
}
while ((i < str_len) && str[i] && len < n && n > 0) {
ch = str[i];
ch2 = str[i + 1];
if (ch == 0x1b) {
if (ch2 == '\\') {
i++;
} else if (ch2 == ']') {
if (!strncmp (str + 2 + 5, "rgb:", 4)) {
i += 18;
}
} else if (ch2 == '[') {
for (++i; (i < str_len) && str[i] && str[i] != 'J' && str[i] != 'm' && str[i] != 'H';
i++) {
;
}
}
} else if ((str[i] & 0xc0) != 0x80) {
len++;
}
i++;
back = i; 
}
str[back] = 0;
return back;
}
