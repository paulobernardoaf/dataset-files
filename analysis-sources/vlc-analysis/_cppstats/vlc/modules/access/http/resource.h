struct vlc_http_msg;
struct vlc_http_mgr;
struct vlc_http_resource;
struct vlc_http_resource_cbs
{
int (*request_format)(const struct vlc_http_resource *,
struct vlc_http_msg *, void *);
int (*response_validate)(const struct vlc_http_resource *,
const struct vlc_http_msg *, void *);
};
struct vlc_http_resource
{
const struct vlc_http_resource_cbs *cbs;
struct vlc_http_msg *response;
struct vlc_http_mgr *manager;
bool secure;
bool negotiate;
bool failure;
char *host;
unsigned port;
char *authority;
char *path;
char *username;
char *password;
char *agent;
char *referrer;
};
int vlc_http_res_init(struct vlc_http_resource *,
const struct vlc_http_resource_cbs *cbs,
struct vlc_http_mgr *mgr,
const char *uri, const char *ua, const char *ref);
void vlc_http_res_destroy(struct vlc_http_resource *);
struct vlc_http_msg *vlc_http_res_open(struct vlc_http_resource *res, void *);
int vlc_http_res_get_status(struct vlc_http_resource *res);
char *vlc_http_res_get_redirect(struct vlc_http_resource *);
char *vlc_http_res_get_type(struct vlc_http_resource *);
struct block_t *vlc_http_res_read(struct vlc_http_resource *);
int vlc_http_res_set_login(struct vlc_http_resource *res,
const char *username, const char *password);
char *vlc_http_res_get_basic_realm(struct vlc_http_resource *res);
