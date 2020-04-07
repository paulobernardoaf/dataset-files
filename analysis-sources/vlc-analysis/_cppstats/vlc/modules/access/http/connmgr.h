struct vlc_http_mgr;
struct vlc_http_msg;
struct vlc_http_cookie_jar_t;
struct vlc_http_msg *vlc_http_mgr_request(struct vlc_http_mgr *mgr, bool https,
const char *host, unsigned port,
const struct vlc_http_msg *req);
struct vlc_http_cookie_jar_t *vlc_http_mgr_get_jar(struct vlc_http_mgr *);
struct vlc_http_mgr *vlc_http_mgr_create(vlc_object_t *obj,
struct vlc_http_cookie_jar_t *jar);
void vlc_http_mgr_destroy(struct vlc_http_mgr *mgr);
