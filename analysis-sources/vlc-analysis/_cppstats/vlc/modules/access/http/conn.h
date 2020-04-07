struct vlc_tls;
struct vlc_http_conn;
struct vlc_http_msg;
struct vlc_http_stream;
struct vlc_http_conn_cbs
{
struct vlc_http_stream *(*stream_open)(struct vlc_http_conn *,
const struct vlc_http_msg *);
void (*release)(struct vlc_http_conn *);
};
struct vlc_http_conn
{
const struct vlc_http_conn_cbs *cbs;
struct vlc_tls *tls;
};
static inline struct vlc_http_stream *
vlc_http_stream_open(struct vlc_http_conn *conn, const struct vlc_http_msg *m)
{
return conn->cbs->stream_open(conn, m);
}
static inline void vlc_http_conn_release(struct vlc_http_conn *conn)
{
conn->cbs->release(conn);
}
void vlc_http_err(void *, const char *msg, ...) VLC_FORMAT(2, 3);
void vlc_http_dbg(void *, const char *msg, ...) VLC_FORMAT(2, 3);
struct vlc_http_conn *vlc_h1_conn_create(void *ctx, struct vlc_tls *,
bool proxy);
struct vlc_http_stream *vlc_chunked_open(struct vlc_http_stream *,
struct vlc_tls *);
struct vlc_http_stream *vlc_h1_request(void *ctx, const char *hostname,
unsigned port, bool proxy,
const struct vlc_http_msg *req,
bool idempotent,
struct vlc_http_conn **restrict connp);
struct vlc_http_conn *vlc_h2_conn_create(void *ctx, struct vlc_tls *);
