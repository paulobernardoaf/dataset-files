#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#if defined(WIN32)
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#else
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#endif
#include <errno.h>
#define IPADDR "127.0.0.1"
#define PORTNUM 80
#if !defined(INADDR_NONE)
#define INADDR_NONE 0xffffffff
#endif
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
return written;
}
static int closecb(void *clientp, curl_socket_t item)
{
(void)clientp;
printf("libcurl wants to close %d now\n", (int)item);
return 0;
}
static curl_socket_t opensocket(void *clientp,
curlsocktype purpose,
struct curl_sockaddr *address)
{
curl_socket_t sockfd;
(void)purpose;
(void)address;
sockfd = *(curl_socket_t *)clientp;
return sockfd;
}
static int sockopt_callback(void *clientp, curl_socket_t curlfd,
curlsocktype purpose)
{
(void)clientp;
(void)curlfd;
(void)purpose;
return CURL_SOCKOPT_ALREADY_CONNECTED;
}
int main(void)
{
CURL *curl;
CURLcode res;
struct sockaddr_in servaddr; 
curl_socket_t sockfd;
#if defined(WIN32)
WSADATA wsaData;
int initwsa = WSAStartup(MAKEWORD(2, 0), &wsaData);
if(initwsa != 0) {
printf("WSAStartup failed: %d\n", initwsa);
return 1;
}
#endif
curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, "http://99.99.99.99:9999");
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if(sockfd == CURL_SOCKET_BAD) {
printf("Error creating listening socket.\n");
return 3;
}
memset(&servaddr, 0, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(PORTNUM);
servaddr.sin_addr.s_addr = inet_addr(IPADDR);
if(INADDR_NONE == servaddr.sin_addr.s_addr) {
close(sockfd);
return 2;
}
if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) ==
-1) {
close(sockfd);
printf("client error: connect: %s\n", strerror(errno));
return 1;
}
curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
curl_easy_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, opensocket);
curl_easy_setopt(curl, CURLOPT_OPENSOCKETDATA, &sockfd);
curl_easy_setopt(curl, CURLOPT_CLOSESOCKETFUNCTION, closecb);
curl_easy_setopt(curl, CURLOPT_CLOSESOCKETDATA, &sockfd);
curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
res = curl_easy_perform(curl);
curl_easy_cleanup(curl);
close(sockfd);
if(res) {
printf("libcurl error: %d\n", res);
return 4;
}
}
#if defined(WIN32)
WSACleanup();
#endif
return 0;
}
