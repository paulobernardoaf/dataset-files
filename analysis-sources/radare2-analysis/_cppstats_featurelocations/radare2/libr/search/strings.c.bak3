

#include "r_search.h"


enum {
ENCODING_ASCII = 0,
ENCODING_CP850 = 1
};

static char *encodings[3] = { "ascii", "cp850", NULL };



R_API int r_search_get_encoding(const char *name) {
int i;
if (!name || !*name) {
return ENCODING_ASCII;
}
ut32 lename = strlen (name);
for (i = 0; encodings[i]; i++) {
ut32 sz = R_MIN (strlen (encodings[i]), lename);
if (!r_str_ncasecmp (name, encodings[i], sz)) {
return i; 
}
}
return ENCODING_ASCII;
}

static bool is_encoded(int encoding, unsigned char c) {
switch (encoding) {
case ENCODING_ASCII:
break;
case ENCODING_CP850:
switch (c) {

case 128: 
case 133: 
case 135: 
case 160: 
case 161: 
case 129: 
case 130: 
case 139: 
case 162: 
case 163: 
case 164: 
case 165: 
case 181: 
case 144: 
case 214: 
case 224: 
case 233: 
return true;
}
break;
}
return false;
}

R_API int r_search_strings_update(RSearch *s, ut64 from, const ut8 *buf, int len) {
int i = 0;
int widechar = 0;
int matches = 0;
char str[4096];
RListIter *iter;
RSearchKeyword *kw;

r_list_foreach (s->kws, iter, kw) {
for (i = 0; i < len; i++) {
char ch = buf[i];

if (IS_PRINTABLE(ch) || IS_WHITESPACE(ch) || is_encoded (0, ch)) {
str[matches] = ch;
if (matches < sizeof (str)) {
matches++;
}
} else {

if (matches && i + 2 < len && buf[i+2]=='\0' && buf[i]=='\0' && buf[i+1]!='\0') {

return 1; 
}

if (matches >= s->string_min && (s->string_max == 0 || matches <= s->string_max)) {
str[matches] = '\0';
int len = strlen(str);
if (len>2) {
if (widechar) {
ut64 off = (ut64)from+i-(len*2)+1;
r_search_hit_new (s, kw, off);
} else {
ut64 off = (ut64)from+i-matches;
r_search_hit_new (s, kw, off);
}
}
fflush (stdout);
}
matches = 0;
widechar = 0;
}
}
}
return 0;
}
