#if !defined(HEADER_CURL_GETHOSTNAME_H)
#define HEADER_CURL_GETHOSTNAME_H























#define HOSTNAME_MAX 1024


int Curl_gethostname(char *name, GETHOSTNAME_TYPE_ARG2 namelen);

#endif 
