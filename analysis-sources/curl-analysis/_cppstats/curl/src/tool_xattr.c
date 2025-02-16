#include "tool_setup.h"
#if defined(HAVE_FSETXATTR)
#include <sys/xattr.h> 
#define USE_XATTR
#elif defined(__FreeBSD_version) && (__FreeBSD_version > 500000)
#include <sys/types.h>
#include <sys/extattr.h>
#define USE_XATTR
#endif
#include "tool_xattr.h"
#include "memdebug.h" 
#if defined(USE_XATTR)
static const struct xattr_mapping {
const char *attr; 
CURLINFO info;
} mappings[] = {
{ "user.xdg.origin.url", CURLINFO_EFFECTIVE_URL },
{ "user.mime_type", CURLINFO_CONTENT_TYPE },
{ NULL, CURLINFO_NONE } 
};
#if defined(UNITTESTS)
bool stripcredentials(char **url);
#else
static
#endif
bool stripcredentials(char **url)
{
CURLU *u;
CURLUcode uc;
char *nurl;
u = curl_url();
if(u) {
uc = curl_url_set(u, CURLUPART_URL, *url, 0);
if(uc)
goto error;
uc = curl_url_set(u, CURLUPART_USER, NULL, 0);
if(uc)
goto error;
uc = curl_url_set(u, CURLUPART_PASSWORD, NULL, 0);
if(uc)
goto error;
uc = curl_url_get(u, CURLUPART_URL, &nurl, 0);
if(uc)
goto error;
curl_url_cleanup(u);
*url = nurl;
return TRUE;
}
error:
curl_url_cleanup(u);
return FALSE;
}
int fwrite_xattr(CURL *curl, int fd)
{
int i = 0;
int err = 0;
while(err == 0 && mappings[i].attr != NULL) {
char *value = NULL;
CURLcode result = curl_easy_getinfo(curl, mappings[i].info, &value);
if(!result && value) {
bool freeptr = FALSE;
if(CURLINFO_EFFECTIVE_URL == mappings[i].info)
freeptr = stripcredentials(&value);
if(value) {
#if defined(HAVE_FSETXATTR_6)
err = fsetxattr(fd, mappings[i].attr, value, strlen(value), 0, 0);
#elif defined(HAVE_FSETXATTR_5)
err = fsetxattr(fd, mappings[i].attr, value, strlen(value), 0);
#elif defined(__FreeBSD_version)
{
ssize_t rc = extattr_set_fd(fd, EXTATTR_NAMESPACE_USER,
mappings[i].attr, value, strlen(value));
err = (rc < 0 ? -1 : 0);
}
#endif
if(freeptr)
curl_free(value);
}
}
i++;
}
return err;
}
#else
int fwrite_xattr(CURL *curl, int fd)
{
(void)curl;
(void)fd;
return 0;
}
#endif
