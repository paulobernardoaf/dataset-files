#include "cache.h"
#include "urlmatch.h"
#define URL_ALPHA "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define URL_DIGIT "0123456789"
#define URL_ALPHADIGIT URL_ALPHA URL_DIGIT
#define URL_SCHEME_CHARS URL_ALPHADIGIT "+.-"
#define URL_HOST_CHARS URL_ALPHADIGIT ".-[:]" 
#define URL_UNSAFE_CHARS " <>\"%{}|\\^`" 
#define URL_GEN_RESERVED ":/?#[]@"
#define URL_SUB_RESERVED "!$&'()*+,;="
#define URL_RESERVED URL_GEN_RESERVED URL_SUB_RESERVED 
static int append_normalized_escapes(struct strbuf *buf,
const char *from,
size_t from_len,
const char *esc_extra,
const char *esc_ok)
{
while (from_len) {
int ch = *from++;
int was_esc = 0;
from_len--;
if (ch == '%') {
if (from_len < 2)
return 0;
ch = hex2chr(from);
if (ch < 0)
return 0;
from += 2;
from_len -= 2;
was_esc = 1;
}
if ((unsigned char)ch <= 0x1F || (unsigned char)ch >= 0x7F ||
strchr(URL_UNSAFE_CHARS, ch) ||
(esc_extra && strchr(esc_extra, ch)) ||
(was_esc && strchr(esc_ok, ch)))
strbuf_addf(buf, "%%%02X", (unsigned char)ch);
else
strbuf_addch(buf, ch);
}
return 1;
}
static const char *end_of_token(const char *s, int c, size_t n)
{
const char *next = memchr(s, c, n);
if (!next)
next = s + n;
return next;
}
static int match_host(const struct url_info *url_info,
const struct url_info *pattern_info)
{
const char *url = url_info->url + url_info->host_off;
const char *pat = pattern_info->url + pattern_info->host_off;
int url_len = url_info->host_len;
int pat_len = pattern_info->host_len;
while (url_len && pat_len) {
const char *url_next = end_of_token(url, '.', url_len);
const char *pat_next = end_of_token(pat, '.', pat_len);
if (pat_next == pat + 1 && pat[0] == '*')
;
else if ((pat_next - pat) == (url_next - url) &&
!memcmp(url, pat, url_next - url))
;
else
return 0; 
if (url_next < url + url_len)
url_next++;
url_len -= url_next - url;
url = url_next;
if (pat_next < pat + pat_len)
pat_next++;
pat_len -= pat_next - pat;
pat = pat_next;
}
return (!url_len && !pat_len);
}
static char *url_normalize_1(const char *url, struct url_info *out_info, char allow_globs)
{
size_t url_len = strlen(url);
struct strbuf norm;
size_t spanned;
size_t scheme_len, user_off=0, user_len=0, passwd_off=0, passwd_len=0;
size_t host_off=0, host_len=0, port_off=0, port_len=0, path_off, path_len, result_len;
const char *slash_ptr, *at_ptr, *colon_ptr, *path_start;
char *result;
spanned = strspn(url, URL_SCHEME_CHARS);
if (!spanned || !isalpha(url[0]) || spanned + 3 > url_len ||
url[spanned] != ':' || url[spanned+1] != '/' || url[spanned+2] != '/') {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid URL scheme name or missing '://' suffix");
}
return NULL; 
}
strbuf_init(&norm, url_len);
scheme_len = spanned;
spanned += 3;
url_len -= spanned;
while (spanned--)
strbuf_addch(&norm, tolower(*url++));
at_ptr = strchr(url, '@');
slash_ptr = url + strcspn(url, "/?#");
if (at_ptr && at_ptr < slash_ptr) {
user_off = norm.len;
if (at_ptr > url) {
if (!append_normalized_escapes(&norm, url, at_ptr - url,
"", URL_RESERVED)) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid %XX escape sequence");
}
strbuf_release(&norm);
return NULL;
}
colon_ptr = strchr(norm.buf + scheme_len + 3, ':');
if (colon_ptr) {
passwd_off = (colon_ptr + 1) - norm.buf;
passwd_len = norm.len - passwd_off;
user_len = (passwd_off - 1) - (scheme_len + 3);
} else {
user_len = norm.len - (scheme_len + 3);
}
}
strbuf_addch(&norm, '@');
url_len -= (++at_ptr - url);
url = at_ptr;
}
if (!url_len || strchr(":/?#", *url)) {
if (strncmp(norm.buf, "file:", 5)) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("missing host and scheme is not 'file:'");
}
strbuf_release(&norm);
return NULL;
}
} else {
host_off = norm.len;
}
colon_ptr = slash_ptr - 1;
while (colon_ptr > url && *colon_ptr != ':' && *colon_ptr != ']')
colon_ptr--;
if (*colon_ptr != ':') {
colon_ptr = slash_ptr;
} else if (!host_off && colon_ptr < slash_ptr && colon_ptr + 1 != slash_ptr) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("a 'file:' URL may not have a port number");
}
strbuf_release(&norm);
return NULL;
}
if (allow_globs)
spanned = strspn(url, URL_HOST_CHARS "*");
else
spanned = strspn(url, URL_HOST_CHARS);
if (spanned < colon_ptr - url) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid characters in host name");
}
strbuf_release(&norm);
return NULL;
}
while (url < colon_ptr) {
strbuf_addch(&norm, tolower(*url++));
url_len--;
}
if (colon_ptr < slash_ptr) {
url++;
url += strspn(url, "0");
if (url == slash_ptr && url[-1] == '0')
url--;
if (url == slash_ptr) {
} else if (slash_ptr - url == 2 &&
!strncmp(norm.buf, "http:", 5) &&
!strncmp(url, "80", 2)) {
} else if (slash_ptr - url == 3 &&
!strncmp(norm.buf, "https:", 6) &&
!strncmp(url, "443", 3)) {
} else {
unsigned long pnum = 0;
spanned = strspn(url, URL_DIGIT);
if (spanned < slash_ptr - url) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid port number");
}
strbuf_release(&norm);
return NULL;
}
if (slash_ptr - url <= 5)
pnum = strtoul(url, NULL, 10);
if (pnum == 0 || pnum > 65535) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid port number");
}
strbuf_release(&norm);
return NULL;
}
strbuf_addch(&norm, ':');
port_off = norm.len;
strbuf_add(&norm, url, slash_ptr - url);
port_len = slash_ptr - url;
}
url_len -= slash_ptr - colon_ptr;
url = slash_ptr;
}
if (host_off)
host_len = norm.len - host_off - (port_len ? port_len + 1 : 0);
path_off = norm.len;
path_start = norm.buf + path_off;
strbuf_addch(&norm, '/');
if (*url == '/') {
url++;
url_len--;
}
for (;;) {
const char *seg_start;
size_t seg_start_off = norm.len;
const char *next_slash = url + strcspn(url, "/?#");
int skip_add_slash = 0;
if (!append_normalized_escapes(&norm, url, next_slash - url, "",
URL_RESERVED)) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid %XX escape sequence");
}
strbuf_release(&norm);
return NULL;
}
seg_start = norm.buf + seg_start_off;
if (!strcmp(seg_start, ".")) {
if (seg_start == path_start + 1) {
strbuf_setlen(&norm, norm.len - 1);
skip_add_slash = 1;
} else {
strbuf_setlen(&norm, norm.len - 2);
}
} else if (!strcmp(seg_start, "..")) {
const char *prev_slash = norm.buf + norm.len - 3;
if (prev_slash == path_start) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid '..' path segment");
}
strbuf_release(&norm);
return NULL;
}
while (*--prev_slash != '/') {}
if (prev_slash == path_start) {
strbuf_setlen(&norm, prev_slash - norm.buf + 1);
skip_add_slash = 1;
} else {
strbuf_setlen(&norm, prev_slash - norm.buf);
}
}
url_len -= next_slash - url;
url = next_slash;
if (*url != '/')
break;
url++;
url_len--;
if (!skip_add_slash)
strbuf_addch(&norm, '/');
}
path_len = norm.len - path_off;
if (*url) {
if (!append_normalized_escapes(&norm, url, url_len, "", URL_RESERVED)) {
if (out_info) {
out_info->url = NULL;
out_info->err = _("invalid %XX escape sequence");
}
strbuf_release(&norm);
return NULL;
}
}
result = strbuf_detach(&norm, &result_len);
if (out_info) {
out_info->url = result;
out_info->err = NULL;
out_info->url_len = result_len;
out_info->scheme_len = scheme_len;
out_info->user_off = user_off;
out_info->user_len = user_len;
out_info->passwd_off = passwd_off;
out_info->passwd_len = passwd_len;
out_info->host_off = host_off;
out_info->host_len = host_len;
out_info->port_off = port_off;
out_info->port_len = port_len;
out_info->path_off = path_off;
out_info->path_len = path_len;
}
return result;
}
char *url_normalize(const char *url, struct url_info *out_info)
{
return url_normalize_1(url, out_info, 0);
}
static size_t url_match_prefix(const char *url,
const char *url_prefix,
size_t url_prefix_len)
{
if (!url || !url_prefix)
return 0;
if (!url_prefix_len || (url_prefix_len == 1 && *url_prefix == '/'))
return (!*url || *url == '/') ? 1 : 0;
if (url_prefix[url_prefix_len - 1] == '/')
url_prefix_len--;
if (strncmp(url, url_prefix, url_prefix_len))
return 0;
if ((strlen(url) == url_prefix_len) || (url[url_prefix_len] == '/'))
return url_prefix_len + 1;
return 0;
}
static int match_urls(const struct url_info *url,
const struct url_info *url_prefix,
struct urlmatch_item *match)
{
char usermatched = 0;
size_t pathmatchlen;
if (!url || !url_prefix || !url->url || !url_prefix->url)
return 0;
if (url_prefix->scheme_len != url->scheme_len ||
strncmp(url->url, url_prefix->url, url->scheme_len))
return 0; 
if (url_prefix->user_off) {
if (!url->user_off || url->user_len != url_prefix->user_len ||
strncmp(url->url + url->user_off,
url_prefix->url + url_prefix->user_off,
url->user_len))
return 0; 
usermatched = 1;
}
if (!match_host(url, url_prefix))
return 0; 
if (url_prefix->port_len != url->port_len ||
strncmp(url->url + url->port_off,
url_prefix->url + url_prefix->port_off, url->port_len))
return 0; 
pathmatchlen = url_match_prefix(
url->url + url->path_off,
url_prefix->url + url_prefix->path_off,
url_prefix->url_len - url_prefix->path_off);
if (!pathmatchlen)
return 0; 
if (match) {
match->hostmatch_len = url_prefix->host_len;
match->pathmatch_len = pathmatchlen;
match->user_matched = usermatched;
}
return 1;
}
static int cmp_matches(const struct urlmatch_item *a,
const struct urlmatch_item *b)
{
if (a->hostmatch_len != b->hostmatch_len)
return a->hostmatch_len < b->hostmatch_len ? -1 : 1;
if (a->pathmatch_len != b->pathmatch_len)
return a->pathmatch_len < b->pathmatch_len ? -1 : 1;
if (a->user_matched != b->user_matched)
return b->user_matched ? -1 : 1;
return 0;
}
int urlmatch_config_entry(const char *var, const char *value, void *cb)
{
struct string_list_item *item;
struct urlmatch_config *collect = cb;
struct urlmatch_item matched = {0};
struct url_info *url = &collect->url;
const char *key, *dot;
struct strbuf synthkey = STRBUF_INIT;
int retval;
int (*select_fn)(const struct urlmatch_item *a, const struct urlmatch_item *b) =
collect->select_fn ? collect->select_fn : cmp_matches;
if (!skip_prefix(var, collect->section, &key) || *(key++) != '.') {
if (collect->cascade_fn)
return collect->cascade_fn(var, value, cb);
return 0; 
}
dot = strrchr(key, '.');
if (dot) {
char *config_url, *norm_url;
struct url_info norm_info;
config_url = xmemdupz(key, dot - key);
norm_url = url_normalize_1(config_url, &norm_info, 1);
free(config_url);
if (!norm_url)
return 0;
retval = match_urls(url, &norm_info, &matched);
free(norm_url);
if (!retval)
return 0;
key = dot + 1;
}
if (collect->key && strcmp(key, collect->key))
return 0;
item = string_list_insert(&collect->vars, key);
if (!item->util) {
item->util = xcalloc(1, sizeof(matched));
} else {
if (select_fn(&matched, item->util) < 0)
return 0;
}
memcpy(item->util, &matched, sizeof(matched));
strbuf_addstr(&synthkey, collect->section);
strbuf_addch(&synthkey, '.');
strbuf_addstr(&synthkey, key);
retval = collect->collect_fn(synthkey.buf, value, collect->cb);
strbuf_release(&synthkey);
return retval;
}
