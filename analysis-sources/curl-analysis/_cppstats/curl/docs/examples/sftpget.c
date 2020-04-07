#include <stdio.h>
#include <curl/curl.h>
#undef DISABLE_SSH_AGENT
struct FtpFile {
const char *filename;
FILE *stream;
};
static size_t my_fwrite(void *buffer, size_t size, size_t nmemb,
void *stream)
{
struct FtpFile *out = (struct FtpFile *)stream;
if(!out->stream) {
out->stream = fopen(out->filename, "wb");
if(!out->stream)
return -1; 
}
return fwrite(buffer, size, nmemb, out->stream);
}
int main(void)
{
CURL *curl;
CURLcode res;
struct FtpFile ftpfile = {
"yourfile.bin", 
NULL
};
curl_global_init(CURL_GLOBAL_DEFAULT);
curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL,
"sftp://user@server/home/user/file.txt");
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
#if !defined(DISABLE_SSH_AGENT)
curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_AGENT);
#endif
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
res = curl_easy_perform(curl);
curl_easy_cleanup(curl);
if(CURLE_OK != res) {
fprintf(stderr, "curl told us %d\n", res);
}
}
if(ftpfile.stream)
fclose(ftpfile.stream); 
curl_global_cleanup();
return 0;
}
