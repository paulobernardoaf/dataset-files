#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
static size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
FILE *f = (FILE *)stream;
size_t n;
if(ferror(f))
return CURL_READFUNC_ABORT;
n = fread(ptr, size, nmemb, f) * size;
return n;
}
static curl_off_t sftpGetRemoteFileSize(const char *i_remoteFile)
{
CURLcode result = CURLE_GOT_NOTHING;
curl_off_t remoteFileSizeByte = -1;
CURL *curlHandlePtr = curl_easy_init();
curl_easy_setopt(curlHandlePtr, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(curlHandlePtr, CURLOPT_URL, i_remoteFile);
curl_easy_setopt(curlHandlePtr, CURLOPT_NOPROGRESS, 1);
curl_easy_setopt(curlHandlePtr, CURLOPT_NOBODY, 1);
curl_easy_setopt(curlHandlePtr, CURLOPT_HEADER, 1);
curl_easy_setopt(curlHandlePtr, CURLOPT_FILETIME, 1);
result = curl_easy_perform(curlHandlePtr);
if(CURLE_OK == result) {
result = curl_easy_getinfo(curlHandlePtr,
CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
&remoteFileSizeByte);
if(result)
return -1;
printf("filesize: %" CURL_FORMAT_CURL_OFF_T "\n", remoteFileSizeByte);
}
curl_easy_cleanup(curlHandlePtr);
return remoteFileSizeByte;
}
static int sftpResumeUpload(CURL *curlhandle, const char *remotepath,
const char *localpath)
{
FILE *f = NULL;
CURLcode result = CURLE_GOT_NOTHING;
curl_off_t remoteFileSizeByte = sftpGetRemoteFileSize(remotepath);
if(-1 == remoteFileSizeByte) {
printf("Error reading the remote file size: unable to resume upload\n");
return -1;
}
f = fopen(localpath, "rb");
if(!f) {
perror(NULL);
return 0;
}
curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);
curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);
curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);
curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);
#if defined(_WIN32)
_fseeki64(f, remoteFileSizeByte, SEEK_SET);
#else
fseek(f, (long)remoteFileSizeByte, SEEK_SET);
#endif
curl_easy_setopt(curlhandle, CURLOPT_APPEND, 1L);
result = curl_easy_perform(curlhandle);
fclose(f);
if(result == CURLE_OK)
return 1;
else {
fprintf(stderr, "%s\n", curl_easy_strerror(result));
return 0;
}
}
int main(void)
{
const char *remote = "sftp://user:pass@example.com/path/filename";
const char *filename = "filename";
CURL *curlhandle = NULL;
curl_global_init(CURL_GLOBAL_ALL);
curlhandle = curl_easy_init();
if(!sftpResumeUpload(curlhandle, remote, filename)) {
printf("resumed upload using curl %s failed\n", curl_version());
}
curl_easy_cleanup(curlhandle);
curl_global_cleanup();
return 0;
}
