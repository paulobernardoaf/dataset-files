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
if(!libtest_arg2) {
fprintf(stderr, "Usage: <url> <file-to-upload>\n");
return TEST_ERR_USAGE;
}
hd_src = fopen(libtest_arg2, "rb");
if(NULL == hd_src) {
fprintf(stderr, "fopen failed with error: %d %s\n",
errno, strerror(errno));
fprintf(stderr, "Error opening file: %s\n", libtest_arg2);
return -2; 
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
test_setopt(curl, CURLOPT_UPLOAD, 1L);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_READDATA, hd_src);
curl_easy_perform(curl);
res = curl_easy_perform(curl);
test_cleanup:
fclose(hd_src);
curl_easy_cleanup(curl);
curl_global_cleanup();
return res;
}
