
























#include "test.h"

#include "warnless.h"
#include "memdebug.h"

struct data {
char trace_ascii; 
};

static
void dump(const char *text,
FILE *stream, unsigned char *ptr, size_t size,
char nohex)
{
size_t i;
size_t c;

unsigned int width = 0x10;

if(nohex)

width = 0x40;

fprintf(stream, "%s, %zu bytes (0x%zx)\n", text, size, size);

for(i = 0; i<size; i += width) {

fprintf(stream, "%04zx: ", i);

if(!nohex) {

for(c = 0; c < width; c++)
if(i + c < size)
fprintf(stream, "%02x ", ptr[i + c]);
else
fputs(" ", stream);
}

for(c = 0; (c < width) && (i + c < size); c++) {

if(nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
ptr[i + c + 1] == 0x0A) {
i += (c + 2 - width);
break;
}
fprintf(stream, "%c",
(ptr[i + c] >= 0x20) && (ptr[i + c]<0x80)? ptr[i + c] : '.');

if(nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
ptr[i + c + 2] == 0x0A) {
i += (c + 3 - width);
break;
}
}
fputc('\n', stream); 
}
fflush(stream);
}

static
int my_trace(CURL *handle, curl_infotype type,
char *data, size_t size,
void *userp)
{
struct data *config = (struct data *)userp;
const char *text;
(void)handle; 

switch(type) {
case CURLINFO_TEXT:
fprintf(stderr, "== Info: %s", (char *)data);

default: 
return 0;

case CURLINFO_HEADER_OUT:
text = "=> Send header";
break;
case CURLINFO_DATA_OUT:
text = "=> Send data";
break;
case CURLINFO_SSL_DATA_OUT:
text = "=> Send SSL data";
break;
case CURLINFO_HEADER_IN:
text = "<= Recv header";
break;
case CURLINFO_DATA_IN:
text = "<= Recv data";
break;
case CURLINFO_SSL_DATA_IN:
text = "<= Recv SSL data";
break;
}

dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
return 0;
}


static size_t current_offset = 0;
static char databuf[70000]; 


static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
size_t amount = nmemb * size; 
size_t available = sizeof(databuf) - current_offset; 

size_t given = amount < available ? amount : available; 
(void)stream;
memcpy(ptr, databuf + current_offset, given);
current_offset += given;
return given;
}


static size_t write_callback(void *ptr, size_t size, size_t nmemb,
void *stream)
{
int amount = curlx_uztosi(size * nmemb);
printf("%.*s", amount, (char *)ptr);
(void)stream;
return size * nmemb;
}


static curlioerr ioctl_callback(CURL *handle, int cmd, void *clientp)
{
(void)clientp;
if(cmd == CURLIOCMD_RESTARTREAD) {
printf("APPLICATION: received a CURLIOCMD_RESTARTREAD request\n");
printf("APPLICATION: ** REWINDING! **\n");
current_offset = 0;
return CURLIOE_OK;
}
(void)handle;
return CURLIOE_UNKNOWNCMD;
}



int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
struct data config;
size_t i;
static const char fill[] = "test data";

config.trace_ascii = 1; 

global_init(CURL_GLOBAL_ALL);
easy_init(curl);

test_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
test_setopt(curl, CURLOPT_DEBUGDATA, &config);

test_setopt(curl, CURLOPT_VERBOSE, 1L);


for(i = 0; i < sizeof(databuf); ++i)
databuf[i] = fill[i % sizeof(fill)];


test_setopt(curl, CURLOPT_POST, 1L);

#if defined(CURL_DOES_CONVERSIONS)

test_setopt(curl, CURLOPT_TRANSFERTEXT, 1L);
#endif


test_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) sizeof(databuf));
test_setopt(curl, CURLOPT_READFUNCTION, read_callback);


test_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);


test_setopt(curl, CURLOPT_IOCTLFUNCTION, ioctl_callback);

test_setopt(curl, CURLOPT_PROXY, libtest_arg2);

test_setopt(curl, CURLOPT_URL, URL);



test_setopt(curl, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);

res = curl_easy_perform(curl);
fprintf(stderr, "curl_easy_perform = %d\n", (int)res);

test_cleanup:

curl_easy_cleanup(curl);
curl_global_cleanup();
return (int)res;
}
