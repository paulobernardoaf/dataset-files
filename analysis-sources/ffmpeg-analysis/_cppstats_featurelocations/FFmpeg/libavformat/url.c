





















#include "avformat.h"
#include "config.h"
#include "url.h"
#if CONFIG_NETWORK
#include "network.h"
#endif
#include "libavutil/avstring.h"






int ff_url_join(char *str, int size, const char *proto,
const char *authorization, const char *hostname,
int port, const char *fmt, ...)
{
#if CONFIG_NETWORK
struct addrinfo hints = { 0 }, *ai;
#endif

str[0] = '\0';
if (proto)
av_strlcatf(str, size, "%s://", proto);
if (authorization && authorization[0])
av_strlcatf(str, size, "%s@", authorization);
#if CONFIG_NETWORK && defined(AF_INET6)


hints.ai_flags = AI_NUMERICHOST;
if (!getaddrinfo(hostname, NULL, &hints, &ai)) {
if (ai->ai_family == AF_INET6) {
av_strlcat(str, "[", size);
av_strlcat(str, hostname, size);
av_strlcat(str, "]", size);
} else {
av_strlcat(str, hostname, size);
}
freeaddrinfo(ai);
} else
#endif

av_strlcat(str, hostname, size);

if (port >= 0)
av_strlcatf(str, size, ":%d", port);
if (fmt) {
va_list vl;
size_t len = strlen(str);

va_start(vl, fmt);
vsnprintf(str + len, size > len ? size - len : 0, fmt, vl);
va_end(vl);
}
return strlen(str);
}

void ff_make_absolute_url(char *buf, int size, const char *base,
const char *rel)
{
char *sep, *path_query;

if (base && strstr(base, "://") && rel[0] == '/') {
if (base != buf)
av_strlcpy(buf, base, size);
sep = strstr(buf, "://");
if (sep) {

if (rel[1] == '/') {
sep[1] = '\0';
} else {

sep += 3;
sep = strchr(sep, '/');
if (sep)
*sep = '\0';
}
}
av_strlcat(buf, rel, size);
return;
}

if (!base || strstr(rel, "://") || rel[0] == '/') {
av_strlcpy(buf, rel, size);
return;
}
if (base != buf)
av_strlcpy(buf, base, size);


path_query = strchr(buf, '?');
if (path_query)
*path_query = '\0';


if (rel[0] == '?') {
av_strlcat(buf, rel, size);
return;
}


sep = strrchr(buf, '/');
if (sep)
sep[1] = '\0';
else
buf[0] = '\0';
while (av_strstart(rel, "../", NULL) && sep) {

sep[0] = '\0';
sep = strrchr(buf, '/');

if (!strcmp(sep ? &sep[1] : buf, "..")) {

av_strlcat(buf, "/", size);
break;
}

if (sep)
sep[1] = '\0';
else
buf[0] = '\0';
rel += 3;
}
av_strlcat(buf, rel, size);
}

AVIODirEntry *ff_alloc_dir_entry(void)
{
AVIODirEntry *entry = av_mallocz(sizeof(AVIODirEntry));
if (entry) {
entry->type = AVIO_ENTRY_UNKNOWN;
entry->size = -1;
entry->modification_timestamp = -1;
entry->access_timestamp = -1;
entry->status_change_timestamp = -1;
entry->user_id = -1;
entry->group_id = -1;
entry->filemode = -1;
}
return entry;
}
