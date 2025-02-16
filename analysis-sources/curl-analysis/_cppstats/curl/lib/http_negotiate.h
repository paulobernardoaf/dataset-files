#if !defined(CURL_DISABLE_HTTP) && defined(USE_SPNEGO)
CURLcode Curl_input_negotiate(struct connectdata *conn, bool proxy,
const char *header);
CURLcode Curl_output_negotiate(struct connectdata *conn, bool proxy);
void Curl_http_auth_cleanup_negotiate(struct connectdata *conn);
#endif 
