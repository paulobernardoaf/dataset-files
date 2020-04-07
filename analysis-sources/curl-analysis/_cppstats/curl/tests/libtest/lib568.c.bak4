




















#include "test.h"

#if defined(HAVE_SYS_STAT_H)
#include <sys/stat.h>
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include "memdebug.h"


static char *suburl(const char *base, int i)
{
return curl_maprintf("%s%.4d", base, i);
}




int test(char *URL)
{
int res;
CURL *curl;
int sdp;
FILE *sdpf = NULL;
struct_stat file_info;
char *stream_uri = NULL;
int request = 1;
struct curl_slist *custom_headers = NULL;

if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}

curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

test_setopt(curl, CURLOPT_HEADERDATA, stdout);
test_setopt(curl, CURLOPT_WRITEDATA, stdout);

test_setopt(curl, CURLOPT_URL, URL);

stream_uri = suburl(URL, request++);
if(!stream_uri) {
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
free(stream_uri);
stream_uri = NULL;

sdp = open("log/file568.txt", O_RDONLY);
fstat(sdp, &file_info);
close(sdp);

sdpf = fopen("log/file568.txt", "rb");
if(sdpf == NULL) {
fprintf(stderr, "can't open log/file568.txt\n");
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_ANNOUNCE);

test_setopt(curl, CURLOPT_READDATA, sdpf);
test_setopt(curl, CURLOPT_UPLOAD, 1L);
test_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) file_info.st_size);


res = curl_easy_perform(curl);
if(res)
goto test_cleanup;

test_setopt(curl, CURLOPT_UPLOAD, 0L);
fclose(sdpf);
sdpf = NULL;


stream_uri = suburl(URL, request++);
if(!stream_uri) {
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
free(stream_uri);
stream_uri = NULL;

test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_DESCRIBE);
res = curl_easy_perform(curl);
if(res)
goto test_cleanup;



stream_uri = suburl(URL, request++);
if(!stream_uri) {
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
free(stream_uri);
stream_uri = NULL;

custom_headers = curl_slist_append(custom_headers,
"Content-Type: posty goodness");
if(!custom_headers) {
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_RTSPHEADER, custom_headers);
test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_ANNOUNCE);
test_setopt(curl, CURLOPT_POSTFIELDS,
"postyfield=postystuff&project=curl\n");

res = curl_easy_perform(curl);
if(res)
goto test_cleanup;

test_setopt(curl, CURLOPT_POSTFIELDS, NULL);
test_setopt(curl, CURLOPT_RTSPHEADER, NULL);
curl_slist_free_all(custom_headers);
custom_headers = NULL;


stream_uri = suburl(URL, request++);
if(!stream_uri) {
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
free(stream_uri);
stream_uri = NULL;

test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_OPTIONS);
res = curl_easy_perform(curl);

test_cleanup:

if(sdpf)
fclose(sdpf);

free(stream_uri);

if(custom_headers)
curl_slist_free_all(custom_headers);

curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
