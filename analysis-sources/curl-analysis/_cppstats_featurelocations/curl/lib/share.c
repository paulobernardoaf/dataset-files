





















#include "curl_setup.h"

#include <curl/curl.h>
#include "urldata.h"
#include "share.h"
#include "psl.h"
#include "vtls/vtls.h"
#include "curl_memory.h"


#include "memdebug.h"

struct Curl_share *
curl_share_init(void)
{
struct Curl_share *share = calloc(1, sizeof(struct Curl_share));
if(share) {
share->specifier |= (1<<CURL_LOCK_DATA_SHARE);

if(Curl_mk_dnscache(&share->hostcache)) {
free(share);
return NULL;
}
}

return share;
}

#undef curl_share_setopt
CURLSHcode
curl_share_setopt(struct Curl_share *share, CURLSHoption option, ...)
{
va_list param;
int type;
curl_lock_function lockfunc;
curl_unlock_function unlockfunc;
void *ptr;
CURLSHcode res = CURLSHE_OK;

if(share->dirty)


return CURLSHE_IN_USE;

va_start(param, option);

switch(option) {
case CURLSHOPT_SHARE:

type = va_arg(param, int);
share->specifier |= (1<<type);
switch(type) {
case CURL_LOCK_DATA_DNS:
break;

case CURL_LOCK_DATA_COOKIE:
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
if(!share->cookies) {
share->cookies = Curl_cookie_init(NULL, NULL, NULL, TRUE);
if(!share->cookies)
res = CURLSHE_NOMEM;
}
#else 
res = CURLSHE_NOT_BUILT_IN;
#endif
break;

case CURL_LOCK_DATA_SSL_SESSION:
#if defined(USE_SSL)
if(!share->sslsession) {
share->max_ssl_sessions = 8;
share->sslsession = calloc(share->max_ssl_sessions,
sizeof(struct curl_ssl_session));
share->sessionage = 0;
if(!share->sslsession)
res = CURLSHE_NOMEM;
}
#else
res = CURLSHE_NOT_BUILT_IN;
#endif
break;

case CURL_LOCK_DATA_CONNECT: 
if(Curl_conncache_init(&share->conn_cache, 103))
res = CURLSHE_NOMEM;
break;

case CURL_LOCK_DATA_PSL:
#if !defined(USE_LIBPSL)
res = CURLSHE_NOT_BUILT_IN;
#endif
break;

default:
res = CURLSHE_BAD_OPTION;
}
break;

case CURLSHOPT_UNSHARE:

type = va_arg(param, int);
share->specifier &= ~(1<<type);
switch(type) {
case CURL_LOCK_DATA_DNS:
break;

case CURL_LOCK_DATA_COOKIE:
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
if(share->cookies) {
Curl_cookie_cleanup(share->cookies);
share->cookies = NULL;
}
#else 
res = CURLSHE_NOT_BUILT_IN;
#endif
break;

case CURL_LOCK_DATA_SSL_SESSION:
#if defined(USE_SSL)
Curl_safefree(share->sslsession);
#else
res = CURLSHE_NOT_BUILT_IN;
#endif
break;

case CURL_LOCK_DATA_CONNECT:
break;

default:
res = CURLSHE_BAD_OPTION;
break;
}
break;

case CURLSHOPT_LOCKFUNC:
lockfunc = va_arg(param, curl_lock_function);
share->lockfunc = lockfunc;
break;

case CURLSHOPT_UNLOCKFUNC:
unlockfunc = va_arg(param, curl_unlock_function);
share->unlockfunc = unlockfunc;
break;

case CURLSHOPT_USERDATA:
ptr = va_arg(param, void *);
share->clientdata = ptr;
break;

default:
res = CURLSHE_BAD_OPTION;
break;
}

va_end(param);

return res;
}

CURLSHcode
curl_share_cleanup(struct Curl_share *share)
{
if(share == NULL)
return CURLSHE_INVALID;

if(share->lockfunc)
share->lockfunc(NULL, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE,
share->clientdata);

if(share->dirty) {
if(share->unlockfunc)
share->unlockfunc(NULL, CURL_LOCK_DATA_SHARE, share->clientdata);
return CURLSHE_IN_USE;
}

Curl_conncache_close_all_connections(&share->conn_cache);
Curl_conncache_destroy(&share->conn_cache);
Curl_hash_destroy(&share->hostcache);

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
Curl_cookie_cleanup(share->cookies);
#endif

#if defined(USE_SSL)
if(share->sslsession) {
size_t i;
for(i = 0; i < share->max_ssl_sessions; i++)
Curl_ssl_kill_session(&(share->sslsession[i]));
free(share->sslsession);
}
#endif

Curl_psl_destroy(&share->psl);

if(share->unlockfunc)
share->unlockfunc(NULL, CURL_LOCK_DATA_SHARE, share->clientdata);
free(share);

return CURLSHE_OK;
}


CURLSHcode
Curl_share_lock(struct Curl_easy *data, curl_lock_data type,
curl_lock_access accesstype)
{
struct Curl_share *share = data->share;

if(share == NULL)
return CURLSHE_INVALID;

if(share->specifier & (1<<type)) {
if(share->lockfunc) 
share->lockfunc(data, type, accesstype, share->clientdata);
}


return CURLSHE_OK;
}

CURLSHcode
Curl_share_unlock(struct Curl_easy *data, curl_lock_data type)
{
struct Curl_share *share = data->share;

if(share == NULL)
return CURLSHE_INVALID;

if(share->specifier & (1<<type)) {
if(share->unlockfunc) 
share->unlockfunc (data, type, share->clientdata);
}

return CURLSHE_OK;
}
