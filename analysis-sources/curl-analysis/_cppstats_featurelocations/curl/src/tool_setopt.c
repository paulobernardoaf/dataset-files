




















#include "tool_setup.h"

#if !defined(CURL_DISABLE_LIBCURL_OPTION)

#define ENABLE_CURLX_PRINTF

#include "curlx.h"

#include "tool_cfgable.h"
#include "tool_easysrc.h"
#include "tool_setopt.h"
#include "tool_convert.h"

#include "memdebug.h" 






#define NV(e) {#e, e}
#define NV1(e, v) {#e, (v)}
#define NVEND {NULL, 0} 

const NameValue setopt_nv_CURLPROXY[] = {
NV(CURLPROXY_HTTP),
NV(CURLPROXY_HTTP_1_0),
NV(CURLPROXY_HTTPS),
NV(CURLPROXY_SOCKS4),
NV(CURLPROXY_SOCKS5),
NV(CURLPROXY_SOCKS4A),
NV(CURLPROXY_SOCKS5_HOSTNAME),
NVEND,
};

const NameValue setopt_nv_CURL_SOCKS_PROXY[] = {
NV(CURLPROXY_SOCKS4),
NV(CURLPROXY_SOCKS5),
NV(CURLPROXY_SOCKS4A),
NV(CURLPROXY_SOCKS5_HOSTNAME),
NVEND,
};

const NameValueUnsigned setopt_nv_CURLAUTH[] = {
NV(CURLAUTH_ANY), 
NV(CURLAUTH_ANYSAFE), 
NV(CURLAUTH_BASIC),
NV(CURLAUTH_DIGEST),
NV(CURLAUTH_GSSNEGOTIATE),
NV(CURLAUTH_NTLM),
NV(CURLAUTH_DIGEST_IE),
NV(CURLAUTH_NTLM_WB),
NV(CURLAUTH_ONLY),
NV(CURLAUTH_NONE),
NVEND,
};

const NameValue setopt_nv_CURL_HTTP_VERSION[] = {
NV(CURL_HTTP_VERSION_NONE),
NV(CURL_HTTP_VERSION_1_0),
NV(CURL_HTTP_VERSION_1_1),
NV(CURL_HTTP_VERSION_2_0),
NV(CURL_HTTP_VERSION_2TLS),
NV(CURL_HTTP_VERSION_3),
NVEND,
};

const NameValue setopt_nv_CURL_SSLVERSION[] = {
NV(CURL_SSLVERSION_DEFAULT),
NV(CURL_SSLVERSION_TLSv1),
NV(CURL_SSLVERSION_SSLv2),
NV(CURL_SSLVERSION_SSLv3),
NV(CURL_SSLVERSION_TLSv1_0),
NV(CURL_SSLVERSION_TLSv1_1),
NV(CURL_SSLVERSION_TLSv1_2),
NV(CURL_SSLVERSION_TLSv1_3),
NVEND,
};

const NameValue setopt_nv_CURL_TIMECOND[] = {
NV(CURL_TIMECOND_IFMODSINCE),
NV(CURL_TIMECOND_IFUNMODSINCE),
NV(CURL_TIMECOND_LASTMOD),
NV(CURL_TIMECOND_NONE),
NVEND,
};

const NameValue setopt_nv_CURLFTPSSL_CCC[] = {
NV(CURLFTPSSL_CCC_NONE),
NV(CURLFTPSSL_CCC_PASSIVE),
NV(CURLFTPSSL_CCC_ACTIVE),
NVEND,
};

const NameValue setopt_nv_CURLUSESSL[] = {
NV(CURLUSESSL_NONE),
NV(CURLUSESSL_TRY),
NV(CURLUSESSL_CONTROL),
NV(CURLUSESSL_ALL),
NVEND,
};

const NameValueUnsigned setopt_nv_CURLSSLOPT[] = {
NV(CURLSSLOPT_ALLOW_BEAST),
NV(CURLSSLOPT_NO_REVOKE),
NV(CURLSSLOPT_NO_PARTIALCHAIN),
NV(CURLSSLOPT_REVOKE_BEST_EFFORT),
NVEND,
};

const NameValue setopt_nv_CURL_NETRC[] = {
NV(CURL_NETRC_IGNORED),
NV(CURL_NETRC_OPTIONAL),
NV(CURL_NETRC_REQUIRED),
NVEND,
};



const NameValue setopt_nv_CURLPROTO[] = {
NV(CURLPROTO_ALL), 
NV(CURLPROTO_DICT),
NV(CURLPROTO_FILE),
NV(CURLPROTO_FTP),
NV(CURLPROTO_FTPS),
NV(CURLPROTO_GOPHER),
NV(CURLPROTO_HTTP),
NV(CURLPROTO_HTTPS),
NV(CURLPROTO_IMAP),
NV(CURLPROTO_IMAPS),
NV(CURLPROTO_LDAP),
NV(CURLPROTO_LDAPS),
NV(CURLPROTO_POP3),
NV(CURLPROTO_POP3S),
NV(CURLPROTO_RTSP),
NV(CURLPROTO_SCP),
NV(CURLPROTO_SFTP),
NV(CURLPROTO_SMB),
NV(CURLPROTO_SMBS),
NV(CURLPROTO_SMTP),
NV(CURLPROTO_SMTPS),
NV(CURLPROTO_TELNET),
NV(CURLPROTO_TFTP),
NVEND,
};


static const NameValue setopt_nv_CURLNONZERODEFAULTS[] = {
NV1(CURLOPT_SSL_VERIFYPEER, 1),
NV1(CURLOPT_SSL_VERIFYHOST, 1),
NV1(CURLOPT_SSL_ENABLE_NPN, 1),
NV1(CURLOPT_SSL_ENABLE_ALPN, 1),
NV1(CURLOPT_TCP_NODELAY, 1),
NV1(CURLOPT_PROXY_SSL_VERIFYPEER, 1),
NV1(CURLOPT_PROXY_SSL_VERIFYHOST, 1),
NV1(CURLOPT_SOCKS5_AUTH, 1),
NVEND
};


#define ADD(args) do { ret = easysrc_add args; if(ret) goto nomem; } while(0)




#define ADDF(args) do { ret = easysrc_addf args; if(ret) goto nomem; } while(0)




#define NULL_CHECK(p) do { if(!p) { ret = CURLE_OUT_OF_MEMORY; goto nomem; } } while(0)






#define DECL0(s) ADD((&easysrc_decl, s))
#define DECL1(f,a) ADDF((&easysrc_decl, f,a))

#define DATA0(s) ADD((&easysrc_data, s))
#define DATA1(f,a) ADDF((&easysrc_data, f,a))
#define DATA2(f,a,b) ADDF((&easysrc_data, f,a,b))
#define DATA3(f,a,b,c) ADDF((&easysrc_data, f,a,b,c))

#define CODE0(s) ADD((&easysrc_code, s))
#define CODE1(f,a) ADDF((&easysrc_code, f,a))
#define CODE2(f,a,b) ADDF((&easysrc_code, f,a,b))
#define CODE3(f,a,b,c) ADDF((&easysrc_code, f,a,b,c))

#define CLEAN0(s) ADD((&easysrc_clean, s))
#define CLEAN1(f,a) ADDF((&easysrc_clean, f,a))

#define REM0(s) ADD((&easysrc_toohard, s))
#define REM1(f,a) ADDF((&easysrc_toohard, f,a))
#define REM2(f,a,b) ADDF((&easysrc_toohard, f,a,b))



static char *c_escape(const char *str, size_t len)
{
const char *s;
unsigned char c;
char *escaped, *e;

if(len == CURL_ZERO_TERMINATED)
len = strlen(str);


if(len > (~(size_t) 0) / 4)
return NULL;


escaped = malloc(4 * len + 1);
if(!escaped)
return NULL;

e = escaped;
for(s = str; (c = *s) != '\0'; s++) {
if(c == '\n') {
strcpy(e, "\\n");
e += 2;
}
else if(c == '\r') {
strcpy(e, "\\r");
e += 2;
}
else if(c == '\t') {
strcpy(e, "\\t");
e += 2;
}
else if(c == '\\') {
strcpy(e, "\\\\");
e += 2;
}
else if(c == '"') {
strcpy(e, "\\\"");
e += 2;
}
else if(! isprint(c)) {
msnprintf(e, 5, "\\%03o", (unsigned)c);
e += 4;
}
else
*e++ = c;
}
*e = '\0';
return escaped;
}


CURLcode tool_setopt_enum(CURL *curl, struct GlobalConfig *config,
const char *name, CURLoption tag,
const NameValue *nvlist, long lval)
{
CURLcode ret = CURLE_OK;
bool skip = FALSE;

ret = curl_easy_setopt(curl, tag, lval);
if(!lval)
skip = TRUE;

if(config->libcurl && !skip && !ret) {

const NameValue *nv = NULL;
for(nv = nvlist; nv->name; nv++) {
if(nv->value == lval)
break; 
}
if(! nv->name) {



CODE2("curl_easy_setopt(hnd, %s, %ldL);", name, lval);
}
else {
CODE2("curl_easy_setopt(hnd, %s, (long)%s);", name, nv->name);
}
}

nomem:
return ret;
}


CURLcode tool_setopt_flags(CURL *curl, struct GlobalConfig *config,
const char *name, CURLoption tag,
const NameValue *nvlist, long lval)
{
CURLcode ret = CURLE_OK;
bool skip = FALSE;

ret = curl_easy_setopt(curl, tag, lval);
if(!lval)
skip = TRUE;

if(config->libcurl && !skip && !ret) {

char preamble[80]; 
long rest = lval; 
const NameValue *nv = NULL;
msnprintf(preamble, sizeof(preamble),
"curl_easy_setopt(hnd, %s, ", name);
for(nv = nvlist; nv->name; nv++) {
if((nv->value & ~ rest) == 0) {

rest &= ~ nv->value; 
CODE3("%s(long)%s%s",
preamble, nv->name, rest ? " |" : ");");
if(!rest)
break; 

msnprintf(preamble, sizeof(preamble), "%*s", strlen(preamble), "");
}
}



if(rest)
CODE2("%s%ldL);", preamble, rest);
}

nomem:
return ret;
}


CURLcode tool_setopt_bitmask(CURL *curl, struct GlobalConfig *config,
const char *name, CURLoption tag,
const NameValueUnsigned *nvlist,
long lval)
{
CURLcode ret = CURLE_OK;
bool skip = FALSE;

ret = curl_easy_setopt(curl, tag, lval);
if(!lval)
skip = TRUE;

if(config->libcurl && !skip && !ret) {

char preamble[80];
unsigned long rest = (unsigned long)lval;
const NameValueUnsigned *nv = NULL;
msnprintf(preamble, sizeof(preamble),
"curl_easy_setopt(hnd, %s, ", name);
for(nv = nvlist; nv->name; nv++) {
if((nv->value & ~ rest) == 0) {

rest &= ~ nv->value; 
CODE3("%s(long)%s%s",
preamble, nv->name, rest ? " |" : ");");
if(!rest)
break; 

msnprintf(preamble, sizeof(preamble), "%*s", strlen(preamble), "");
}
}



if(rest)
CODE2("%s%luUL);", preamble, rest);
}

nomem:
return ret;
}


static CURLcode libcurl_generate_slist(struct curl_slist *slist, int *slistno)
{
CURLcode ret = CURLE_OK;
char *escaped = NULL;


*slistno = ++easysrc_slist_count;

DECL1("struct curl_slist *slist%d;", *slistno);
DATA1("slist%d = NULL;", *slistno);
CLEAN1("curl_slist_free_all(slist%d);", *slistno);
CLEAN1("slist%d = NULL;", *slistno);
for(; slist; slist = slist->next) {
Curl_safefree(escaped);
escaped = c_escape(slist->data, CURL_ZERO_TERMINATED);
if(!escaped)
return CURLE_OUT_OF_MEMORY;
DATA3("slist%d = curl_slist_append(slist%d, \"%s\");",
*slistno, *slistno, escaped);
}

nomem:
Curl_safefree(escaped);
return ret;
}

static CURLcode libcurl_generate_mime(CURL *curl,
struct GlobalConfig *config,
tool_mime *toolmime,
int *mimeno); 


static CURLcode libcurl_generate_mime_part(CURL *curl,
struct GlobalConfig *config,
tool_mime *part,
int mimeno)
{
CURLcode ret = CURLE_OK;
int submimeno = 0;
char *escaped = NULL;
const char *data = NULL;
const char *filename = part->filename;


if(part->prev) {
ret = libcurl_generate_mime_part(curl, config, part->prev, mimeno);
if(ret)
return ret;
}


CODE2("part%d = curl_mime_addpart(mime%d);", mimeno, mimeno);

switch(part->kind) {
case TOOLMIME_PARTS:
ret = libcurl_generate_mime(curl, config, part, &submimeno);
if(!ret) {
CODE2("curl_mime_subparts(part%d, mime%d);", mimeno, submimeno);
CODE1("mime%d = NULL;", submimeno); 
}
break;

case TOOLMIME_DATA:
#if defined(CURL_DOES_CONVERSIONS)

escaped = c_escape(part->data, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE1("/* \"%s\" */", escaped);


{
size_t size = strlen(part->data);
char *cp = malloc(size + 1);

NULL_CHECK(cp);
memcpy(cp, part->data, size + 1);
ret = convert_to_network(cp, size);
data = cp;
}
#else
data = part->data;
#endif
if(!ret) {
Curl_safefree(escaped);
escaped = c_escape(data, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE2("curl_mime_data(part%d, \"%s\", CURL_ZERO_TERMINATED);",
mimeno, escaped);
}
break;

case TOOLMIME_FILE:
case TOOLMIME_FILEDATA:
escaped = c_escape(part->data, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE2("curl_mime_filedata(part%d, \"%s\");", mimeno, escaped);
if(part->kind == TOOLMIME_FILEDATA && !filename) {
CODE1("curl_mime_filename(part%d, NULL);", mimeno);
}
break;

case TOOLMIME_STDIN:
if(!filename)
filename = "-";

case TOOLMIME_STDINDATA:

CODE1("curl_mime_data_cb(part%d, -1, (curl_read_callback) fread, \\",
mimeno);
CODE0(" (curl_seek_callback) fseek, NULL, stdin);");
break;
default:

break;
}

if(!ret && part->encoder) {
Curl_safefree(escaped);
escaped = c_escape(part->encoder, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE2("curl_mime_encoder(part%d, \"%s\");", mimeno, escaped);
}

if(!ret && filename) {
Curl_safefree(escaped);
escaped = c_escape(filename, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE2("curl_mime_filename(part%d, \"%s\");", mimeno, escaped);
}

if(!ret && part->name) {
Curl_safefree(escaped);
escaped = c_escape(part->name, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE2("curl_mime_name(part%d, \"%s\");", mimeno, escaped);
}

if(!ret && part->type) {
Curl_safefree(escaped);
escaped = c_escape(part->type, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE2("curl_mime_type(part%d, \"%s\");", mimeno, escaped);
}

if(!ret && part->headers) {
int slistno;

ret = libcurl_generate_slist(part->headers, &slistno);
if(!ret) {
CODE2("curl_mime_headers(part%d, slist%d, 1);", mimeno, slistno);
CODE1("slist%d = NULL;", slistno); 
}
}

nomem:
#if defined(CURL_DOES_CONVERSIONS)
if(data)
free((char *) data);
#endif

Curl_safefree(escaped);
return ret;
}


static CURLcode libcurl_generate_mime(CURL *curl,
struct GlobalConfig *config,
tool_mime *toolmime,
int *mimeno)
{
CURLcode ret = CURLE_OK;


*mimeno = ++easysrc_mime_count;
DECL1("curl_mime *mime%d;", *mimeno);
DATA1("mime%d = NULL;", *mimeno);
CODE1("mime%d = curl_mime_init(hnd);", *mimeno);
CLEAN1("curl_mime_free(mime%d);", *mimeno);
CLEAN1("mime%d = NULL;", *mimeno);

if(toolmime->subparts) {
DECL1("curl_mimepart *part%d;", *mimeno);
ret = libcurl_generate_mime_part(curl, config,
toolmime->subparts, *mimeno);
}

nomem:
return ret;
}


CURLcode tool_setopt_mimepost(CURL *curl, struct GlobalConfig *config,
const char *name, CURLoption tag,
curl_mime *mimepost)
{
CURLcode ret = curl_easy_setopt(curl, tag, mimepost);
int mimeno = 0;

if(!ret && config->libcurl) {
ret = libcurl_generate_mime(curl, config,
config->current->mimeroot, &mimeno);

if(!ret)
CODE2("curl_easy_setopt(hnd, %s, mime%d);", name, mimeno);
}

nomem:
return ret;
}


CURLcode tool_setopt_slist(CURL *curl, struct GlobalConfig *config,
const char *name, CURLoption tag,
struct curl_slist *list)
{
CURLcode ret = CURLE_OK;

ret = curl_easy_setopt(curl, tag, list);

if(config->libcurl && list && !ret) {
int i;

ret = libcurl_generate_slist(list, &i);
if(!ret)
CODE2("curl_easy_setopt(hnd, %s, slist%d);", name, i);
}

nomem:
return ret;
}



CURLcode tool_setopt(CURL *curl, bool str, struct GlobalConfig *config,
const char *name, CURLoption tag, ...)
{
va_list arg;
char buf[256];
const char *value = NULL;
bool remark = FALSE;
bool skip = FALSE;
bool escape = FALSE;
char *escaped = NULL;
CURLcode ret = CURLE_OK;

va_start(arg, tag);

if(tag < CURLOPTTYPE_OBJECTPOINT) {

long lval = va_arg(arg, long);
long defval = 0L;
const NameValue *nv = NULL;
for(nv = setopt_nv_CURLNONZERODEFAULTS; nv->name; nv++) {
if(!strcmp(name, nv->name)) {
defval = nv->value;
break; 
}
}

msnprintf(buf, sizeof(buf), "%ldL", lval);
value = buf;
ret = curl_easy_setopt(curl, tag, lval);
if(lval == defval)
skip = TRUE;
}
else if(tag < CURLOPTTYPE_OFF_T) {

void *pval = va_arg(arg, void *);


if(tag >= CURLOPTTYPE_FUNCTIONPOINT) {
if(pval) {
value = "functionpointer";
remark = TRUE;
}
else
skip = TRUE;
}

else if(pval && str) {
value = (char *)pval;
escape = TRUE;
}
else if(pval) {
value = "objectpointer";
remark = TRUE;
}
else
skip = TRUE;

ret = curl_easy_setopt(curl, tag, pval);

}
else {

curl_off_t oval = va_arg(arg, curl_off_t);
msnprintf(buf, sizeof(buf),
"(curl_off_t)%" CURL_FORMAT_CURL_OFF_T, oval);
value = buf;
ret = curl_easy_setopt(curl, tag, oval);

if(!oval)
skip = TRUE;
}

va_end(arg);

if(config->libcurl && !skip && !ret) {


if(remark)
REM2("%s set to a %s", name, value);
else {
if(escape) {
escaped = c_escape(value, CURL_ZERO_TERMINATED);
NULL_CHECK(escaped);
CODE2("curl_easy_setopt(hnd, %s, \"%s\");", name, escaped);
}
else
CODE2("curl_easy_setopt(hnd, %s, %s);", name, value);
}
}

nomem:
Curl_safefree(escaped);
return ret;
}

#else 

#include "tool_cfgable.h"
#include "tool_setopt.h"

#endif 





bool tool_setopt_skip(CURLoption tag)
{
#if defined(CURL_DISABLE_PROXY)
#define USED_TAG
switch(tag) {
case CURLOPT_HAPROXYPROTOCOL:
case CURLOPT_HTTPPROXYTUNNEL:
case CURLOPT_NOPROXY:
case CURLOPT_PRE_PROXY:
case CURLOPT_PROXY:
case CURLOPT_PROXYAUTH:
case CURLOPT_PROXY_CAINFO:
case CURLOPT_PROXY_CAPATH:
case CURLOPT_PROXY_CRLFILE:
case CURLOPT_PROXYHEADER:
case CURLOPT_PROXY_KEYPASSWD:
case CURLOPT_PROXYPASSWORD:
case CURLOPT_PROXY_PINNEDPUBLICKEY:
case CURLOPT_PROXYPORT:
case CURLOPT_PROXY_SERVICE_NAME:
case CURLOPT_PROXY_SSLCERT:
case CURLOPT_PROXY_SSLCERTTYPE:
case CURLOPT_PROXY_SSL_CIPHER_LIST:
case CURLOPT_PROXY_SSLKEY:
case CURLOPT_PROXY_SSLKEYTYPE:
case CURLOPT_PROXY_SSL_OPTIONS:
case CURLOPT_PROXY_SSL_VERIFYHOST:
case CURLOPT_PROXY_SSL_VERIFYPEER:
case CURLOPT_PROXY_SSLVERSION:
case CURLOPT_PROXY_TLS13_CIPHERS:
case CURLOPT_PROXY_TLSAUTH_PASSWORD:
case CURLOPT_PROXY_TLSAUTH_TYPE:
case CURLOPT_PROXY_TLSAUTH_USERNAME:
case CURLOPT_PROXY_TRANSFER_MODE:
case CURLOPT_PROXYTYPE:
case CURLOPT_PROXYUSERNAME:
case CURLOPT_PROXYUSERPWD:
return TRUE;
default:
break;
}
#endif
#if defined(CURL_DISABLE_FTP)
#define USED_TAG
switch(tag) {
case CURLOPT_FTPPORT:
case CURLOPT_FTP_ACCOUNT:
case CURLOPT_FTP_ALTERNATIVE_TO_USER:
case CURLOPT_FTP_FILEMETHOD:
case CURLOPT_FTP_SKIP_PASV_IP:
case CURLOPT_FTP_USE_EPRT:
case CURLOPT_FTP_USE_EPSV:
case CURLOPT_FTP_USE_PRET:
case CURLOPT_KRBLEVEL:
return TRUE;
default:
break;
}
#endif
#if defined(CURL_DISABLE_RTSP)
#define USED_TAG
switch(tag) {
case CURLOPT_INTERLEAVEDATA:
return TRUE;
default:
break;
}
#endif
#if defined(CURL_DISABLE_HTTP) || defined(CURL_DISABLE_COOKIES)
#define USED_TAG
switch(tag) {
case CURLOPT_COOKIE:
case CURLOPT_COOKIEFILE:
case CURLOPT_COOKIEJAR:
case CURLOPT_COOKIESESSION:
return TRUE;
default:
break;
}
#endif
#if defined(CURL_DISABLE_TELNET)
#define USED_TAG
switch(tag) {
case CURLOPT_TELNETOPTIONS:
return TRUE;
default:
break;
}
#endif
#if defined(CURL_DISABLE_TFTP)
#define USED_TAG
switch(tag) {
case CURLOPT_TFTP_BLKSIZE:
case CURLOPT_TFTP_NO_OPTIONS:
return TRUE;
default:
break;
}
#endif
#if defined(CURL_DISABLE_NETRC)
#define USED_TAG
switch(tag) {
case CURLOPT_NETRC:
case CURLOPT_NETRC_FILE:
return TRUE;
default:
break;
}
#endif

#if !defined(USED_TAG)
(void)tag;
#endif
return FALSE;
}
