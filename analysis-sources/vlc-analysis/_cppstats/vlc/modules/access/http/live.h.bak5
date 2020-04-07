


























struct vlc_http_resource;
struct block_t;

struct vlc_http_resource *vlc_http_live_create(struct vlc_http_mgr *mgr,
const char *uri, const char *ua,
const char *ref);
struct block_t *vlc_http_live_read(struct vlc_http_resource *);

#define vlc_http_live_get_status vlc_http_res_get_status
#define vlc_http_live_get_redirect vlc_http_res_get_redirect
#define vlc_http_live_get_type vlc_http_res_get_type
#define vlc_http_live_destroy vlc_http_res_destroy


