#if defined(USE_WINSOCK)
#include <curl/curl.h> 
#endif
#define CURLX_FUNCTION_CAST(target_type, func) (target_type)(void (*) (void))(func)
unsigned short curlx_ultous(unsigned long ulnum);
unsigned char curlx_ultouc(unsigned long ulnum);
int curlx_ultosi(unsigned long ulnum);
int curlx_uztosi(size_t uznum);
curl_off_t curlx_uztoso(size_t uznum);
unsigned long curlx_uztoul(size_t uznum);
unsigned int curlx_uztoui(size_t uznum);
int curlx_sltosi(long slnum);
unsigned int curlx_sltoui(long slnum);
unsigned short curlx_sltous(long slnum);
ssize_t curlx_uztosz(size_t uznum);
size_t curlx_sotouz(curl_off_t sonum);
int curlx_sztosi(ssize_t sznum);
unsigned short curlx_uitous(unsigned int uinum);
size_t curlx_sitouz(int sinum);
#if defined(USE_WINSOCK)
int curlx_sktosi(curl_socket_t s);
curl_socket_t curlx_sitosk(int i);
#endif 
#if defined(WIN32) || defined(_WIN32)
ssize_t curlx_read(int fd, void *buf, size_t count);
ssize_t curlx_write(int fd, const void *buf, size_t count);
#if !defined(BUILDING_WARNLESS_C)
#undef read
#define read(fd, buf, count) curlx_read(fd, buf, count)
#undef write
#define write(fd, buf, count) curlx_write(fd, buf, count)
#endif
#endif 
#if defined(__INTEL_COMPILER) && defined(__unix__)
int curlx_FD_ISSET(int fd, fd_set *fdset);
void curlx_FD_SET(int fd, fd_set *fdset);
void curlx_FD_ZERO(fd_set *fdset);
unsigned short curlx_htons(unsigned short usnum);
unsigned short curlx_ntohs(unsigned short usnum);
#if !defined(BUILDING_WARNLESS_C)
#undef FD_ISSET
#define FD_ISSET(a,b) curlx_FD_ISSET((a),(b))
#undef FD_SET
#define FD_SET(a,b) curlx_FD_SET((a),(b))
#undef FD_ZERO
#define FD_ZERO(a) curlx_FD_ZERO((a))
#undef htons
#define htons(a) curlx_htons((a))
#undef ntohs
#define ntohs(a) curlx_ntohs((a))
#endif
#endif 
