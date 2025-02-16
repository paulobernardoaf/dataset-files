#include "cache.h"
#include "config.h"
static struct strbuf git_default_name = STRBUF_INIT;
static struct strbuf git_default_email = STRBUF_INIT;
static struct strbuf git_default_date = STRBUF_INIT;
static struct strbuf git_author_name = STRBUF_INIT;
static struct strbuf git_author_email = STRBUF_INIT;
static struct strbuf git_committer_name = STRBUF_INIT;
static struct strbuf git_committer_email = STRBUF_INIT;
static int default_email_is_bogus;
static int default_name_is_bogus;
static int ident_use_config_only;
#define IDENT_NAME_GIVEN 01
#define IDENT_MAIL_GIVEN 02
#define IDENT_ALL_GIVEN (IDENT_NAME_GIVEN|IDENT_MAIL_GIVEN)
static int committer_ident_explicitly_given;
static int author_ident_explicitly_given;
static int ident_config_given;
#if defined(NO_GECOS_IN_PWENT)
#define get_gecos(ignored) "&"
#else
#define get_gecos(struct_passwd) ((struct_passwd)->pw_gecos)
#endif
static struct passwd *xgetpwuid_self(int *is_bogus)
{
struct passwd *pw;
errno = 0;
pw = getpwuid(getuid());
if (!pw) {
static struct passwd fallback;
fallback.pw_name = "unknown";
#if !defined(NO_GECOS_IN_PWENT)
fallback.pw_gecos = "Unknown";
#endif
pw = &fallback;
if (is_bogus)
*is_bogus = 1;
}
return pw;
}
static void copy_gecos(const struct passwd *w, struct strbuf *name)
{
char *src;
for (src = get_gecos(w); *src && *src != ','; src++) {
int ch = *src;
if (ch != '&')
strbuf_addch(name, ch);
else {
strbuf_addch(name, toupper(*w->pw_name));
strbuf_addstr(name, w->pw_name + 1);
}
}
}
static int add_mailname_host(struct strbuf *buf)
{
FILE *mailname;
struct strbuf mailnamebuf = STRBUF_INIT;
mailname = fopen_or_warn("/etc/mailname", "r");
if (!mailname)
return -1;
if (strbuf_getline(&mailnamebuf, mailname) == EOF) {
if (ferror(mailname))
warning_errno("cannot read /etc/mailname");
strbuf_release(&mailnamebuf);
fclose(mailname);
return -1;
}
strbuf_addbuf(buf, &mailnamebuf);
strbuf_release(&mailnamebuf);
fclose(mailname);
return 0;
}
static int canonical_name(const char *host, struct strbuf *out)
{
int status = -1;
#if !defined(NO_IPV6)
struct addrinfo hints, *ai;
memset (&hints, '\0', sizeof (hints));
hints.ai_flags = AI_CANONNAME;
if (!getaddrinfo(host, NULL, &hints, &ai)) {
if (ai && ai->ai_canonname && strchr(ai->ai_canonname, '.')) {
strbuf_addstr(out, ai->ai_canonname);
status = 0;
}
freeaddrinfo(ai);
}
#else
struct hostent *he = gethostbyname(host);
if (he && strchr(he->h_name, '.')) {
strbuf_addstr(out, he->h_name);
status = 0;
}
#endif 
return status;
}
static void add_domainname(struct strbuf *out, int *is_bogus)
{
char buf[HOST_NAME_MAX + 1];
if (xgethostname(buf, sizeof(buf))) {
warning_errno("cannot get host name");
strbuf_addstr(out, "(none)");
*is_bogus = 1;
return;
}
if (strchr(buf, '.'))
strbuf_addstr(out, buf);
else if (canonical_name(buf, out) < 0) {
strbuf_addf(out, "%s.(none)", buf);
*is_bogus = 1;
}
}
static void copy_email(const struct passwd *pw, struct strbuf *email,
int *is_bogus)
{
strbuf_addstr(email, pw->pw_name);
strbuf_addch(email, '@');
if (!add_mailname_host(email))
return; 
add_domainname(email, is_bogus);
}
const char *ident_default_name(void)
{
if (!(ident_config_given & IDENT_NAME_GIVEN) && !git_default_name.len) {
copy_gecos(xgetpwuid_self(&default_name_is_bogus), &git_default_name);
strbuf_trim(&git_default_name);
}
return git_default_name.buf;
}
const char *ident_default_email(void)
{
if (!(ident_config_given & IDENT_MAIL_GIVEN) && !git_default_email.len) {
const char *email = getenv("EMAIL");
if (email && email[0]) {
strbuf_addstr(&git_default_email, email);
committer_ident_explicitly_given |= IDENT_MAIL_GIVEN;
author_ident_explicitly_given |= IDENT_MAIL_GIVEN;
} else if ((email = query_user_email()) && email[0]) {
strbuf_addstr(&git_default_email, email);
free((char *)email);
} else
copy_email(xgetpwuid_self(&default_email_is_bogus),
&git_default_email, &default_email_is_bogus);
strbuf_trim(&git_default_email);
}
return git_default_email.buf;
}
static const char *ident_default_date(void)
{
if (!git_default_date.len)
datestamp(&git_default_date);
return git_default_date.buf;
}
void reset_ident_date(void)
{
strbuf_reset(&git_default_date);
}
static int crud(unsigned char c)
{
return c <= 32 ||
c == '.' ||
c == ',' ||
c == ':' ||
c == ';' ||
c == '<' ||
c == '>' ||
c == '"' ||
c == '\\' ||
c == '\'';
}
static int has_non_crud(const char *str)
{
for (; *str; str++) {
if (!crud(*str))
return 1;
}
return 0;
}
static void strbuf_addstr_without_crud(struct strbuf *sb, const char *src)
{
size_t i, len;
unsigned char c;
while ((c = *src) != 0) {
if (!crud(c))
break;
src++;
}
len = strlen(src);
while (len > 0) {
c = src[len-1];
if (!crud(c))
break;
--len;
}
strbuf_grow(sb, len);
for (i = 0; i < len; i++) {
c = *src++;
switch (c) {
case '\n': case '<': case '>':
continue;
}
sb->buf[sb->len++] = c;
}
sb->buf[sb->len] = '\0';
}
int split_ident_line(struct ident_split *split, const char *line, int len)
{
const char *cp;
size_t span;
int status = -1;
memset(split, 0, sizeof(*split));
split->name_begin = line;
for (cp = line; *cp && cp < line + len; cp++)
if (*cp == '<') {
split->mail_begin = cp + 1;
break;
}
if (!split->mail_begin)
return status;
for (cp = split->mail_begin - 2; line <= cp; cp--)
if (!isspace(*cp)) {
split->name_end = cp + 1;
break;
}
if (!split->name_end) {
split->name_end = split->name_begin;
}
for (cp = split->mail_begin; cp < line + len; cp++)
if (*cp == '>') {
split->mail_end = cp;
break;
}
if (!split->mail_end)
return status;
for (cp = line + len - 1; *cp != '>'; cp--)
;
for (cp = cp + 1; cp < line + len && isspace(*cp); cp++)
;
if (line + len <= cp)
goto person_only;
split->date_begin = cp;
span = strspn(cp, "0123456789");
if (!span)
goto person_only;
split->date_end = split->date_begin + span;
for (cp = split->date_end; cp < line + len && isspace(*cp); cp++)
;
if (line + len <= cp || (*cp != '+' && *cp != '-'))
goto person_only;
split->tz_begin = cp;
span = strspn(cp + 1, "0123456789");
if (!span)
goto person_only;
split->tz_end = split->tz_begin + 1 + span;
return 0;
person_only:
split->date_begin = NULL;
split->date_end = NULL;
split->tz_begin = NULL;
split->tz_end = NULL;
return 0;
}
static const char *env_hint =
N_("\n"
"*** Please tell me who you are.\n"
"\n"
"Run\n"
"\n"
" git config --global user.email \"you@example.com\"\n"
" git config --global user.name \"Your Name\"\n"
"\n"
"to set your account\'s default identity.\n"
"Omit --global to set the identity only in this repository.\n"
"\n");
const char *fmt_ident(const char *name, const char *email,
enum want_ident whose_ident, const char *date_str, int flag)
{
static struct strbuf ident = STRBUF_INIT;
int strict = (flag & IDENT_STRICT);
int want_date = !(flag & IDENT_NO_DATE);
int want_name = !(flag & IDENT_NO_NAME);
if (!email) {
if (whose_ident == WANT_AUTHOR_IDENT && git_author_email.len)
email = git_author_email.buf;
else if (whose_ident == WANT_COMMITTER_IDENT && git_committer_email.len)
email = git_committer_email.buf;
}
if (!email) {
if (strict && ident_use_config_only
&& !(ident_config_given & IDENT_MAIL_GIVEN)) {
fputs(_(env_hint), stderr);
die(_("no email was given and auto-detection is disabled"));
}
email = ident_default_email();
if (strict && default_email_is_bogus) {
fputs(_(env_hint), stderr);
die(_("unable to auto-detect email address (got '%s')"), email);
}
}
if (want_name) {
int using_default = 0;
if (!name) {
if (whose_ident == WANT_AUTHOR_IDENT && git_author_name.len)
name = git_author_name.buf;
else if (whose_ident == WANT_COMMITTER_IDENT &&
git_committer_name.len)
name = git_committer_name.buf;
}
if (!name) {
if (strict && ident_use_config_only
&& !(ident_config_given & IDENT_NAME_GIVEN)) {
fputs(_(env_hint), stderr);
die(_("no name was given and auto-detection is disabled"));
}
name = ident_default_name();
using_default = 1;
if (strict && default_name_is_bogus) {
fputs(_(env_hint), stderr);
die(_("unable to auto-detect name (got '%s')"), name);
}
}
if (!*name) {
struct passwd *pw;
if (strict) {
if (using_default)
fputs(_(env_hint), stderr);
die(_("empty ident name (for <%s>) not allowed"), email);
}
pw = xgetpwuid_self(NULL);
name = pw->pw_name;
}
if (strict && !has_non_crud(name))
die(_("name consists only of disallowed characters: %s"), name);
}
strbuf_reset(&ident);
if (want_name) {
strbuf_addstr_without_crud(&ident, name);
strbuf_addstr(&ident, " <");
}
strbuf_addstr_without_crud(&ident, email);
if (want_name)
strbuf_addch(&ident, '>');
if (want_date) {
strbuf_addch(&ident, ' ');
if (date_str && date_str[0]) {
if (parse_date(date_str, &ident) < 0)
die(_("invalid date format: %s"), date_str);
}
else
strbuf_addstr(&ident, ident_default_date());
}
return ident.buf;
}
const char *fmt_name(enum want_ident whose_ident)
{
char *name = NULL;
char *email = NULL;
switch (whose_ident) {
case WANT_BLANK_IDENT:
break;
case WANT_AUTHOR_IDENT:
name = getenv("GIT_AUTHOR_NAME");
email = getenv("GIT_AUTHOR_EMAIL");
break;
case WANT_COMMITTER_IDENT:
name = getenv("GIT_COMMITTER_NAME");
email = getenv("GIT_COMMITTER_EMAIL");
break;
}
return fmt_ident(name, email, whose_ident, NULL,
IDENT_STRICT | IDENT_NO_DATE);
}
const char *git_author_info(int flag)
{
if (getenv("GIT_AUTHOR_NAME"))
author_ident_explicitly_given |= IDENT_NAME_GIVEN;
if (getenv("GIT_AUTHOR_EMAIL"))
author_ident_explicitly_given |= IDENT_MAIL_GIVEN;
return fmt_ident(getenv("GIT_AUTHOR_NAME"),
getenv("GIT_AUTHOR_EMAIL"),
WANT_AUTHOR_IDENT,
getenv("GIT_AUTHOR_DATE"),
flag);
}
const char *git_committer_info(int flag)
{
if (getenv("GIT_COMMITTER_NAME"))
committer_ident_explicitly_given |= IDENT_NAME_GIVEN;
if (getenv("GIT_COMMITTER_EMAIL"))
committer_ident_explicitly_given |= IDENT_MAIL_GIVEN;
return fmt_ident(getenv("GIT_COMMITTER_NAME"),
getenv("GIT_COMMITTER_EMAIL"),
WANT_COMMITTER_IDENT,
getenv("GIT_COMMITTER_DATE"),
flag);
}
static int ident_is_sufficient(int user_ident_explicitly_given)
{
#if !defined(WINDOWS)
return (user_ident_explicitly_given & IDENT_MAIL_GIVEN);
#else
return (user_ident_explicitly_given == IDENT_ALL_GIVEN);
#endif
}
int committer_ident_sufficiently_given(void)
{
return ident_is_sufficient(committer_ident_explicitly_given);
}
int author_ident_sufficiently_given(void)
{
return ident_is_sufficient(author_ident_explicitly_given);
}
static int set_ident(const char *var, const char *value)
{
if (!strcmp(var, "author.name")) {
if (!value)
return config_error_nonbool(var);
strbuf_reset(&git_author_name);
strbuf_addstr(&git_author_name, value);
author_ident_explicitly_given |= IDENT_NAME_GIVEN;
ident_config_given |= IDENT_NAME_GIVEN;
return 0;
}
if (!strcmp(var, "author.email")) {
if (!value)
return config_error_nonbool(var);
strbuf_reset(&git_author_email);
strbuf_addstr(&git_author_email, value);
author_ident_explicitly_given |= IDENT_MAIL_GIVEN;
ident_config_given |= IDENT_MAIL_GIVEN;
return 0;
}
if (!strcmp(var, "committer.name")) {
if (!value)
return config_error_nonbool(var);
strbuf_reset(&git_committer_name);
strbuf_addstr(&git_committer_name, value);
committer_ident_explicitly_given |= IDENT_NAME_GIVEN;
ident_config_given |= IDENT_NAME_GIVEN;
return 0;
}
if (!strcmp(var, "committer.email")) {
if (!value)
return config_error_nonbool(var);
strbuf_reset(&git_committer_email);
strbuf_addstr(&git_committer_email, value);
committer_ident_explicitly_given |= IDENT_MAIL_GIVEN;
ident_config_given |= IDENT_MAIL_GIVEN;
return 0;
}
if (!strcmp(var, "user.name")) {
if (!value)
return config_error_nonbool(var);
strbuf_reset(&git_default_name);
strbuf_addstr(&git_default_name, value);
committer_ident_explicitly_given |= IDENT_NAME_GIVEN;
author_ident_explicitly_given |= IDENT_NAME_GIVEN;
ident_config_given |= IDENT_NAME_GIVEN;
return 0;
}
if (!strcmp(var, "user.email")) {
if (!value)
return config_error_nonbool(var);
strbuf_reset(&git_default_email);
strbuf_addstr(&git_default_email, value);
committer_ident_explicitly_given |= IDENT_MAIL_GIVEN;
author_ident_explicitly_given |= IDENT_MAIL_GIVEN;
ident_config_given |= IDENT_MAIL_GIVEN;
return 0;
}
return 0;
}
int git_ident_config(const char *var, const char *value, void *data)
{
if (!strcmp(var, "user.useconfigonly")) {
ident_use_config_only = git_config_bool(var, value);
return 0;
}
return set_ident(var, value);
}
static void set_env_if(const char *key, const char *value, int *given, int bit)
{
if ((*given & bit) || getenv(key))
return; 
setenv(key, value, 0);
*given |= bit;
}
void prepare_fallback_ident(const char *name, const char *email)
{
set_env_if("GIT_AUTHOR_NAME", name,
&author_ident_explicitly_given, IDENT_NAME_GIVEN);
set_env_if("GIT_AUTHOR_EMAIL", email,
&author_ident_explicitly_given, IDENT_MAIL_GIVEN);
set_env_if("GIT_COMMITTER_NAME", name,
&committer_ident_explicitly_given, IDENT_NAME_GIVEN);
set_env_if("GIT_COMMITTER_EMAIL", email,
&committer_ident_explicitly_given, IDENT_MAIL_GIVEN);
}
static int buf_cmp(const char *a_begin, const char *a_end,
const char *b_begin, const char *b_end)
{
int a_len = a_end - a_begin;
int b_len = b_end - b_begin;
int min = a_len < b_len ? a_len : b_len;
int cmp;
cmp = memcmp(a_begin, b_begin, min);
if (cmp)
return cmp;
return a_len - b_len;
}
int ident_cmp(const struct ident_split *a,
const struct ident_split *b)
{
int cmp;
cmp = buf_cmp(a->mail_begin, a->mail_end,
b->mail_begin, b->mail_end);
if (cmp)
return cmp;
return buf_cmp(a->name_begin, a->name_end,
b->name_begin, b->name_end);
}
