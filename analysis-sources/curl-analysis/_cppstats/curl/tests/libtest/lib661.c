#include "test.h"
#include "memdebug.h"
int test(char *URL)
{
CURLcode res;
CURL *curl = NULL;
char *newURL = NULL;
struct curl_slist *slist = NULL;
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
newURL = aprintf("%s/folderA/661", URL);
test_setopt(curl, CURLOPT_URL, newURL);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_IGNORE_CONTENT_LENGTH, 1L);
test_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long) CURLFTPMETHOD_SINGLECWD);
res = curl_easy_perform(curl);
free(newURL);
newURL = aprintf("%s/folderB/661", URL);
test_setopt(curl, CURLOPT_URL, newURL);
res = curl_easy_perform(curl);
curl_easy_cleanup(curl);
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
free(newURL);
newURL = aprintf("%s/folderA/661", URL);
test_setopt(curl, CURLOPT_URL, newURL);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_IGNORE_CONTENT_LENGTH, 1L);
test_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long) CURLFTPMETHOD_NOCWD);
res = curl_easy_perform(curl);
free(newURL);
newURL = aprintf("%s/folderB/661", URL);
test_setopt(curl, CURLOPT_URL, newURL);
test_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long) CURLFTPMETHOD_SINGLECWD);
res = curl_easy_perform(curl);
free(newURL);
newURL = aprintf("%s/folderA/661", URL);
test_setopt(curl, CURLOPT_URL, newURL);
test_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long) CURLFTPMETHOD_NOCWD);
res = curl_easy_perform(curl);
curl_easy_cleanup(curl);
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
slist = curl_slist_append(NULL, "SYST");
if(slist == NULL) {
fprintf(stderr, "curl_slist_append() failed\n");
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_NOBODY, 1L);
test_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long) CURLFTPMETHOD_NOCWD);
test_setopt(curl, CURLOPT_QUOTE, slist);
res = curl_easy_perform(curl);
curl_easy_cleanup(curl);
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_NOBODY, 1L);
test_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long) CURLFTPMETHOD_SINGLECWD);
test_setopt(curl, CURLOPT_QUOTE, slist);
res = curl_easy_perform(curl);
curl_easy_reset(curl);
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_NOBODY, 1L);
test_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long) CURLFTPMETHOD_NOCWD);
test_setopt(curl, CURLOPT_QUOTE, slist);
res = curl_easy_perform(curl);
test_cleanup:
curl_slist_free_all(slist);
free(newURL);
curl_easy_cleanup(curl);
curl_global_cleanup();
return (int)res;
}
