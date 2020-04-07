#include <stddef.h>
#include <stdbool.h>
struct vlc_tls;
struct vlc_tls_client;
struct vlc_tls *vlc_https_connect(struct vlc_tls_client *creds,
const char *name, unsigned port,
bool *restrict two);
struct vlc_tls *vlc_https_connect_proxy(void *ctx,
struct vlc_tls_client *creds,
const char *name, unsigned port,
bool *restrict two, const char *proxy);
bool vlc_http_port_blocked(unsigned port);
