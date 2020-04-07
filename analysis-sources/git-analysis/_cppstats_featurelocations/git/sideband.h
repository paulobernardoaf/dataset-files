#if !defined(SIDEBAND_H)
#define SIDEBAND_H

enum sideband_type {
SIDEBAND_PROTOCOL_ERROR = -2,
SIDEBAND_REMOTE_ERROR = -1,
SIDEBAND_FLUSH = 0,
SIDEBAND_PRIMARY = 1
};












int demultiplex_sideband(const char *me, char *buf, int len,
int die_on_error,
struct strbuf *scratch,
enum sideband_type *sideband_type);

void send_sideband(int fd, int band, const char *data, ssize_t sz, int packet_max);

#endif
