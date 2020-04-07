#if !defined(CURL_DISABLE_RTSP)
extern const struct Curl_handler Curl_handler_rtsp;
CURLcode Curl_rtsp_parseheader(struct connectdata *conn, char *header);
#else
#define Curl_rtsp_parseheader(x,y) CURLE_NOT_BUILT_IN
#endif 
struct rtsp_conn {
char *rtp_buf;
ssize_t rtp_bufsize;
int rtp_channel;
};
struct RTSP {
struct HTTP http_wrapper; 
long CSeq_sent; 
long CSeq_recv; 
};
