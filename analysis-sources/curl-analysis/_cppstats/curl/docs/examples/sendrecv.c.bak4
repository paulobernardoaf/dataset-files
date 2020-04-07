

























#include <stdio.h>
#include <string.h>
#include <curl/curl.h>


static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms)
{
struct timeval tv;
fd_set infd, outfd, errfd;
int res;

tv.tv_sec = timeout_ms / 1000;
tv.tv_usec = (timeout_ms % 1000) * 1000;

FD_ZERO(&infd);
FD_ZERO(&outfd);
FD_ZERO(&errfd);

FD_SET(sockfd, &errfd); 

if(for_recv) {
FD_SET(sockfd, &infd);
}
else {
FD_SET(sockfd, &outfd);
}


res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
return res;
}

int main(void)
{
CURL *curl;

const char *request = "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";
size_t request_len = strlen(request);









curl = curl_easy_init();
if(curl) {
CURLcode res;
curl_socket_t sockfd;
size_t nsent_total = 0;

curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");

curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
res = curl_easy_perform(curl);

if(res != CURLE_OK) {
printf("Error: %s\n", curl_easy_strerror(res));
return 1;
}


res = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockfd);

if(res != CURLE_OK) {
printf("Error: %s\n", curl_easy_strerror(res));
return 1;
}

printf("Sending request.\n");

do {



size_t nsent;
do {
nsent = 0;
res = curl_easy_send(curl, request + nsent_total,
request_len - nsent_total, &nsent);
nsent_total += nsent;

if(res == CURLE_AGAIN && !wait_on_socket(sockfd, 0, 60000L)) {
printf("Error: timeout.\n");
return 1;
}
} while(res == CURLE_AGAIN);

if(res != CURLE_OK) {
printf("Error: %s\n", curl_easy_strerror(res));
return 1;
}

printf("Sent %" CURL_FORMAT_CURL_OFF_T " bytes.\n",
(curl_off_t)nsent);

} while(nsent_total < request_len);

printf("Reading response.\n");

for(;;) {

char buf[1024];
size_t nread;
do {
nread = 0;
res = curl_easy_recv(curl, buf, sizeof(buf), &nread);

if(res == CURLE_AGAIN && !wait_on_socket(sockfd, 1, 60000L)) {
printf("Error: timeout.\n");
return 1;
}
} while(res == CURLE_AGAIN);

if(res != CURLE_OK) {
printf("Error: %s\n", curl_easy_strerror(res));
break;
}

if(nread == 0) {

break;
}

printf("Received %" CURL_FORMAT_CURL_OFF_T " bytes.\n",
(curl_off_t)nread);
}


curl_easy_cleanup(curl);
}
return 0;
}
