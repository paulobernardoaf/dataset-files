






































#include <netinet/in.h>
#include <netinet/sctp.h>

#include "config.h"

#if HAVE_POLL_H
#include <poll.h>
#endif

#include "libavutil/intreadwrite.h"
#include "libavutil/parseutils.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include "internal.h"
#include "network.h"
#include "os_support.h"
#include "url.h"

















static int ff_sctp_recvmsg(int s, void *msg, size_t len, struct sockaddr *from,
socklen_t *fromlen, struct sctp_sndrcvinfo *sinfo,
int *msg_flags)
{
int recvb;
struct iovec iov;
char incmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
struct msghdr inmsg = { 0 };
struct cmsghdr *cmsg = NULL;

iov.iov_base = msg;
iov.iov_len = len;

inmsg.msg_name = from;
inmsg.msg_namelen = fromlen ? *fromlen : 0;
inmsg.msg_iov = &iov;
inmsg.msg_iovlen = 1;
inmsg.msg_control = incmsg;
inmsg.msg_controllen = sizeof(incmsg);

if ((recvb = recvmsg(s, &inmsg, msg_flags ? *msg_flags : 0)) < 0)
return recvb;

if (fromlen)
*fromlen = inmsg.msg_namelen;
if (msg_flags)
*msg_flags = inmsg.msg_flags;

for (cmsg = CMSG_FIRSTHDR(&inmsg); cmsg;
cmsg = CMSG_NXTHDR(&inmsg, cmsg)) {
if ((IPPROTO_SCTP == cmsg->cmsg_level) &&
(SCTP_SNDRCV == cmsg->cmsg_type))
break;
}


if (cmsg)
memcpy(sinfo, CMSG_DATA(cmsg), sizeof(struct sctp_sndrcvinfo));

return recvb;
}

static int ff_sctp_send(int s, const void *msg, size_t len,
const struct sctp_sndrcvinfo *sinfo, int flags)
{
struct msghdr outmsg = { 0 };
struct iovec iov;

outmsg.msg_name = NULL;
outmsg.msg_namelen = 0;
outmsg.msg_iov = &iov;
iov.iov_base = (void*)msg;
iov.iov_len = len;
outmsg.msg_iovlen = 1;
outmsg.msg_controllen = 0;

if (sinfo) {
char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
struct cmsghdr *cmsg;

outmsg.msg_control = outcmsg;
outmsg.msg_controllen = sizeof(outcmsg);
outmsg.msg_flags = 0;

cmsg = CMSG_FIRSTHDR(&outmsg);
cmsg->cmsg_level = IPPROTO_SCTP;
cmsg->cmsg_type = SCTP_SNDRCV;
cmsg->cmsg_len = CMSG_LEN(sizeof(struct sctp_sndrcvinfo));

outmsg.msg_controllen = cmsg->cmsg_len;
memcpy(CMSG_DATA(cmsg), sinfo, sizeof(struct sctp_sndrcvinfo));
}

return sendmsg(s, &outmsg, flags | MSG_NOSIGNAL);
}

typedef struct SCTPContext {
const AVClass *class;
int fd;
int listen;
int timeout;
int listen_timeout;
int max_streams;
struct sockaddr_storage dest_addr;
} SCTPContext;

#define OFFSET(x) offsetof(SCTPContext, x)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "listen", "Listen for incoming connections", OFFSET(listen), AV_OPT_TYPE_BOOL,{ .i64 = 0 }, 0, 1, .flags = D|E },
{ "timeout", "Connection timeout (in milliseconds)", OFFSET(timeout), AV_OPT_TYPE_INT, { .i64 = 10000 }, INT_MIN, INT_MAX, .flags = D|E },
{ "listen_timeout", "Bind timeout (in milliseconds)", OFFSET(listen_timeout), AV_OPT_TYPE_INT, { .i64 = -1 }, INT_MIN, INT_MAX, .flags = D|E },
{ "max_streams", "Max stream to allocate", OFFSET(max_streams), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT16_MAX, .flags = D|E },
{ NULL }
};

static const AVClass sctp_class = {
.class_name = "sctp",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

static int sctp_open(URLContext *h, const char *uri, int flags)
{
struct addrinfo *ai, *cur_ai;
struct addrinfo hints = { 0 };
struct sctp_event_subscribe event = { 0 };
struct sctp_initmsg initparams = { 0 };
int port;
int fd = -1;
SCTPContext *s = h->priv_data;
const char *p;
char buf[256];
int ret;
char hostname[1024], proto[1024], path[1024];
char portstr[10];

av_url_split(proto, sizeof(proto), NULL, 0, hostname, sizeof(hostname),
&port, path, sizeof(path), uri);
if (strcmp(proto, "sctp"))
return AVERROR(EINVAL);
if (port <= 0 || port >= 65536) {
av_log(s, AV_LOG_ERROR, "Port missing in uri\n");
return AVERROR(EINVAL);
}

p = strchr(uri, '?');
if (p) {
if (av_find_info_tag(buf, sizeof(buf), "listen", p))
s->listen = 1;
if (av_find_info_tag(buf, sizeof(buf), "max_streams", p))
s->max_streams = strtol(buf, NULL, 10);
}

hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
snprintf(portstr, sizeof(portstr), "%d", port);
ret = getaddrinfo(hostname, portstr, &hints, &ai);
if (ret) {
av_log(h, AV_LOG_ERROR, "Failed to resolve hostname %s: %s\n",
hostname, gai_strerror(ret));
return AVERROR(EIO);
}

cur_ai = ai;

restart:
fd = ff_socket(cur_ai->ai_family, SOCK_STREAM, IPPROTO_SCTP);
if (fd < 0) {
ret = ff_neterrno();
goto fail;
}

if (s->listen) {
if ((fd = ff_listen_bind(fd, cur_ai->ai_addr, cur_ai->ai_addrlen,
s->listen_timeout, h)) < 0) {
ret = fd;
goto fail1;
}
} else {
if ((ret = ff_listen_connect(fd, cur_ai->ai_addr, cur_ai->ai_addrlen,
s->timeout, h, !!cur_ai->ai_next)) < 0) {

if (ret == AVERROR_EXIT)
goto fail1;
else
goto fail;
}
}

event.sctp_data_io_event = 1;


if (setsockopt(fd, IPPROTO_SCTP, SCTP_EVENTS, &event,
sizeof(event)) != 0) {
av_log(h, AV_LOG_ERROR,
"SCTP ERROR: Unable to subscribe to events\n");
goto fail1;
}

if (s->max_streams) {
initparams.sinit_max_instreams = s->max_streams;
initparams.sinit_num_ostreams = s->max_streams;
if (setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &initparams,
sizeof(initparams)) < 0) {
av_log(h, AV_LOG_ERROR,
"SCTP ERROR: Unable to initialize socket max streams %d\n",
s->max_streams);
ret = ff_neterrno();
goto fail1;
}
}

h->priv_data = s;
h->is_streamed = 1;
s->fd = fd;
freeaddrinfo(ai);
return 0;

fail:
if (cur_ai->ai_next) {

cur_ai = cur_ai->ai_next;
if (fd >= 0)
closesocket(fd);
ret = 0;
goto restart;
}
fail1:
ret = AVERROR(EIO);
freeaddrinfo(ai);
return ret;
}

static int sctp_wait_fd(int fd, int write)
{
int ev = write ? POLLOUT : POLLIN;
struct pollfd p = { .fd = fd, .events = ev, .revents = 0 };
int ret;

ret = poll(&p, 1, 100);
return ret < 0 ? ff_neterrno() : p.revents & ev ? 0 : AVERROR(EAGAIN);
}

static int sctp_read(URLContext *h, uint8_t *buf, int size)
{
SCTPContext *s = h->priv_data;
int ret;

if (!(h->flags & AVIO_FLAG_NONBLOCK)) {
ret = sctp_wait_fd(s->fd, 0);
if (ret < 0)
return ret;
}

if (s->max_streams) {

struct sctp_sndrcvinfo info = { 0 };
ret = ff_sctp_recvmsg(s->fd, buf + 2, size - 2, NULL, 0, &info, 0);
AV_WB16(buf, info.sinfo_stream);
ret = ret < 0 ? ret : ret + 2;
} else
ret = recv(s->fd, buf, size, 0);

return ret < 0 ? ff_neterrno() : ret;
}

static int sctp_write(URLContext *h, const uint8_t *buf, int size)
{
SCTPContext *s = h->priv_data;
int ret;

if (!(h->flags & AVIO_FLAG_NONBLOCK)) {
ret = sctp_wait_fd(s->fd, 1);
if (ret < 0)
return ret;
}

if (s->max_streams) {

struct sctp_sndrcvinfo info = { 0 };
info.sinfo_stream = AV_RB16(buf);
if (info.sinfo_stream > s->max_streams) {
av_log(h, AV_LOG_ERROR, "bad input data\n");
return AVERROR_BUG;
}
ret = ff_sctp_send(s->fd, buf + 2, size - 2, &info, MSG_EOR);
} else
ret = send(s->fd, buf, size, MSG_NOSIGNAL);

return ret < 0 ? ff_neterrno() : ret;
}

static int sctp_close(URLContext *h)
{
SCTPContext *s = h->priv_data;
closesocket(s->fd);
return 0;
}

static int sctp_get_file_handle(URLContext *h)
{
SCTPContext *s = h->priv_data;
return s->fd;
}

const URLProtocol ff_sctp_protocol = {
.name = "sctp",
.url_open = sctp_open,
.url_read = sctp_read,
.url_write = sctp_write,
.url_close = sctp_close,
.url_get_file_handle = sctp_get_file_handle,
.priv_data_size = sizeof(SCTPContext),
.flags = URL_PROTOCOL_FLAG_NETWORK,
.priv_data_class = &sctp_class,
};
