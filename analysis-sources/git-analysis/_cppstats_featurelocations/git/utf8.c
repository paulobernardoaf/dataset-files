#include "git-compat-util.h"
#include "strbuf.h"
#include "utf8.h"



static const char utf16_be_bom[] = {'\xFE', '\xFF'};
static const char utf16_le_bom[] = {'\xFF', '\xFE'};
static const char utf32_be_bom[] = {'\0', '\0', '\xFE', '\xFF'};
static const char utf32_le_bom[] = {'\xFF', '\xFE', '\0', '\0'};

struct interval {
ucs_char_t first;
ucs_char_t last;
};

size_t display_mode_esc_sequence_len(const char *s)
{
const char *p = s;
if (*p++ != '\033')
return 0;
if (*p++ != '[')
return 0;
while (isdigit(*p) || *p == ';')
p++;
if (*p++ != 'm')
return 0;
return p - s;
}


static int bisearch(ucs_char_t ucs, const struct interval *table, int max)
{
int min = 0;
int mid;

if (ucs < table[0].first || ucs > table[max].last)
return 0;
while (max >= min) {
mid = min + (max - min) / 2;
if (ucs > table[mid].last)
min = mid + 1;
else if (ucs < table[mid].first)
max = mid - 1;
else
return 1;
}

return 0;
}

































static int git_wcwidth(ucs_char_t ch)
{



#include "unicode-width.h"


if (ch == 0)
return 0;
if (ch < 32 || (ch >= 0x7f && ch < 0xa0))
return -1;


if (bisearch(ch, zero_width, ARRAY_SIZE(zero_width) - 1))
return 0;


if (bisearch(ch, double_width, ARRAY_SIZE(double_width) - 1))
return 2;

return 1;
}













static ucs_char_t pick_one_utf8_char(const char **start, size_t *remainder_p)
{
unsigned char *s = (unsigned char *)*start;
ucs_char_t ch;
size_t remainder, incr;






remainder = (remainder_p ? *remainder_p : 999);

if (remainder < 1) {
goto invalid;
} else if (*s < 0x80) {

ch = *s;
incr = 1;
} else if ((s[0] & 0xe0) == 0xc0) {

if (remainder < 2 ||
(s[1] & 0xc0) != 0x80 ||
(s[0] & 0xfe) == 0xc0)
goto invalid;
ch = ((s[0] & 0x1f) << 6) | (s[1] & 0x3f);
incr = 2;
} else if ((s[0] & 0xf0) == 0xe0) {

if (remainder < 3 ||
(s[1] & 0xc0) != 0x80 ||
(s[2] & 0xc0) != 0x80 ||

(s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) ||

(s[0] == 0xed && (s[1] & 0xe0) == 0xa0) ||

(s[0] == 0xef && s[1] == 0xbf &&
(s[2] & 0xfe) == 0xbe))
goto invalid;
ch = ((s[0] & 0x0f) << 12) |
((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
incr = 3;
} else if ((s[0] & 0xf8) == 0xf0) {

if (remainder < 4 ||
(s[1] & 0xc0) != 0x80 ||
(s[2] & 0xc0) != 0x80 ||
(s[3] & 0xc0) != 0x80 ||

(s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) ||

(s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4)
goto invalid;
ch = ((s[0] & 0x07) << 18) | ((s[1] & 0x3f) << 12) |
((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
incr = 4;
} else {
invalid:
*start = NULL;
return 0;
}

*start += incr;
if (remainder_p)
*remainder_p = remainder - incr;
return ch;
}








int utf8_width(const char **start, size_t *remainder_p)
{
ucs_char_t ch = pick_one_utf8_char(start, remainder_p);
if (!*start)
return 0;
return git_wcwidth(ch);
}






int utf8_strnwidth(const char *string, int len, int skip_ansi)
{
int width = 0;
const char *orig = string;

if (len == -1)
len = strlen(string);
while (string && string < orig + len) {
int skip;
while (skip_ansi &&
(skip = display_mode_esc_sequence_len(string)) != 0)
string += skip;
width += utf8_width(&string, NULL);
}
return string ? width : len;
}

int utf8_strwidth(const char *string)
{
return utf8_strnwidth(string, -1, 0);
}

int is_utf8(const char *text)
{
while (*text) {
if (*text == '\n' || *text == '\t' || *text == '\r') {
text++;
continue;
}
utf8_width(&text, NULL);
if (!text)
return 0;
}
return 1;
}

static void strbuf_add_indented_text(struct strbuf *buf, const char *text,
int indent, int indent2)
{
if (indent < 0)
indent = 0;
while (*text) {
const char *eol = strchrnul(text, '\n');
if (*eol == '\n')
eol++;
strbuf_addchars(buf, ' ', indent);
strbuf_add(buf, text, eol - text);
text = eol;
indent = indent2;
}
}







void strbuf_add_wrapped_text(struct strbuf *buf,
const char *text, int indent1, int indent2, int width)
{
int indent, w, assume_utf8 = 1;
const char *bol, *space, *start = text;
size_t orig_len = buf->len;

if (width <= 0) {
strbuf_add_indented_text(buf, text, indent1, indent2);
return;
}

retry:
bol = text;
w = indent = indent1;
space = NULL;
if (indent < 0) {
w = -indent;
space = text;
}

for (;;) {
char c;
size_t skip;

while ((skip = display_mode_esc_sequence_len(text)))
text += skip;

c = *text;
if (!c || isspace(c)) {
if (w <= width || !space) {
const char *start = bol;
if (!c && text == start)
return;
if (space)
start = space;
else
strbuf_addchars(buf, ' ', indent);
strbuf_add(buf, start, text - start);
if (!c)
return;
space = text;
if (c == '\t')
w |= 0x07;
else if (c == '\n') {
space++;
if (*space == '\n') {
strbuf_addch(buf, '\n');
goto new_line;
}
else if (!isalnum(*space))
goto new_line;
else
strbuf_addch(buf, ' ');
}
w++;
text++;
}
else {
new_line:
strbuf_addch(buf, '\n');
text = bol = space + isspace(*space);
space = NULL;
w = indent = indent2;
}
continue;
}
if (assume_utf8) {
w += utf8_width(&text, NULL);
if (!text) {
assume_utf8 = 0;
text = start;
strbuf_setlen(buf, orig_len);
goto retry;
}
} else {
w++;
text++;
}
}
}

void strbuf_add_wrapped_bytes(struct strbuf *buf, const char *data, int len,
int indent, int indent2, int width)
{
char *tmp = xstrndup(data, len);
strbuf_add_wrapped_text(buf, tmp, indent, indent2, width);
free(tmp);
}

void strbuf_utf8_replace(struct strbuf *sb_src, int pos, int width,
const char *subst)
{
struct strbuf sb_dst = STRBUF_INIT;
char *src = sb_src->buf;
char *end = src + sb_src->len;
char *dst;
int w = 0, subst_len = 0;

if (subst)
subst_len = strlen(subst);
strbuf_grow(&sb_dst, sb_src->len + subst_len);
dst = sb_dst.buf;

while (src < end) {
char *old;
size_t n;

while ((n = display_mode_esc_sequence_len(src))) {
memcpy(dst, src, n);
src += n;
dst += n;
}

if (src >= end)
break;

old = src;
n = utf8_width((const char**)&src, NULL);
if (!src) 
goto out;
if (n && w >= pos && w < pos + width) {
if (subst) {
memcpy(dst, subst, subst_len);
dst += subst_len;
subst = NULL;
}
w += n;
continue;
}
memcpy(dst, old, src - old);
dst += src - old;
w += n;
}
strbuf_setlen(&sb_dst, dst - sb_dst.buf);
strbuf_swap(sb_src, &sb_dst);
out:
strbuf_release(&sb_dst);
}






static int same_utf_encoding(const char *src, const char *dst)
{
if (skip_iprefix(src, "utf", &src) && skip_iprefix(dst, "utf", &dst)) {
skip_prefix(src, "-", &src);
skip_prefix(dst, "-", &dst);
return !strcasecmp(src, dst);
}
return 0;
}

int is_encoding_utf8(const char *name)
{
if (!name)
return 1;
if (same_utf_encoding("utf-8", name))
return 1;
return 0;
}

int same_encoding(const char *src, const char *dst)
{
static const char utf8[] = "UTF-8";

if (!src)
src = utf8;
if (!dst)
dst = utf8;
if (same_utf_encoding(src, dst))
return 1;
return !strcasecmp(src, dst);
}





int utf8_fprintf(FILE *stream, const char *format, ...)
{
struct strbuf buf = STRBUF_INIT;
va_list arg;
int columns;

va_start(arg, format);
strbuf_vaddf(&buf, format, arg);
va_end(arg);

columns = fputs(buf.buf, stream);
if (0 <= columns) 
columns = utf8_strwidth(buf.buf);
strbuf_release(&buf);
return columns;
}





#if !defined(NO_ICONV)
#if defined(OLD_ICONV) || (defined(__sun__) && !defined(_XPG6))
typedef const char * iconv_ibp;
#else
typedef char * iconv_ibp;
#endif
char *reencode_string_iconv(const char *in, size_t insz, iconv_t conv,
size_t bom_len, size_t *outsz_p)
{
size_t outsz, outalloc;
char *out, *outpos;
iconv_ibp cp;

outsz = insz;
outalloc = st_add(outsz, 1 + bom_len); 
out = xmalloc(outalloc);
outpos = out + bom_len;
cp = (iconv_ibp)in;

while (1) {
size_t cnt = iconv(conv, &cp, &insz, &outpos, &outsz);

if (cnt == (size_t) -1) {
size_t sofar;
if (errno != E2BIG) {
free(out);
return NULL;
}





sofar = outpos - out;
outalloc = st_add3(sofar, st_mult(insz, 2), 32);
out = xrealloc(out, outalloc);
outpos = out + sofar;
outsz = outalloc - sofar - 1;
}
else {
*outpos = '\0';
if (outsz_p)
*outsz_p = outpos - out;
break;
}
}
return out;
}

static const char *fallback_encoding(const char *name)
{







if (is_encoding_utf8(name))
return "UTF-8";





if (!strcasecmp(name, "latin-1"))
return "ISO-8859-1";

return name;
}

char *reencode_string_len(const char *in, size_t insz,
const char *out_encoding, const char *in_encoding,
size_t *outsz)
{
iconv_t conv;
char *out;
const char *bom_str = NULL;
size_t bom_len = 0;

if (!in_encoding)
return NULL;


if (same_utf_encoding("UTF-16LE-BOM", in_encoding))
in_encoding = "UTF-16";









if (same_utf_encoding("UTF-16LE-BOM", out_encoding)) {
bom_str = utf16_le_bom;
bom_len = sizeof(utf16_le_bom);
out_encoding = "UTF-16LE";
} else if (same_utf_encoding("UTF-16BE-BOM", out_encoding)) {
bom_str = utf16_be_bom;
bom_len = sizeof(utf16_be_bom);
out_encoding = "UTF-16BE";
#if defined(ICONV_OMITS_BOM)
} else if (same_utf_encoding("UTF-16", out_encoding)) {
bom_str = utf16_be_bom;
bom_len = sizeof(utf16_be_bom);
out_encoding = "UTF-16BE";
} else if (same_utf_encoding("UTF-32", out_encoding)) {
bom_str = utf32_be_bom;
bom_len = sizeof(utf32_be_bom);
out_encoding = "UTF-32BE";
#endif
}

conv = iconv_open(out_encoding, in_encoding);
if (conv == (iconv_t) -1) {
in_encoding = fallback_encoding(in_encoding);
out_encoding = fallback_encoding(out_encoding);

conv = iconv_open(out_encoding, in_encoding);
if (conv == (iconv_t) -1)
return NULL;
}
out = reencode_string_iconv(in, insz, conv, bom_len, outsz);
iconv_close(conv);
if (out && bom_str && bom_len)
memcpy(out, bom_str, bom_len);
return out;
}
#endif

static int has_bom_prefix(const char *data, size_t len,
const char *bom, size_t bom_len)
{
return data && bom && (len >= bom_len) && !memcmp(data, bom, bom_len);
}

int has_prohibited_utf_bom(const char *enc, const char *data, size_t len)
{
return (
(same_utf_encoding("UTF-16BE", enc) ||
same_utf_encoding("UTF-16LE", enc)) &&
(has_bom_prefix(data, len, utf16_be_bom, sizeof(utf16_be_bom)) ||
has_bom_prefix(data, len, utf16_le_bom, sizeof(utf16_le_bom)))
) || (
(same_utf_encoding("UTF-32BE", enc) ||
same_utf_encoding("UTF-32LE", enc)) &&
(has_bom_prefix(data, len, utf32_be_bom, sizeof(utf32_be_bom)) ||
has_bom_prefix(data, len, utf32_le_bom, sizeof(utf32_le_bom)))
);
}

int is_missing_required_utf_bom(const char *enc, const char *data, size_t len)
{
return (
(same_utf_encoding(enc, "UTF-16")) &&
!(has_bom_prefix(data, len, utf16_be_bom, sizeof(utf16_be_bom)) ||
has_bom_prefix(data, len, utf16_le_bom, sizeof(utf16_le_bom)))
) || (
(same_utf_encoding(enc, "UTF-32")) &&
!(has_bom_prefix(data, len, utf32_be_bom, sizeof(utf32_be_bom)) ||
has_bom_prefix(data, len, utf32_le_bom, sizeof(utf32_le_bom)))
);
}










int mbs_chrlen(const char **text, size_t *remainder_p, const char *encoding)
{
int chrlen;
const char *p = *text;
size_t r = (remainder_p ? *remainder_p : SIZE_MAX);

if (r < 1)
return 0;

if (is_encoding_utf8(encoding)) {
pick_one_utf8_char(&p, &r);

chrlen = p ? (p - *text)
: 1 ;
}
else {




chrlen = 1;
}

*text += chrlen;
if (remainder_p)
*remainder_p -= chrlen;

return chrlen;
}






static ucs_char_t next_hfs_char(const char **in)
{
while (1) {
ucs_char_t out = pick_one_utf8_char(in, NULL);






if (!*in)
return 0;


switch (out) {
case 0x200c: 
case 0x200d: 
case 0x200e: 
case 0x200f: 
case 0x202a: 
case 0x202b: 
case 0x202c: 
case 0x202d: 
case 0x202e: 
case 0x206a: 
case 0x206b: 
case 0x206c: 
case 0x206d: 
case 0x206e: 
case 0x206f: 
case 0xfeff: 
continue;
}

return out;
}
}

static int is_hfs_dot_generic(const char *path,
const char *needle, size_t needle_len)
{
ucs_char_t c;

c = next_hfs_char(&path);
if (c != '.')
return 0;






for (; needle_len > 0; needle++, needle_len--) {
c = next_hfs_char(&path);





if (c > 127)
return 0;
if (tolower(c) != *needle)
return 0;
}

c = next_hfs_char(&path);
if (c && !is_dir_sep(c))
return 0;

return 1;
}





static inline int is_hfs_dot_str(const char *path, const char *needle)
{
return is_hfs_dot_generic(path, needle, strlen(needle));
}

int is_hfs_dotgit(const char *path)
{
return is_hfs_dot_str(path, "git");
}

int is_hfs_dotgitmodules(const char *path)
{
return is_hfs_dot_str(path, "gitmodules");
}

int is_hfs_dotgitignore(const char *path)
{
return is_hfs_dot_str(path, "gitignore");
}

int is_hfs_dotgitattributes(const char *path)
{
return is_hfs_dot_str(path, "gitattributes");
}

const char utf8_bom[] = "\357\273\277";

int skip_utf8_bom(char **text, size_t len)
{
if (len < strlen(utf8_bom) ||
memcmp(*text, utf8_bom, strlen(utf8_bom)))
return 0;
*text += strlen(utf8_bom);
return 1;
}

void strbuf_utf8_align(struct strbuf *buf, align_type position, unsigned int width,
const char *s)
{
int slen = strlen(s);
int display_len = utf8_strnwidth(s, slen, 0);
int utf8_compensation = slen - display_len;

if (display_len >= width) {
strbuf_addstr(buf, s);
return;
}

if (position == ALIGN_LEFT)
strbuf_addf(buf, "%-*s", width + utf8_compensation, s);
else if (position == ALIGN_MIDDLE) {
int left = (width - display_len) / 2;
strbuf_addf(buf, "%*s%-*s", left, "", width - left + utf8_compensation, s);
} else if (position == ALIGN_RIGHT)
strbuf_addf(buf, "%*s", width + utf8_compensation, s);
}
