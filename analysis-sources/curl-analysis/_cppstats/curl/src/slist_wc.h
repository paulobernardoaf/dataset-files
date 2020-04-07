#include "tool_setup.h"
#if !defined(CURL_DISABLE_LIBCURL_OPTION)
struct slist_wc {
struct curl_slist *first;
struct curl_slist *last;
};
struct slist_wc *slist_wc_append(struct slist_wc *, const char *);
void slist_wc_free_all(struct slist_wc *);
#endif 
