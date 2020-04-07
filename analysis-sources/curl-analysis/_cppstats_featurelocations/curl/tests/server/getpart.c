




















#include "server_setup.h"

#include "getpart.h"

#define ENABLE_CURLX_PRINTF


#include "curlx.h" 


struct Curl_easy {
int fake;
};

#include "curl_base64.h"
#include "curl_memory.h"


#include "memdebug.h"

#define EAT_SPACE(p) while(*(p) && ISSPACE(*(p))) (p)++

#define EAT_WORD(p) while(*(p) && !ISSPACE(*(p)) && ('>' != *(p))) (p)++

#if defined(DEBUG_GETPART)
#define show(x) printf x
#else
#define show(x) Curl_nop_stmt
#endif

#if defined(_MSC_VER) && defined(_DLL)
#pragma warning(disable:4232) 
#endif

curl_malloc_callback Curl_cmalloc = (curl_malloc_callback)malloc;
curl_free_callback Curl_cfree = (curl_free_callback)free;
curl_realloc_callback Curl_crealloc = (curl_realloc_callback)realloc;
curl_strdup_callback Curl_cstrdup = (curl_strdup_callback)strdup;
curl_calloc_callback Curl_ccalloc = (curl_calloc_callback)calloc;
#if defined(WIN32) && defined(UNICODE)
curl_wcsdup_callback Curl_cwcsdup = (curl_wcsdup_callback)_wcsdup;
#endif

#if defined(_MSC_VER) && defined(_DLL)
#pragma warning(default:4232) 
#endif









CURLcode Curl_convert_clone(struct Curl_easy *data,
const char *indata,
size_t insize,
char **outbuf);
CURLcode Curl_convert_clone(struct Curl_easy *data,
const char *indata,
size_t insize,
char **outbuf)
{
char *convbuf;
(void)data;

convbuf = malloc(insize);
if(!convbuf)
return CURLE_OUT_OF_MEMORY;

memcpy(convbuf, indata, insize);
*outbuf = convbuf;
return CURLE_OK;
}



















static int readline(char **buffer, size_t *bufsize, FILE *stream)
{
size_t offset = 0;
char *newptr;

if(!*buffer) {
*buffer = malloc(128);
if(!*buffer)
return GPE_OUT_OF_MEMORY;
*bufsize = 128;
}

for(;;) {
size_t length;
int bytestoread = curlx_uztosi(*bufsize - offset);

if(!fgets(*buffer + offset, bytestoread, stream))
return (offset != 0) ? GPE_OK : GPE_END_OF_FILE;

length = offset + strlen(*buffer + offset);
if(*(*buffer + length - 1) == '\n')
break;
offset = length;
if(length < *bufsize - 1)
continue;

newptr = realloc(*buffer, *bufsize * 2);
if(!newptr)
return GPE_OUT_OF_MEMORY;
*buffer = newptr;
*bufsize *= 2;
}

return GPE_OK;
}



























static int appenddata(char **dst_buf, 
size_t *dst_len, 
size_t *dst_alloc, 
char *src_buf, 
int src_b64) 
{
size_t need_alloc = 0;
size_t src_len = strlen(src_buf);

if(!src_len)
return GPE_OK;

need_alloc = src_len + *dst_len + 1;

if(src_b64) {
if(src_buf[src_len - 1] == '\r')
src_len--;

if(src_buf[src_len - 1] == '\n')
src_len--;
}


if(need_alloc > *dst_alloc) {
size_t newsize = need_alloc * 2;
char *newptr = realloc(*dst_buf, newsize);
if(!newptr) {
return GPE_OUT_OF_MEMORY;
}
*dst_alloc = newsize;
*dst_buf = newptr;
}


memcpy(*dst_buf + *dst_len, src_buf, src_len);
*dst_len += src_len;
*(*dst_buf + *dst_len) = '\0';

return GPE_OK;
}

static int decodedata(char **buf, 
size_t *len) 
{
CURLcode error = CURLE_OK;
unsigned char *buf64 = NULL;
size_t src_len = 0;

if(!*len)
return GPE_OK;


error = Curl_base64_decode(*buf, &buf64, &src_len);
if(error)
return GPE_OUT_OF_MEMORY;

if(!src_len) {






free(buf64);

return GPE_OUT_OF_MEMORY;
}


memcpy(*buf, buf64, src_len);
*len = src_len;
*(*buf + src_len) = '\0';

free(buf64);

return GPE_OK;
}























int getpart(char **outbuf, size_t *outlen,
const char *main, const char *sub, FILE *stream)
{
#define MAX_TAG_LEN 79
char couter[MAX_TAG_LEN + 1]; 
char cmain[MAX_TAG_LEN + 1]; 
char csub[MAX_TAG_LEN + 1]; 
char ptag[MAX_TAG_LEN + 1]; 
char patt[MAX_TAG_LEN + 1]; 
char *buffer = NULL;
char *ptr;
char *end;
union {
ssize_t sig;
size_t uns;
} len;
size_t bufsize = 0;
size_t outalloc = 256;
int in_wanted_part = 0;
int base64 = 0;
int error;

enum {
STATE_OUTSIDE = 0,
STATE_OUTER = 1,
STATE_INMAIN = 2,
STATE_INSUB = 3,
STATE_ILLEGAL = 4
} state = STATE_OUTSIDE;

*outlen = 0;
*outbuf = malloc(outalloc);
if(!*outbuf)
return GPE_OUT_OF_MEMORY;
*(*outbuf) = '\0';

couter[0] = cmain[0] = csub[0] = ptag[0] = patt[0] = '\0';

while((error = readline(&buffer, &bufsize, stream)) == GPE_OK) {

ptr = buffer;
EAT_SPACE(ptr);

if('<' != *ptr) {
if(in_wanted_part) {
show(("=> %s", buffer));
error = appenddata(outbuf, outlen, &outalloc, buffer, base64);
if(error)
break;
}
continue;
}

ptr++;

if('/' == *ptr) {




ptr++;
end = ptr;
EAT_WORD(end);
len.sig = end - ptr;
if(len.sig > MAX_TAG_LEN) {
error = GPE_NO_BUFFER_SPACE;
break;
}
memcpy(ptag, ptr, len.uns);
ptag[len.uns] = '\0';

if((STATE_INSUB == state) && !strcmp(csub, ptag)) {

state = STATE_INMAIN;
csub[0] = '\0';
if(in_wanted_part) {

in_wanted_part = 0;


if(base64) {
error = decodedata(outbuf, outlen);
if(error)
return error;
}
break;
}
}
else if((STATE_INMAIN == state) && !strcmp(cmain, ptag)) {

state = STATE_OUTER;
cmain[0] = '\0';
if(in_wanted_part) {

in_wanted_part = 0;


if(base64) {
error = decodedata(outbuf, outlen);
if(error)
return error;
}
break;
}
}
else if((STATE_OUTER == state) && !strcmp(couter, ptag)) {

state = STATE_OUTSIDE;
couter[0] = '\0';
if(in_wanted_part) {

in_wanted_part = 0;
break;
}
}

}
else if(!in_wanted_part) {





end = ptr;
EAT_WORD(end);
len.sig = end - ptr;
if(len.sig > MAX_TAG_LEN) {
error = GPE_NO_BUFFER_SPACE;
break;
}
memcpy(ptag, ptr, len.uns);
ptag[len.uns] = '\0';


if(('!' == ptag[0]) || ('?' == ptag[0])) {
show(("* ignoring (%s)", buffer));
continue;
}


ptr = end;
EAT_SPACE(ptr);
end = ptr;
while(*end && ('>' != *end))
end++;
len.sig = end - ptr;
if(len.sig > MAX_TAG_LEN) {
error = GPE_NO_BUFFER_SPACE;
break;
}
memcpy(patt, ptr, len.uns);
patt[len.uns] = '\0';

if(STATE_OUTSIDE == state) {

strcpy(couter, ptag);
state = STATE_OUTER;
continue;
}
else if(STATE_OUTER == state) {

strcpy(cmain, ptag);
state = STATE_INMAIN;
continue;
}
else if(STATE_INMAIN == state) {

strcpy(csub, ptag);
state = STATE_INSUB;
if(!strcmp(cmain, main) && !strcmp(csub, sub)) {

in_wanted_part = 1;
if(strstr(patt, "base64="))


base64 = 1;
}
continue;
}

}

if(in_wanted_part) {
show(("=> %s", buffer));
error = appenddata(outbuf, outlen, &outalloc, buffer, base64);
if(error)
break;
}

} 

free(buffer);

if(error != GPE_OK) {
if(error == GPE_END_OF_FILE)
error = GPE_OK;
else {
free(*outbuf);
*outbuf = NULL;
*outlen = 0;
}
}

return error;
}
