
























#include "curl_setup.h"

#include <curl/curl.h>

#include "urldata.h"
#include "warnless.h"
#include "non-ascii.h"
#include "escape.h"
#include "strdup.h"

#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"





bool Curl_isunreserved(unsigned char in)
{
switch(in) {
case '0': case '1': case '2': case '3': case '4':
case '5': case '6': case '7': case '8': case '9':
case 'a': case 'b': case 'c': case 'd': case 'e':
case 'f': case 'g': case 'h': case 'i': case 'j':
case 'k': case 'l': case 'm': case 'n': case 'o':
case 'p': case 'q': case 'r': case 's': case 't':
case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
case 'A': case 'B': case 'C': case 'D': case 'E':
case 'F': case 'G': case 'H': case 'I': case 'J':
case 'K': case 'L': case 'M': case 'N': case 'O':
case 'P': case 'Q': case 'R': case 'S': case 'T':
case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
case '-': case '.': case '_': case '~':
return TRUE;
default:
break;
}
return FALSE;
}


char *curl_escape(const char *string, int inlength)
{
return curl_easy_escape(NULL, string, inlength);
}


char *curl_unescape(const char *string, int length)
{
return curl_easy_unescape(NULL, string, length, NULL);
}

char *curl_easy_escape(struct Curl_easy *data, const char *string,
int inlength)
{
size_t alloc;
char *ns;
char *testing_ptr = NULL;
size_t newlen;
size_t strindex = 0;
size_t length;
CURLcode result;

if(inlength < 0)
return NULL;

alloc = (inlength?(size_t)inlength:strlen(string)) + 1;
newlen = alloc;

ns = malloc(alloc);
if(!ns)
return NULL;

length = alloc-1;
while(length--) {
unsigned char in = *string; 

if(Curl_isunreserved(in))

ns[strindex++] = in;
else {

newlen += 2; 
if(newlen > alloc) {
alloc *= 2;
testing_ptr = Curl_saferealloc(ns, alloc);
if(!testing_ptr)
return NULL;
ns = testing_ptr;
}

result = Curl_convert_to_network(data, (char *)&in, 1);
if(result) {

free(ns);
return NULL;
}

msnprintf(&ns[strindex], 4, "%%%02X", in);

strindex += 3;
}
string++;
}
ns[strindex] = 0; 
return ns;
}













CURLcode Curl_urldecode(struct Curl_easy *data,
const char *string, size_t length,
char **ostring, size_t *olen,
bool reject_ctrl)
{
size_t alloc = (length?length:strlen(string)) + 1;
char *ns = malloc(alloc);
size_t strindex = 0;
unsigned long hex;
CURLcode result = CURLE_OK;

if(!ns)
return CURLE_OUT_OF_MEMORY;

while(--alloc > 0) {
unsigned char in = *string;
if(('%' == in) && (alloc > 2) &&
ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {

char hexstr[3];
char *ptr;
hexstr[0] = string[1];
hexstr[1] = string[2];
hexstr[2] = 0;

hex = strtoul(hexstr, &ptr, 16);

in = curlx_ultouc(hex); 

if(data) {
result = Curl_convert_from_network(data, (char *)&in, 1);
if(result) {

free(ns);
return result;
}
}

string += 2;
alloc -= 2;
}

if(reject_ctrl && (in < 0x20)) {
free(ns);
return CURLE_URL_MALFORMAT;
}

ns[strindex++] = in;
string++;
}
ns[strindex] = 0; 

if(olen)

*olen = strindex;


*ostring = ns;

return CURLE_OK;
}







char *curl_easy_unescape(struct Curl_easy *data, const char *string,
int length, int *olen)
{
char *str = NULL;
if(length >= 0) {
size_t inputlen = length;
size_t outputlen;
CURLcode res = Curl_urldecode(data, string, inputlen, &str, &outputlen,
FALSE);
if(res)
return NULL;

if(olen) {
if(outputlen <= (size_t) INT_MAX)
*olen = curlx_uztosi(outputlen);
else

Curl_safefree(str);
}
}
return str;
}




void curl_free(void *p)
{
free(p);
}
