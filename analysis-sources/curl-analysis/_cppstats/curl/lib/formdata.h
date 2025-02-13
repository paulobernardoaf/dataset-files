#include "curl_setup.h"
#if !defined(CURL_DISABLE_MIME)
typedef struct FormInfo {
char *name;
bool name_alloc;
size_t namelength;
char *value;
bool value_alloc;
curl_off_t contentslength;
char *contenttype;
bool contenttype_alloc;
long flags;
char *buffer; 
size_t bufferlength;
char *showfilename; 
bool showfilename_alloc;
char *userp; 
struct curl_slist *contentheader;
struct FormInfo *more;
} FormInfo;
CURLcode Curl_getformdata(struct Curl_easy *data,
curl_mimepart *,
struct curl_httppost *post,
curl_read_callback fread_func);
#else
#define Curl_getformdata(a,b,c,d) CURLE_NOT_BUILT_IN
#endif
