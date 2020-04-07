struct tr2_tbuf {
char buf[32];
};
void tr2_tbuf_local_time(struct tr2_tbuf *tb);
void tr2_tbuf_utc_datetime_extended(struct tr2_tbuf *tb);
void tr2_tbuf_utc_datetime(struct tr2_tbuf *tb);
