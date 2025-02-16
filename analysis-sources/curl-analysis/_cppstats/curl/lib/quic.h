#include "curl_setup.h"
#if defined(ENABLE_QUIC)
#if defined(USE_NGTCP2)
#include "vquic/ngtcp2.h"
#endif
#if defined(USE_QUICHE)
#include "vquic/quiche.h"
#endif
#include "urldata.h"
CURLcode Curl_quic_connect(struct connectdata *conn,
curl_socket_t sockfd,
int sockindex,
const struct sockaddr *addr,
socklen_t addrlen);
CURLcode Curl_quic_is_connected(struct connectdata *conn,
curl_socket_t sockfd,
bool *connected);
int Curl_quic_ver(char *p, size_t len);
CURLcode Curl_quic_done_sending(struct connectdata *conn);
void Curl_quic_done(struct Curl_easy *data, bool premature);
bool Curl_quic_data_pending(const struct Curl_easy *data);
#else 
#define Curl_quic_done_sending(x)
#define Curl_quic_done(x,y)
#define Curl_quic_data_pending(x)
#endif 
