



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#undef NDEBUG

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(HAVE_SYS_SOCKET_H)
#include <sys/socket.h>
#endif

#include <vlc_common.h>
#include <vlc_block.h>
#include <vlc_tls.h>
#include "h2frame.h"
#include "conn.h"
#include "message.h"

#if defined(PF_UNIX) && !defined(PF_LOCAL)
#define PF_LOCAL PF_UNIX
#endif

const char vlc_module_name[] = "test_h2conn";

static struct vlc_http_conn *conn;
static struct vlc_tls *external_tls;

static void conn_send(struct vlc_h2_frame *f)
{
assert(f != NULL);

size_t len = vlc_h2_frame_size(f);
ssize_t val = vlc_tls_Write(external_tls, f->data, len);
assert((size_t)val == len);
free(f);
}

enum {
DATA, HEADERS, PRIORITY, RST_STREAM, SETTINGS, PUSH_PROMISE, PING, GOAWAY,
WINDOW_UPDATE, CONTINUATION,
};

static void conn_expect(uint_fast8_t wanted)
{
size_t len;
ssize_t val;
uint8_t hdr[9];
uint8_t got;

do {
val = vlc_tls_Read(external_tls, hdr, 9, true);
assert(val == 9);
assert(hdr[0] == 0);


got = hdr[3];
assert(wanted == got || WINDOW_UPDATE == got);

len = (hdr[1] << 8) | hdr[2];
if (len > 0)
{
char buf[len];

val = vlc_tls_Read(external_tls, buf, len, true);
assert(val == (ssize_t)len);
}
}
while (got != wanted);
}

static void conn_create(void)
{
ssize_t val;
vlc_tls_t *tlsv[2];
char hello[24];

if (vlc_tls_SocketPair(PF_LOCAL, 0, tlsv))
assert(!"socketpair");

external_tls = tlsv[0];

conn = vlc_h2_conn_create(NULL, tlsv[1]);
assert(conn != NULL);
conn_send(vlc_h2_frame_settings());

val = vlc_tls_Read(external_tls, hello, 24, true);
assert(val == 24);
assert(!memcmp(hello, "PRI * HTTP/2.0\r\n", 16));
conn_expect(SETTINGS);
conn_expect(SETTINGS);
}

static void conn_destroy(void)
{
vlc_tls_Shutdown(external_tls, false);
vlc_http_conn_release(conn);
vlc_tls_SessionDelete(external_tls);
}

static struct vlc_http_stream *stream_open(void)
{
struct vlc_http_msg *m = vlc_http_req_create("GET", "https",
"www.example.com", "/");
assert(m != NULL);

struct vlc_http_stream *s = vlc_http_stream_open(conn, m);
vlc_http_msg_destroy(m);
return s;
}

static void stream_reply(uint_fast32_t id, bool nodata)
{
struct vlc_http_msg *m = vlc_http_resp_create(200);
assert(m != NULL);
vlc_http_msg_add_agent(m, "VLC-h2-tester");

conn_send(vlc_http_msg_h2_frame(m, id, nodata));
vlc_http_msg_destroy(m);
}

static void stream_continuation(uint_fast32_t id)
{
const char *h[][2] = {
{ ":status", "100" },
};

conn_send(vlc_h2_frame_headers(id, VLC_H2_DEFAULT_MAX_FRAME, false, 1, h));
}

static void stream_data(uint_fast32_t id, const char *str, bool eos)
{
conn_send(vlc_h2_frame_data(id, str, strlen(str), eos));
}



int main(void)
{
struct vlc_http_stream *s, *s2;
struct vlc_http_msg *m;
struct block_t *b;
uint_fast32_t sid = -1; 

conn_create();
conn_destroy();

conn_create();
conn_send(vlc_h2_frame_ping(42));
conn_expect(PING);


sid += 2;
s = stream_open();
assert(s != NULL);
conn_expect(HEADERS);
conn_send(vlc_h2_frame_rst_stream(sid, VLC_H2_REFUSED_STREAM));
m = vlc_http_stream_read_headers(s);
assert(m == NULL);
b = vlc_http_stream_read(s);
assert(b == vlc_http_error);
vlc_http_stream_close(s, false);
conn_expect(RST_STREAM);


sid += 2;
s = stream_open();
assert(s != NULL);
stream_reply(sid, false);
m = vlc_http_msg_get_initial(s);
assert(m != NULL);
vlc_http_msg_destroy(m);

stream_data(3, "Hello ", false); 
stream_data(3, "world!", true);

conn_expect(HEADERS);
conn_expect(RST_STREAM);
conn_expect(RST_STREAM);
conn_expect(RST_STREAM);


sid += 2;
s = stream_open();
assert(s != NULL);
stream_continuation(sid);
m = vlc_http_msg_get_initial(s);
assert(m != NULL);
assert(vlc_http_msg_get_status(m) == 100);
stream_reply(sid, false);
m = vlc_http_msg_iterate(m);
assert(m != NULL);
stream_data(sid, "Hello ", false);
stream_data(sid, "world!", true);
stream_data(sid, "Stray message", false); 
b = vlc_http_msg_read(m);
assert(b != NULL);
block_Release(b);
b = vlc_http_msg_read(m);
assert(b != NULL);
block_Release(b);
b = vlc_http_msg_read(m);
assert(b == NULL);
vlc_http_msg_destroy(m);

conn_expect(HEADERS);
conn_expect(RST_STREAM);
conn_expect(RST_STREAM);


sid += 2;
s = stream_open();
assert(s != NULL);
stream_continuation(sid);
stream_reply(sid, true);
sid += 2;
s2 = stream_open(); 
assert(s2 != NULL);
stream_reply(sid, true);
m = vlc_http_msg_get_initial(s2);
assert(m != NULL);
vlc_http_msg_destroy(m);
m = vlc_http_msg_get_initial(s);
assert(m != NULL);
assert(vlc_http_msg_get_status(m) == 200);
b = vlc_http_msg_read(m);
assert(b == NULL);
vlc_http_msg_destroy(m);

conn_expect(HEADERS);
conn_expect(HEADERS);
conn_expect(RST_STREAM);
conn_expect(RST_STREAM);


conn_send(vlc_h2_frame_rst_stream(sid + 100, VLC_H2_REFUSED_STREAM));


sid += 2;
s = stream_open();
assert(s != NULL);
sid += 2;
s2 = stream_open();
assert(s2 != NULL);
stream_reply(sid, false);
stream_reply(sid - 2, true);
stream_data(sid, "Discarded", false); 
m = vlc_http_msg_get_initial(s);
assert(m != NULL);
vlc_http_msg_destroy(m);
m = vlc_http_msg_get_initial(s2);
assert(m != NULL);
vlc_http_msg_destroy(m);

conn_expect(HEADERS);
conn_expect(HEADERS);
conn_expect(RST_STREAM);
conn_expect(RST_STREAM);



sid += 2;
s = stream_open();
assert(s != NULL);
conn_send(vlc_h2_frame_goaway(sid - 2, VLC_H2_NO_ERROR));
m = vlc_http_stream_read_headers(s);
assert(m == NULL);


assert(stream_open() == NULL);


conn_destroy();
vlc_http_stream_close(s, false);

return 0;
}
