struct FILEPROTO {
char *path; 
char *freepath; 
int fd; 
};
#if !defined(CURL_DISABLE_FILE)
extern const struct Curl_handler Curl_handler_file;
#endif
