
























#if !defined(__OS400_SYS_)
#define __OS400_SYS_




typedef enum {
LK_SSL_ERROR,
LK_GSK_ERROR,
LK_LDAP_ERROR,
LK_CURL_VERSION,
LK_VERSION_INFO,
LK_VERSION_INFO_DATA,
LK_EASY_STRERROR,
LK_SHARE_STRERROR,
LK_MULTI_STRERROR,
LK_ZLIB_VERSION,
LK_ZLIB_MSG,
LK_LAST
} localkey_t;


extern char * (* Curl_thread_buffer)(localkey_t key, long size);




#define MAX_CONV_EXPANSION 4 

#endif
