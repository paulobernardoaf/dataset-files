#if defined(BUILDING_LIBCURL) && !defined(CURL_NO_OLDIES)
#define CURL_NO_OLDIES
#endif
#if defined(_MSC_VER)
#pragma warning(disable:4127)
#endif
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32) && !defined(__SYMBIAN32__)
#define WIN32
#endif
#if defined(WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOGDI)
#define NOGDI
#endif
#endif
#if defined(HAVE_CONFIG_H)
#include "curl_config.h"
#else 
#if defined(_WIN32_WCE)
#include "config-win32ce.h"
#else
#if defined(WIN32)
#include "config-win32.h"
#endif
#endif
#if defined(macintosh) && defined(__MRC__)
#include "config-mac.h"
#endif
#if defined(__riscos__)
#include "config-riscos.h"
#endif
#if defined(__AMIGA__)
#include "config-amigaos.h"
#endif
#if defined(__SYMBIAN32__)
#include "config-symbian.h"
#endif
#if defined(__OS400__)
#include "config-os400.h"
#endif
#if defined(TPF)
#include "config-tpf.h"
#endif
#if defined(__VXWORKS__)
#include "config-vxworks.h"
#endif
#if defined(__PLAN9__)
#include "config-plan9.h"
#endif
#endif 
#if defined(NEED_THREAD_SAFE)
#if !defined(_THREAD_SAFE)
#define _THREAD_SAFE
#endif
#endif
#if defined(NEED_REENTRANT)
#if !defined(_REENTRANT)
#define _REENTRANT
#endif
#endif
#if defined(sun) || defined(__sun)
#if !defined(_POSIX_PTHREAD_SEMANTICS)
#define _POSIX_PTHREAD_SEMANTICS 1
#endif
#endif
#include <curl/curl.h>
#define CURL_SIZEOF_CURL_OFF_T SIZEOF_CURL_OFF_T
#if defined(HTTP_ONLY)
#if !defined(CURL_DISABLE_TFTP)
#define CURL_DISABLE_TFTP
#endif
#if !defined(CURL_DISABLE_FTP)
#define CURL_DISABLE_FTP
#endif
#if !defined(CURL_DISABLE_LDAP)
#define CURL_DISABLE_LDAP
#endif
#if !defined(CURL_DISABLE_TELNET)
#define CURL_DISABLE_TELNET
#endif
#if !defined(CURL_DISABLE_DICT)
#define CURL_DISABLE_DICT
#endif
#if !defined(CURL_DISABLE_FILE)
#define CURL_DISABLE_FILE
#endif
#if !defined(CURL_DISABLE_RTSP)
#define CURL_DISABLE_RTSP
#endif
#if !defined(CURL_DISABLE_POP3)
#define CURL_DISABLE_POP3
#endif
#if !defined(CURL_DISABLE_IMAP)
#define CURL_DISABLE_IMAP
#endif
#if !defined(CURL_DISABLE_SMTP)
#define CURL_DISABLE_SMTP
#endif
#if !defined(CURL_DISABLE_GOPHER)
#define CURL_DISABLE_GOPHER
#endif
#if !defined(CURL_DISABLE_SMB)
#define CURL_DISABLE_SMB
#endif
#endif
#if defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_RTSP)
#define CURL_DISABLE_RTSP
#endif
#if defined(__OS400__)
#include "setup-os400.h"
#endif
#if defined(__VMS)
#include "setup-vms.h"
#endif
#if defined(HAVE_WINDOWS_H)
#include "setup-win32.h"
#endif
#if defined(__APPLE__)
#define USE_RESOLVE_ON_IPS 1
#endif
#if defined(USE_LWIPSOCK)
#include <lwip/init.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#endif
#if defined(HAVE_EXTRA_STRICMP_H)
#include <extra/stricmp.h>
#endif
#if defined(HAVE_EXTRA_STRDUP_H)
#include <extra/strdup.h>
#endif
#if defined(TPF)
#include <strings.h> 
#include <string.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <tpf/sysapi.h> 
#define select(a,b,c,d,e) tpf_select_libcurl(a,b,c,d,e)
#endif
#if defined(__VXWORKS__)
#include <sockLib.h> 
#include <ioLib.h> 
#endif
#if defined(__AMIGA__)
#include <exec/types.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <proto/dos.h>
#if defined(HAVE_PROTO_BSDSOCKET_H)
#include <proto/bsdsocket.h> 
#define select(a,b,c,d,e) WaitSelect(a,b,c,d,e,0)
#endif
#endif
#include <stdio.h>
#if defined(HAVE_ASSERT_H)
#include <assert.h>
#endif
#if defined(__TANDEM)
#include <floss.h>
#endif
#if !defined(STDC_HEADERS)
#include <curl/stdcheaders.h>
#endif
#if defined(__POCC__)
#include <sys/types.h>
#include <unistd.h>
#define sys_nerr EILSEQ
#endif
#if defined(__SALFORDC__)
#pragma suppress 353 
#pragma suppress 593 
#pragma suppress 61 
#pragma suppress 106 
#include <clib.h>
#endif
#if defined(USE_WIN32_LARGE_FILES)
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#undef lseek
#define lseek(fdes,offset,whence) _lseeki64(fdes, offset, whence)
#undef fstat
#define fstat(fdes,stp) _fstati64(fdes, stp)
#undef stat
#define stat(fname,stp) _stati64(fname, stp)
#define struct_stat struct _stati64
#define LSEEK_ERROR (__int64)-1
#endif
#if defined(USE_WIN32_SMALL_FILES)
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#if !defined(_WIN32_WCE)
#undef lseek
#define lseek(fdes,offset,whence) _lseek(fdes, (long)offset, whence)
#define fstat(fdes,stp) _fstat(fdes, stp)
#define stat(fname,stp) _stat(fname, stp)
#define struct_stat struct _stat
#endif
#define LSEEK_ERROR (long)-1
#endif
#if !defined(struct_stat)
#define struct_stat struct stat
#endif
#if !defined(LSEEK_ERROR)
#define LSEEK_ERROR (off_t)-1
#endif
#if !defined(SIZEOF_TIME_T)
#define SIZEOF_TIME_T 4
#endif
#if !defined(SIZEOF_OFF_T)
#if defined(__VMS) && !defined(__VAX)
#if defined(_LARGEFILE)
#define SIZEOF_OFF_T 8
#endif
#elif defined(__OS400__) && defined(__ILEC400__)
#if defined(_LARGE_FILES)
#define SIZEOF_OFF_T 8
#endif
#elif defined(__MVS__) && defined(__IBMC__)
#if defined(_LP64) || defined(_LARGE_FILES)
#define SIZEOF_OFF_T 8
#endif
#elif defined(__370__) && defined(__IBMC__)
#if defined(_LP64) || defined(_LARGE_FILES)
#define SIZEOF_OFF_T 8
#endif
#endif
#if !defined(SIZEOF_OFF_T)
#define SIZEOF_OFF_T 4
#endif
#endif
#if (SIZEOF_CURL_OFF_T == 4)
#define CURL_OFF_T_MAX CURL_OFF_T_C(0x7FFFFFFF)
#else
#define CURL_OFF_T_MAX CURL_OFF_T_C(0x7FFFFFFFFFFFFFFF)
#endif
#define CURL_OFF_T_MIN (-CURL_OFF_T_MAX - CURL_OFF_T_C(1))
#if (SIZEOF_TIME_T == 4)
#if defined(HAVE_TIME_T_UNSIGNED)
#define TIME_T_MAX UINT_MAX
#define TIME_T_MIN 0
#else
#define TIME_T_MAX INT_MAX
#define TIME_T_MIN INT_MIN
#endif
#else
#if defined(HAVE_TIME_T_UNSIGNED)
#define TIME_T_MAX 0xFFFFFFFFFFFFFFFF
#define TIME_T_MIN 0
#else
#define TIME_T_MAX 0x7FFFFFFFFFFFFFFF
#define TIME_T_MIN (-TIME_T_MAX - 1)
#endif
#endif
#if !defined(SIZE_T_MAX)
#if defined(SIZEOF_SIZE_T) && (SIZEOF_SIZE_T > 4)
#define SIZE_T_MAX 18446744073709551615U
#else
#define SIZE_T_MAX 4294967295U
#endif
#endif
#if !defined(GETHOSTNAME_TYPE_ARG2)
#if defined(USE_WINSOCK)
#define GETHOSTNAME_TYPE_ARG2 int
#else
#define GETHOSTNAME_TYPE_ARG2 size_t
#endif
#endif
#if defined(WIN32)
#define DIR_CHAR "\\"
#else 
#if defined(MSDOS)
#include <sys/ioctl.h>
#define select(n,r,w,x,t) select_s(n,r,w,x,t)
#define ioctl(x,y,z) ioctlsocket(x,y,(char *)(z))
#include <tcp.h>
#if defined(word)
#undef word
#endif
#if defined(byte)
#undef byte
#endif
#endif 
#if defined(__minix)
extern char *strtok_r(char *s, const char *delim, char **last);
extern struct tm *gmtime_r(const time_t * const timep, struct tm *tmp);
#endif
#define DIR_CHAR "/"
#if !defined(fileno)
int fileno(FILE *stream);
#endif
#endif 
#if defined(_MSC_VER) && !defined(__POCC__) && !defined(USE_LWIPSOCK)
#if !defined(HAVE_WS2TCPIP_H) || ((_MSC_VER < 1300) && !defined(INET6_ADDRSTRLEN))
#undef HAVE_GETADDRINFO_THREADSAFE
#undef HAVE_FREEADDRINFO
#undef HAVE_GETADDRINFO
#undef HAVE_GETNAMEINFO
#undef ENABLE_IPV6
#endif
#endif
#if defined(__LCC__) && defined(WIN32)
#undef USE_THREADS_POSIX
#undef USE_THREADS_WIN32
#endif
#if defined(_MSC_VER) && !defined(__POCC__) && !defined(_MT)
#undef USE_THREADS_POSIX
#undef USE_THREADS_WIN32
#endif
#if defined(ENABLE_IPV6) && defined(HAVE_GETADDRINFO)
#define CURLRES_IPV6
#else
#define CURLRES_IPV4
#endif
#if defined(USE_ARES)
#define CURLRES_ASYNCH
#define CURLRES_ARES
#undef HAVE_GETADDRINFO
#undef HAVE_FREEADDRINFO
#undef HAVE_GETHOSTBYNAME
#elif defined(USE_THREADS_POSIX) || defined(USE_THREADS_WIN32)
#define CURLRES_ASYNCH
#define CURLRES_THREADED
#else
#define CURLRES_SYNCH
#endif
#if defined(USE_WINSOCK) && (USE_WINSOCK != 2)
#define CURL_DISABLE_TELNET 1
#endif
#if defined(_MSC_VER) && !defined(__POCC__)
#if !defined(HAVE_WINSOCK2_H) || ((_MSC_VER < 1300) && !defined(IPPROTO_ESP))
#undef HAVE_STRUCT_SOCKADDR_STORAGE
#endif
#endif
#if defined(_MSC_VER) && !defined(__POCC__)
#if !defined(HAVE_WINDOWS_H) || ((_MSC_VER < 1300) && !defined(_FILETIME_))
#if !defined(ALLOW_MSVC6_WITHOUT_PSDK)
#error MSVC 6.0 requires "February 2003 Platform SDK" a.k.a. "Windows Server 2003 PSDK"
#else
#define CURL_DISABLE_LDAP 1
#endif
#endif
#endif
#if defined(NETWARE)
int netware_init(void);
#if !defined(__NOVELL_LIBC__)
#include <sys/bsdskt.h>
#include <sys/timeval.h>
#endif
#endif
#if defined(HAVE_LIBIDN2) && defined(HAVE_IDN2_H) && !defined(USE_WIN32_IDN)
#define USE_LIBIDN2
#endif
#if defined(USE_LIBIDN2) && defined(USE_WIN32_IDN)
#error "Both libidn2 and WinIDN are enabled, choose one."
#endif
#define LIBIDN_REQUIRED_VERSION "0.4.1"
#if defined(USE_GNUTLS) || defined(USE_OPENSSL) || defined(USE_NSS) || defined(USE_MBEDTLS) || defined(USE_WOLFSSL) || defined(USE_SCHANNEL) || defined(USE_SECTRANSP) || defined(USE_GSKIT) || defined(USE_MESALINK) || defined(USE_BEARSSL)
#define USE_SSL 
#endif
#if !defined(CURL_DISABLE_CRYPTO_AUTH) && (defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI))
#define USE_SPNEGO
#endif
#if !defined(CURL_DISABLE_CRYPTO_AUTH) && (defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI))
#define USE_KERBEROS5
#endif
#if !defined(CURL_DISABLE_NTLM) && !defined(CURL_DISABLE_CRYPTO_AUTH)
#if defined(USE_OPENSSL) || defined(USE_WINDOWS_SSPI) || defined(USE_GNUTLS) || defined(USE_NSS) || defined(USE_SECTRANSP) || defined(USE_OS400CRYPTO) || defined(USE_WIN32_CRYPTO) || defined(USE_MBEDTLS)
#define USE_NTLM
#if defined(USE_MBEDTLS)
#include <mbedtls/md4.h>
#endif
#endif
#endif
#if defined(CURL_WANTS_CA_BUNDLE_ENV)
#error "No longer supported. Set CURLOPT_CAINFO at runtime instead."
#endif
#if defined(USE_LIBSSH2) || defined(USE_LIBSSH) || defined(USE_WOLFSSH)
#define USE_SSH
#endif
#if defined(__GNUC__) && ((__GNUC__ >= 3) || ((__GNUC__ == 2) && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ >= 7)))
#define UNUSED_PARAM __attribute__((__unused__))
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define UNUSED_PARAM 
#define WARN_UNUSED_RESULT
#endif
#if !defined(HEADER_CURL_SETUP_ONCE_H)
#include "curl_setup_once.h"
#endif
#if !defined(Curl_nop_stmt)
#define Curl_nop_stmt do { } while(0)
#endif
#if defined(__LWIP_OPT_H__) || defined(LWIP_HDR_OPT_H)
#if defined(SOCKET) || defined(USE_WINSOCK) || defined(HAVE_WINSOCK_H) || defined(HAVE_WINSOCK2_H) || defined(HAVE_WS2TCPIP_H)
#error "Winsock and lwIP TCP/IP stack definitions shall not coexist!"
#endif
#endif
#if defined(USE_WINSOCK)
#define SHUT_RD 0x00
#define SHUT_WR 0x01
#define SHUT_RDWR 0x02
#endif
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if defined(WIN32) || defined(MSDOS)
#define FOPEN_READTEXT "rt"
#define FOPEN_WRITETEXT "wt"
#define FOPEN_APPENDTEXT "at"
#elif defined(__CYGWIN__)
#define FOPEN_READTEXT "rt"
#define FOPEN_WRITETEXT "w"
#define FOPEN_APPENDTEXT "a"
#else
#define FOPEN_READTEXT "r"
#define FOPEN_WRITETEXT "w"
#define FOPEN_APPENDTEXT "a"
#endif
#if !defined(DONT_USE_RECV_BEFORE_SEND_WORKAROUND)
#if defined(WIN32) || defined(__CYGWIN__)
#define USE_RECV_BEFORE_SEND_WORKAROUND
#endif
#else 
#if defined(USE_RECV_BEFORE_SEND_WORKAROUND)
#undef USE_RECV_BEFORE_SEND_WORKAROUND
#endif
#endif 
#if (defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)) || defined(WINAPI_FAMILY)
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define CURL_WINDOWS_APP
#endif
#endif
#if !defined(CURL_SA_FAMILY_T)
#define CURL_SA_FAMILY_T unsigned short
#endif
#define CURLMAX(x,y) ((x)>(y)?(x):(y))
#define CURLMIN(x,y) ((x)<(y)?(x):(y))
#if defined(HAVE_GETPWUID_R) && defined(HAVE_DECL_GETPWUID_R_MISSING)
struct passwd;
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf,
size_t buflen, struct passwd **result);
#endif
#if defined(DEBUGBUILD)
#define UNITTEST
#else
#define UNITTEST static
#endif
#if defined(USE_NGTCP2) || defined(USE_QUICHE)
#define ENABLE_QUIC
#endif
