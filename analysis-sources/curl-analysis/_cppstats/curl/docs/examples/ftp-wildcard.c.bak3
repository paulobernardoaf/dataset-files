
























#include <curl/curl.h>
#include <stdio.h>

struct callback_data {
FILE *output;
};

static long file_is_coming(struct curl_fileinfo *finfo,
struct callback_data *data,
int remains);

static long file_is_downloaded(struct callback_data *data);

static size_t write_it(char *buff, size_t size, size_t nmemb,
void *cb_data);

int main(int argc, char **argv)
{

CURL *handle;


struct callback_data data = { 0 };


int rc = curl_global_init(CURL_GLOBAL_ALL);
if(rc)
return rc;


handle = curl_easy_init();
if(!handle) {
curl_global_cleanup();
return CURLE_OUT_OF_MEMORY;
}


curl_easy_setopt(handle, CURLOPT_WILDCARDMATCH, 1L);


curl_easy_setopt(handle, CURLOPT_CHUNK_BGN_FUNCTION, file_is_coming);


curl_easy_setopt(handle, CURLOPT_CHUNK_END_FUNCTION, file_is_downloaded);


curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_it);


curl_easy_setopt(handle, CURLOPT_CHUNK_DATA, &data);
curl_easy_setopt(handle, CURLOPT_WRITEDATA, &data);




if(argc == 2)
curl_easy_setopt(handle, CURLOPT_URL, argv[1]);
else
curl_easy_setopt(handle, CURLOPT_URL, "ftp://example.com/test/*");


rc = curl_easy_perform(handle);

curl_easy_cleanup(handle);
curl_global_cleanup();
return rc;
}

static long file_is_coming(struct curl_fileinfo *finfo,
struct callback_data *data,
int remains)
{
printf("%3d %40s %10luB ", remains, finfo->filename,
(unsigned long)finfo->size);

switch(finfo->filetype) {
case CURLFILETYPE_DIRECTORY:
printf(" DIR\n");
break;
case CURLFILETYPE_FILE:
printf("FILE ");
break;
default:
printf("OTHER\n");
break;
}

if(finfo->filetype == CURLFILETYPE_FILE) {

if(finfo->size > 50) {
printf("SKIPPED\n");
return CURL_CHUNK_BGN_FUNC_SKIP;
}

data->output = fopen(finfo->filename, "wb");
if(!data->output) {
return CURL_CHUNK_BGN_FUNC_FAIL;
}
}

return CURL_CHUNK_BGN_FUNC_OK;
}

static long file_is_downloaded(struct callback_data *data)
{
if(data->output) {
printf("DOWNLOADED\n");
fclose(data->output);
data->output = 0x0;
}
return CURL_CHUNK_END_FUNC_OK;
}

static size_t write_it(char *buff, size_t size, size_t nmemb,
void *cb_data)
{
struct callback_data *data = cb_data;
size_t written = 0;
if(data->output)
written = fwrite(buff, size, nmemb, data->output);
else

written = fwrite(buff, size, nmemb, stdout);
return written;
}
