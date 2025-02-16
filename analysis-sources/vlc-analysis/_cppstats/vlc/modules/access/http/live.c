#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdlib.h>
#include <vlc_common.h>
#include "message.h"
#include "resource.h"
#include "live.h"
#pragma GCC visibility push(default)
static int vlc_http_live_req(const struct vlc_http_resource *res,
struct vlc_http_msg *req, void *opaque)
{
vlc_http_msg_add_header(req, "Accept-Encoding", "gzip, deflate");
(void) res;
(void) opaque;
return 0;
}
static int vlc_http_live_resp(const struct vlc_http_resource *res,
const struct vlc_http_msg *resp, void *opaque)
{
(void) res;
(void) resp;
(void) opaque;
return 0;
}
static const struct vlc_http_resource_cbs vlc_http_live_callbacks =
{
vlc_http_live_req,
vlc_http_live_resp,
};
struct vlc_http_resource *vlc_http_live_create(struct vlc_http_mgr *mgr,
const char *uri, const char *ua,
const char *ref)
{
struct vlc_http_resource *res = malloc(sizeof (*res));
if (unlikely(res == NULL))
return NULL;
if (vlc_http_res_init(res, &vlc_http_live_callbacks, mgr, uri, ua, ref))
{
free(res);
res = NULL;
}
return res;
}
block_t *vlc_http_live_read(struct vlc_http_resource *res)
{
struct block_t *block = vlc_http_res_read(res);
if (block != NULL && block != vlc_http_error)
return block;
vlc_http_msg_destroy(res->response);
res->response = NULL;
return vlc_http_res_read(res);
}
