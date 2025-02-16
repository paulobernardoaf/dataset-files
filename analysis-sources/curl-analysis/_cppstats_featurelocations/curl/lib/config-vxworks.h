#if !defined(HEADER_CURL_CONFIG_VXWORKS_H)
#define HEADER_CURL_CONFIG_VXWORKS_H













































#define CURL_DISABLE_FTP 1





#define CURL_DISABLE_LDAP 1


#define CURL_DISABLE_LDAPS 1


#define CURL_DISABLE_NTLM 1





#define CURL_DISABLE_TELNET 1


#define CURL_DISABLE_TFTP 1














#define ENABLE_IPV6 1


#define GETNAMEINFO_QUAL_ARG1 const


#define GETNAMEINFO_TYPE_ARG1 struct sockaddr *


#define GETNAMEINFO_TYPE_ARG2 socklen_t


#define GETNAMEINFO_TYPE_ARG46 size_t


#define GETNAMEINFO_TYPE_ARG7 unsigned int


#define GETSERVBYPORT_R_ARGS 6


#define GETSERVBYPORT_R_BUFSIZE 4096


#define HAVE_ALARM 1


#define HAVE_ALLOCA_H 1


#define HAVE_ARPA_INET_H 1





#define HAVE_ASSERT_H 1





#define HAVE_BOOL_T 1








#define HAVE_CRYPTO_CLEANUP_ALL_EX_DATA 1





#define HAVE_DLFCN_H 1


#define HAVE_ERRNO_H 1





#define HAVE_FCNTL 1


#define HAVE_FCNTL_H 1


#define HAVE_FCNTL_O_NONBLOCK 1


#define HAVE_FORK 1


#define HAVE_FREEADDRINFO 1


#define HAVE_FREEIFADDRS 1


#define HAVE_FTRUNCATE 1


#define HAVE_GETADDRINFO 1





#define HAVE_GETHOSTBYADDR 1


#define HAVE_GETHOSTBYADDR_R 1








#define HAVE_GETHOSTBYADDR_R_8 1


#define HAVE_GETHOSTBYNAME 1














#define HAVE_GETHOSTNAME 1





#define HAVE_GETNAMEINFO 1





#define HAVE_GETPPID 1


#define HAVE_GETPROTOBYNAME 1





#define HAVE_GETRLIMIT 1











#define HAVE_GMTIME_R 1



































#define HAVE_INET_ADDR 1

















#define HAVE_INTTYPES_H 1


#define HAVE_IOCTL 1















#define HAVE_IOCTL_FIONBIO 1


#define HAVE_IOCTL_SIOCGIFADDR 1


#define HAVE_IO_H 1





















































#define HAVE_LIBZ 1


#define HAVE_LL 1


#define HAVE_LOCALE_H 1


#define HAVE_LOCALTIME_R 1


#define HAVE_LONGLONG 1


#define HAVE_MALLOC_H 1


#define HAVE_MEMORY_H 1





#define HAVE_NETDB_H 1


#define HAVE_NETINET_IN_H 1


#define HAVE_NETINET_TCP_H 1


#define HAVE_NET_IF_H 1









#define HAVE_OPENSSL_CRYPTO_H 1


#define HAVE_OPENSSL_ERR_H 1


#define HAVE_OPENSSL_PEM_H 1


#define HAVE_OPENSSL_PKCS12_H 1


#define HAVE_OPENSSL_RSA_H 1


#define HAVE_OPENSSL_SSL_H 1


#define HAVE_OPENSSL_X509_H 1





#define HAVE_PERROR 1


#define HAVE_PIPE 1

















#define HAVE_RAND_EGD 1





#define HAVE_RAND_STATUS 1


#define HAVE_RECV 1


#define HAVE_RECVFROM 1





#define HAVE_SELECT 1


#define HAVE_SEND 1


#define HAVE_SETJMP_H 1


#define HAVE_SETLOCALE 1


#define HAVE_SETMODE 1


#define HAVE_SETRLIMIT 1


#define HAVE_SETSOCKOPT 1








#define HAVE_SIGACTION 1


#define HAVE_SIGINTERRUPT 1


#define HAVE_SIGNAL 1


#define HAVE_SIGNAL_H 1





#define HAVE_SIG_ATOMIC_T 1





#define HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID 1


#define HAVE_SOCKET 1





#define HAVE_STDBOOL_H 1





#define HAVE_STDIO_H 1


#define HAVE_STDLIB_H 1


#define HAVE_STRCASECMP 1





#define HAVE_STRDUP 1


#define HAVE_STRERROR_R 1





#define HAVE_STRINGS_H 1


#define HAVE_STRING_H 1





#define HAVE_STRNCASECMP 1











#define HAVE_STRSTR 1


#define HAVE_STRTOK_R 1





#define HAVE_STRUCT_SOCKADDR_STORAGE 1


#define HAVE_STRUCT_TIMEVAL 1





#define HAVE_SYS_IOCTL_H 1








#define HAVE_SYS_RESOURCE_H 1





#define HAVE_SYS_SOCKET_H 1





#define HAVE_SYS_STAT_H 1





#define HAVE_SYS_TYPES_H 1


#define HAVE_SYS_UIO_H 1


#define HAVE_SYS_UN_H 1


#define HAVE_SYS_UTIME_H 1


#define HAVE_TERMIOS_H 1


#define HAVE_TERMIO_H 1


#define HAVE_TIME_H 1








#define HAVE_UNAME 1


#define HAVE_UNISTD_H 1


#define HAVE_UTIME 1


#define HAVE_UTIME_H 1


#define HAVE_VARIADIC_MACROS_C99 1


#define HAVE_VARIADIC_MACROS_GCC 1


#define HAVE_VXWORKS_STRERROR_R 1

















#define HAVE_WRITABLE_ARGV 1


#define HAVE_WRITEV 1








#define HAVE_ZLIB_H 1

















#define OPEN_NEEDS_ARG3 1


#define OS "unknown-unknown-vxworks"


#define PACKAGE "curl"


#define RANDOM_FILE "/dev/urandom"


#define RECVFROM_TYPE_ARG1 int


#define RECVFROM_TYPE_ARG2 void


#define RECVFROM_TYPE_ARG2_IS_VOID 1


#define RECVFROM_TYPE_ARG3 size_t


#define RECVFROM_TYPE_ARG4 int


#define RECVFROM_TYPE_ARG5 struct sockaddr





#define RECVFROM_TYPE_ARG6 socklen_t





#define RECVFROM_TYPE_RETV int


#define RECV_TYPE_ARG1 int


#define RECV_TYPE_ARG2 void *


#define RECV_TYPE_ARG3 size_t


#define RECV_TYPE_ARG4 int


#define RECV_TYPE_RETV int


#define RETSIGTYPE void


#define SELECT_QUAL_ARG5


#define SELECT_TYPE_ARG1 int


#define SELECT_TYPE_ARG234 fd_set *


#define SELECT_TYPE_ARG5 struct timeval *


#define SELECT_TYPE_RETV int


#define SEND_QUAL_ARG2 const


#define SEND_TYPE_ARG1 int


#define SEND_TYPE_ARG2 void *


#define SEND_TYPE_ARG3 size_t


#define SEND_TYPE_ARG4 int


#define SEND_TYPE_RETV int


#define SIZEOF_INT 4


#define SIZEOF_LONG 4


#define SIZEOF_OFF_T 8


#define SIZEOF_SHORT 2


#define SIZEOF_SIZE_T 4


#define SIZEOF_TIME_T 4


#define STDC_HEADERS 1




















#define USE_MANUAL 1





#define USE_OPENSSL 1















#if !defined(_ALL_SOURCE)

#endif















#if !defined(__cplusplus)

#endif







#endif 
