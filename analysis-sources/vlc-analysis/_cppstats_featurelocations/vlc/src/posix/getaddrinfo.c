



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <netdb.h>

#include <vlc_common.h>
#include <vlc_interrupt.h>
#include <vlc_network.h>

struct vlc_gai_req
{
const char *name;
const char *service;
const struct addrinfo *hints;
struct addrinfo **result;
int error;
vlc_sem_t done;
};

static void *vlc_gai_thread(void *data)
{
struct vlc_gai_req *req = data;

req->error = EAI_SYSTEM;
req->error = getaddrinfo(req->name, req->service, req->hints, req->result);
vlc_sem_post(&req->done);
return NULL;
}

int vlc_getaddrinfo_i11e(const char *name, unsigned port,
const struct addrinfo *hints,
struct addrinfo **res)
{
struct vlc_gai_req req =
{
.name = name,
.service = NULL,
.hints = hints,
.result = res,
};
char portbuf[6];
vlc_thread_t th;

if (port != 0)
{
if ((size_t)snprintf(portbuf, sizeof (portbuf), "%u",
port) >= sizeof (portbuf))
return EAI_NONAME;

req.service = portbuf;
}

vlc_sem_init(&req.done, 0);

if (vlc_clone(&th, vlc_gai_thread, &req, VLC_THREAD_PRIORITY_LOW))
return EAI_SYSTEM;

vlc_sem_wait_i11e(&req.done);

vlc_cancel(th);
vlc_join(th, NULL);

return req.error;
}
