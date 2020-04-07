#include "test.h"
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#include "memdebug.h"
int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
FILE *hd_src;
int hd;
struct_stat file_info;
struct curl_slist *hl;
struct curl_slist *headerlist = NULL;
const char *buf_1 = "RNFR 505";
const char *buf_2 = "RNTO 505-forreal";
if(!libtest_arg2) {
fprintf(stderr, "Usage: <url> <file-to-upload>\n");
return TEST_ERR_USAGE;
}
hd_src = fopen(libtest_arg2, "rb");
if(NULL == hd_src) {
fprintf(stderr, "fopen failed with error: %d %s\n",
errno, strerror(errno));
fprintf(stderr, "Error opening file: %s\n", libtest_arg2);
return TEST_ERR_MAJOR_BAD; 
}
hd = fstat(fileno(hd_src), &file_info);
if(hd == -1) {
fprintf(stderr, "fstat() failed with error: %d %s\n",
errno, strerror(errno));
fprintf(stderr, "ERROR: cannot open file %s\n", libtest_arg2);
fclose(hd_src);
return TEST_ERR_MAJOR_BAD;
}
if(! file_info.st_size) {
fprintf(stderr, "ERROR: file %s has zero size!\n", libtest_arg2);
fclose(hd_src);
return TEST_ERR_MAJOR_BAD;
}
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
fclose(hd_src);
return TEST_ERR_MAJOR_BAD;
}
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
fclose(hd_src);
return TEST_ERR_MAJOR_BAD;
}
hl = curl_slist_append(headerlist, buf_1);
if(!hl) {
fprintf(stderr, "curl_slist_append() failed\n");
curl_easy_cleanup(curl);
curl_global_cleanup();
fclose(hd_src);
return TEST_ERR_MAJOR_BAD;
}
headerlist = curl_slist_append(hl, buf_2);
if(!headerlist) {
fprintf(stderr, "curl_slist_append() failed\n");
curl_slist_free_all(hl);
curl_easy_cleanup(curl);
curl_global_cleanup();
fclose(hd_src);
return TEST_ERR_MAJOR_BAD;
}
headerlist = hl;
test_setopt(curl, CURLOPT_UPLOAD, 1L);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
test_setopt(curl, CURLOPT_READDATA, hd_src);
test_setopt(curl, CURLOPT_INFILESIZE_LARGE,
(curl_off_t)file_info.st_size);
res = curl_easy_perform(curl);
test_cleanup:
curl_slist_free_all(headerlist);
fclose(hd_src);
curl_easy_cleanup(curl);
curl_global_cleanup();
return res;
}
