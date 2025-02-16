#include "libavutil/parseutils.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include "avio_internal.h"
#include "rtp.h"
#include "rtpproto.h"
#include "url.h"
#include "ip.h"
#include <stdarg.h>
#include "internal.h"
#include "network.h"
#include "os_support.h"
#include <fcntl.h>
#if HAVE_POLL_H
#include <poll.h>
#endif
typedef struct RTPContext {
const AVClass *class;
URLContext *rtp_hd, *rtcp_hd, *fec_hd;
int rtp_fd, rtcp_fd;
IPSourceFilters filters;
int write_to_source;
struct sockaddr_storage last_rtp_source, last_rtcp_source;
socklen_t last_rtp_source_len, last_rtcp_source_len;
int ttl;
int buffer_size;
int rtcp_port, local_rtpport, local_rtcpport;
int connect;
int pkt_size;
int dscp;
char *sources;
char *block;
char *fec_options_str;
} RTPContext;
#define OFFSET(x) offsetof(RTPContext, x)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "ttl", "Time to live (in milliseconds, multicast only)", OFFSET(ttl), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, .flags = D|E },
{ "buffer_size", "Send/Receive buffer size (in bytes)", OFFSET(buffer_size), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, .flags = D|E },
{ "rtcp_port", "Custom rtcp port", OFFSET(rtcp_port), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, .flags = D|E },
{ "local_rtpport", "Local rtp port", OFFSET(local_rtpport), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, .flags = D|E },
{ "local_rtcpport", "Local rtcp port", OFFSET(local_rtcpport), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, .flags = D|E },
{ "connect", "Connect socket", OFFSET(connect), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, .flags = D|E },
{ "write_to_source", "Send packets to the source address of the latest received packet", OFFSET(write_to_source), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, .flags = D|E },
{ "pkt_size", "Maximum packet size", OFFSET(pkt_size), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, .flags = D|E },
{ "dscp", "DSCP class", OFFSET(dscp), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, .flags = D|E },
{ "sources", "Source list", OFFSET(sources), AV_OPT_TYPE_STRING, { .str = NULL }, .flags = D|E },
{ "block", "Block list", OFFSET(block), AV_OPT_TYPE_STRING, { .str = NULL }, .flags = D|E },
{ "fec", "FEC", OFFSET(fec_options_str), AV_OPT_TYPE_STRING, { .str = NULL }, .flags = E },
{ NULL }
};
static const AVClass rtp_class = {
.class_name = "rtp",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
int ff_rtp_set_remote_url(URLContext *h, const char *uri)
{
RTPContext *s = h->priv_data;
char hostname[256];
int port, rtcp_port;
const char *p;
char buf[1024];
char path[1024];
av_url_split(NULL, 0, NULL, 0, hostname, sizeof(hostname), &port,
path, sizeof(path), uri);
rtcp_port = port + 1;
p = strchr(uri, '?');
if (p) {
if (av_find_info_tag(buf, sizeof(buf), "rtcpport", p)) {
rtcp_port = strtol(buf, NULL, 10);
}
}
ff_url_join(buf, sizeof(buf), "udp", NULL, hostname, port, "%s", path);
ff_udp_set_remote_url(s->rtp_hd, buf);
ff_url_join(buf, sizeof(buf), "udp", NULL, hostname, rtcp_port, "%s", path);
ff_udp_set_remote_url(s->rtcp_hd, buf);
return 0;
}
static int get_port(const struct sockaddr_storage *ss)
{
if (ss->ss_family == AF_INET)
return ntohs(((const struct sockaddr_in *)ss)->sin_port);
#if HAVE_STRUCT_SOCKADDR_IN6
if (ss->ss_family == AF_INET6)
return ntohs(((const struct sockaddr_in6 *)ss)->sin6_port);
#endif
return 0;
}
static void set_port(struct sockaddr_storage *ss, int port)
{
if (ss->ss_family == AF_INET)
((struct sockaddr_in *)ss)->sin_port = htons(port);
#if HAVE_STRUCT_SOCKADDR_IN6
else if (ss->ss_family == AF_INET6)
((struct sockaddr_in6 *)ss)->sin6_port = htons(port);
#endif
}
static av_printf_format(3, 4) void url_add_option(char *buf, int buf_size, const char *fmt, ...)
{
char buf1[1024];
va_list ap;
va_start(ap, fmt);
if (strchr(buf, '?'))
av_strlcat(buf, "&", buf_size);
else
av_strlcat(buf, "?", buf_size);
vsnprintf(buf1, sizeof(buf1), fmt, ap);
av_strlcat(buf, buf1, buf_size);
va_end(ap);
}
static void build_udp_url(RTPContext *s,
char *buf, int buf_size,
const char *hostname,
int port, int local_port,
const char *include_sources,
const char *exclude_sources)
{
ff_url_join(buf, buf_size, "udp", NULL, hostname, port, NULL);
if (local_port >= 0)
url_add_option(buf, buf_size, "localport=%d", local_port);
if (s->ttl >= 0)
url_add_option(buf, buf_size, "ttl=%d", s->ttl);
if (s->buffer_size >= 0)
url_add_option(buf, buf_size, "buffer_size=%d", s->buffer_size);
if (s->pkt_size >= 0)
url_add_option(buf, buf_size, "pkt_size=%d", s->pkt_size);
if (s->connect)
url_add_option(buf, buf_size, "connect=1");
if (s->dscp >= 0)
url_add_option(buf, buf_size, "dscp=%d", s->dscp);
url_add_option(buf, buf_size, "fifo_size=0");
if (include_sources && include_sources[0])
url_add_option(buf, buf_size, "sources=%s", include_sources);
if (exclude_sources && exclude_sources[0])
url_add_option(buf, buf_size, "block=%s", exclude_sources);
}
static int rtp_open(URLContext *h, const char *uri, int flags)
{
RTPContext *s = h->priv_data;
AVDictionary *fec_opts = NULL;
int rtp_port;
char hostname[256], include_sources[1024] = "", exclude_sources[1024] = "";
char *sources = include_sources, *block = exclude_sources;
char *fec_protocol = NULL;
char buf[1024];
char path[1024];
const char *p;
int i, max_retry_count = 3;
int rtcpflags;
av_url_split(NULL, 0, NULL, 0, hostname, sizeof(hostname), &rtp_port,
path, sizeof(path), uri);
if (s->rtcp_port < 0)
s->rtcp_port = rtp_port + 1;
p = strchr(uri, '?');
if (p) {
if (av_find_info_tag(buf, sizeof(buf), "ttl", p)) {
s->ttl = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "rtcpport", p)) {
s->rtcp_port = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "localport", p)) {
s->local_rtpport = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "localrtpport", p)) {
s->local_rtpport = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "localrtcpport", p)) {
s->local_rtcpport = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "pkt_size", p)) {
s->pkt_size = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "connect", p)) {
s->connect = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "write_to_source", p)) {
s->write_to_source = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "dscp", p)) {
s->dscp = strtol(buf, NULL, 10);
}
if (av_find_info_tag(buf, sizeof(buf), "sources", p)) {
av_strlcpy(include_sources, buf, sizeof(include_sources));
ff_ip_parse_sources(h, buf, &s->filters);
} else {
ff_ip_parse_sources(h, s->sources, &s->filters);
sources = s->sources;
}
if (av_find_info_tag(buf, sizeof(buf), "block", p)) {
av_strlcpy(exclude_sources, buf, sizeof(exclude_sources));
ff_ip_parse_blocks(h, buf, &s->filters);
} else {
ff_ip_parse_blocks(h, s->block, &s->filters);
block = s->block;
}
}
if (s->fec_options_str) {
p = s->fec_options_str;
if (!(fec_protocol = av_get_token(&p, "="))) {
av_log(h, AV_LOG_ERROR, "Failed to parse the FEC protocol value\n");
goto fail;
}
if (strcmp(fec_protocol, "prompeg")) {
av_log(h, AV_LOG_ERROR, "Unsupported FEC protocol %s\n", fec_protocol);
goto fail;
}
p = s->fec_options_str + strlen(fec_protocol);
while (*p && *p == '=') p++;
if (av_dict_parse_string(&fec_opts, p, "=", ":", 0) < 0) {
av_log(h, AV_LOG_ERROR, "Failed to parse the FEC options\n");
goto fail;
}
if (s->ttl > 0) {
snprintf(buf, sizeof (buf), "%d", s->ttl);
av_dict_set(&fec_opts, "ttl", buf, 0);
}
}
for (i = 0; i < max_retry_count; i++) {
build_udp_url(s, buf, sizeof(buf),
hostname, rtp_port, s->local_rtpport,
sources, block);
if (ffurl_open_whitelist(&s->rtp_hd, buf, flags, &h->interrupt_callback,
NULL, h->protocol_whitelist, h->protocol_blacklist, h) < 0)
goto fail;
s->local_rtpport = ff_udp_get_local_port(s->rtp_hd);
if(s->local_rtpport == 65535) {
s->local_rtpport = -1;
continue;
}
rtcpflags = flags | AVIO_FLAG_WRITE;
if (s->local_rtcpport < 0) {
s->local_rtcpport = s->local_rtpport + 1;
build_udp_url(s, buf, sizeof(buf),
hostname, s->rtcp_port, s->local_rtcpport,
sources, block);
if (ffurl_open_whitelist(&s->rtcp_hd, buf, rtcpflags,
&h->interrupt_callback, NULL,
h->protocol_whitelist, h->protocol_blacklist, h) < 0) {
s->local_rtpport = s->local_rtcpport = -1;
continue;
}
break;
}
build_udp_url(s, buf, sizeof(buf),
hostname, s->rtcp_port, s->local_rtcpport,
sources, block);
if (ffurl_open_whitelist(&s->rtcp_hd, buf, rtcpflags, &h->interrupt_callback,
NULL, h->protocol_whitelist, h->protocol_blacklist, h) < 0)
goto fail;
break;
}
s->fec_hd = NULL;
if (fec_protocol) {
ff_url_join(buf, sizeof(buf), fec_protocol, NULL, hostname, rtp_port, NULL);
if (ffurl_open_whitelist(&s->fec_hd, buf, flags, &h->interrupt_callback,
&fec_opts, h->protocol_whitelist, h->protocol_blacklist, h) < 0)
goto fail;
}
s->rtp_fd = ffurl_get_file_handle(s->rtp_hd);
s->rtcp_fd = ffurl_get_file_handle(s->rtcp_hd);
h->max_packet_size = s->rtp_hd->max_packet_size;
h->is_streamed = 1;
av_free(fec_protocol);
av_dict_free(&fec_opts);
return 0;
fail:
if (s->rtp_hd)
ffurl_close(s->rtp_hd);
if (s->rtcp_hd)
ffurl_close(s->rtcp_hd);
ffurl_closep(&s->fec_hd);
av_free(fec_protocol);
av_dict_free(&fec_opts);
return AVERROR(EIO);
}
static int rtp_read(URLContext *h, uint8_t *buf, int size)
{
RTPContext *s = h->priv_data;
int len, n, i;
struct pollfd p[2] = {{s->rtp_fd, POLLIN, 0}, {s->rtcp_fd, POLLIN, 0}};
int poll_delay = h->flags & AVIO_FLAG_NONBLOCK ? 0 : 100;
struct sockaddr_storage *addrs[2] = { &s->last_rtp_source, &s->last_rtcp_source };
socklen_t *addr_lens[2] = { &s->last_rtp_source_len, &s->last_rtcp_source_len };
for(;;) {
if (ff_check_interrupt(&h->interrupt_callback))
return AVERROR_EXIT;
n = poll(p, 2, poll_delay);
if (n > 0) {
for (i = 1; i >= 0; i--) {
if (!(p[i].revents & POLLIN))
continue;
*addr_lens[i] = sizeof(*addrs[i]);
len = recvfrom(p[i].fd, buf, size, 0,
(struct sockaddr *)addrs[i], addr_lens[i]);
if (len < 0) {
if (ff_neterrno() == AVERROR(EAGAIN) ||
ff_neterrno() == AVERROR(EINTR))
continue;
return AVERROR(EIO);
}
if (ff_ip_check_source_lists(addrs[i], &s->filters))
continue;
return len;
}
} else if (n < 0) {
if (ff_neterrno() == AVERROR(EINTR))
continue;
return AVERROR(EIO);
}
if (h->flags & AVIO_FLAG_NONBLOCK)
return AVERROR(EAGAIN);
}
}
static int rtp_write(URLContext *h, const uint8_t *buf, int size)
{
RTPContext *s = h->priv_data;
int ret, ret_fec;
URLContext *hd;
if (size < 2)
return AVERROR(EINVAL);
if ((buf[0] & 0xc0) != (RTP_VERSION << 6))
av_log(h, AV_LOG_WARNING, "Data doesn't look like RTP packets, "
"make sure the RTP muxer is used\n");
if (s->write_to_source) {
int fd;
struct sockaddr_storage *source, temp_source;
socklen_t *source_len, temp_len;
if (!s->last_rtp_source.ss_family && !s->last_rtcp_source.ss_family) {
av_log(h, AV_LOG_ERROR,
"Unable to send packet to source, no packets received yet\n");
return size;
}
if (RTP_PT_IS_RTCP(buf[1])) {
fd = s->rtcp_fd;
source = &s->last_rtcp_source;
source_len = &s->last_rtcp_source_len;
} else {
fd = s->rtp_fd;
source = &s->last_rtp_source;
source_len = &s->last_rtp_source_len;
}
if (!source->ss_family) {
source = &temp_source;
source_len = &temp_len;
if (RTP_PT_IS_RTCP(buf[1])) {
temp_source = s->last_rtp_source;
temp_len = s->last_rtp_source_len;
set_port(source, get_port(source) + 1);
av_log(h, AV_LOG_INFO,
"Not received any RTCP packets yet, inferring peer port "
"from the RTP port\n");
} else {
temp_source = s->last_rtcp_source;
temp_len = s->last_rtcp_source_len;
set_port(source, get_port(source) - 1);
av_log(h, AV_LOG_INFO,
"Not received any RTP packets yet, inferring peer port "
"from the RTCP port\n");
}
}
if (!(h->flags & AVIO_FLAG_NONBLOCK)) {
ret = ff_network_wait_fd(fd, 1);
if (ret < 0)
return ret;
}
ret = sendto(fd, buf, size, 0, (struct sockaddr *) source,
*source_len);
return ret < 0 ? ff_neterrno() : ret;
}
if (RTP_PT_IS_RTCP(buf[1])) {
hd = s->rtcp_hd;
} else {
hd = s->rtp_hd;
}
if ((ret = ffurl_write(hd, buf, size)) < 0) {
return ret;
}
if (s->fec_hd && !RTP_PT_IS_RTCP(buf[1])) {
if ((ret_fec = ffurl_write(s->fec_hd, buf, size)) < 0) {
av_log(h, AV_LOG_ERROR, "Failed to send FEC\n");
return ret_fec;
}
}
return ret;
}
static int rtp_close(URLContext *h)
{
RTPContext *s = h->priv_data;
ff_ip_reset_filters(&s->filters);
ffurl_close(s->rtp_hd);
ffurl_close(s->rtcp_hd);
ffurl_closep(&s->fec_hd);
return 0;
}
int ff_rtp_get_local_rtp_port(URLContext *h)
{
RTPContext *s = h->priv_data;
return ff_udp_get_local_port(s->rtp_hd);
}
static int rtp_get_file_handle(URLContext *h)
{
RTPContext *s = h->priv_data;
return s->rtp_fd;
}
static int rtp_get_multi_file_handle(URLContext *h, int **handles,
int *numhandles)
{
RTPContext *s = h->priv_data;
int *hs = *handles = av_malloc(sizeof(**handles) * 2);
if (!hs)
return AVERROR(ENOMEM);
hs[0] = s->rtp_fd;
hs[1] = s->rtcp_fd;
*numhandles = 2;
return 0;
}
const URLProtocol ff_rtp_protocol = {
.name = "rtp",
.url_open = rtp_open,
.url_read = rtp_read,
.url_write = rtp_write,
.url_close = rtp_close,
.url_get_file_handle = rtp_get_file_handle,
.url_get_multi_file_handle = rtp_get_multi_file_handle,
.priv_data_size = sizeof(RTPContext),
.flags = URL_PROTOCOL_FLAG_NETWORK,
.priv_data_class = &rtp_class,
};
