





















#include "curl_setup.h"

#if defined(USE_SSH)

#include <curl/curl.h>
#include "curl_memory.h"
#include "curl_path.h"
#include "escape.h"
#include "memdebug.h"


CURLcode Curl_getworkingpath(struct connectdata *conn,
char *homedir, 
char **path) 

{
struct Curl_easy *data = conn->data;
char *real_path = NULL;
char *working_path;
size_t working_path_len;
CURLcode result =
Curl_urldecode(data, data->state.up.path, 0, &working_path,
&working_path_len, FALSE);
if(result)
return result;


if(conn->handler->protocol & CURLPROTO_SCP) {
real_path = malloc(working_path_len + 1);
if(real_path == NULL) {
free(working_path);
return CURLE_OUT_OF_MEMORY;
}
if((working_path_len > 3) && (!memcmp(working_path, "/~/", 3)))

memcpy(real_path, working_path + 3, working_path_len - 2);
else
memcpy(real_path, working_path, 1 + working_path_len);
}
else if(conn->handler->protocol & CURLPROTO_SFTP) {
if((working_path_len > 1) && (working_path[1] == '~')) {
size_t homelen = strlen(homedir);
real_path = malloc(homelen + working_path_len + 1);
if(real_path == NULL) {
free(working_path);
return CURLE_OUT_OF_MEMORY;
}


memcpy(real_path, homedir, homelen);
real_path[homelen] = '/';
real_path[homelen + 1] = '\0';
if(working_path_len > 3) {
memcpy(real_path + homelen + 1, working_path + 3,
1 + working_path_len -3);
}
}
else {
real_path = malloc(working_path_len + 1);
if(real_path == NULL) {
free(working_path);
return CURLE_OUT_OF_MEMORY;
}
memcpy(real_path, working_path, 1 + working_path_len);
}
}

free(working_path);


*path = real_path;

return CURLE_OK;
}


















CURLcode Curl_get_pathname(const char **cpp, char **path, char *homedir)
{
const char *cp = *cpp, *end;
char quot;
unsigned int i, j;
size_t fullPathLength, pathLength;
bool relativePath = false;
static const char WHITESPACE[] = " \t\r\n";

if(!*cp) {
*cpp = NULL;
*path = NULL;
return CURLE_QUOTE_ERROR;
}

cp += strspn(cp, WHITESPACE);

fullPathLength = strlen(cp) + strlen(homedir) + 2;
*path = malloc(fullPathLength);
if(*path == NULL)
return CURLE_OUT_OF_MEMORY;


if(*cp == '\"' || *cp == '\'') {
quot = *cp++;


for(i = j = 0; i <= strlen(cp); i++) {
if(cp[i] == quot) { 
i++;
(*path)[j] = '\0';
break;
}
if(cp[i] == '\0') { 

goto fail;
}
if(cp[i] == '\\') { 
i++;
if(cp[i] != '\'' && cp[i] != '\"' &&
cp[i] != '\\') {


goto fail;
}
}
(*path)[j++] = cp[i];
}

if(j == 0) {

goto fail;
}
*cpp = cp + i + strspn(cp + i, WHITESPACE);
}
else {

end = strpbrk(cp, WHITESPACE);
if(end == NULL)
end = strchr(cp, '\0');

*cpp = end + strspn(end, WHITESPACE);
pathLength = 0;
relativePath = (cp[0] == '/' && cp[1] == '~' && cp[2] == '/');

if(relativePath) {
strcpy(*path, homedir);
pathLength = strlen(homedir);
(*path)[pathLength++] = '/';
(*path)[pathLength] = '\0';
cp += 3;
}

memcpy(&(*path)[pathLength], cp, (int)(end - cp));
pathLength += (int)(end - cp);
(*path)[pathLength] = '\0';
}
return CURLE_OK;

fail:
Curl_safefree(*path);
return CURLE_QUOTE_ERROR;
}

#endif 
