#if !defined(PKTLINE_H)
#define PKTLINE_H

#include "git-compat-util.h"
#include "strbuf.h"
#include "sideband.h"
















void packet_flush(int fd);
void packet_delim(int fd);
void packet_write_fmt(int fd, const char *fmt, ...) __attribute__((format (printf, 2, 3)));
void packet_buf_flush(struct strbuf *buf);
void packet_buf_delim(struct strbuf *buf);
void set_packet_header(char *buf, int size);
void packet_write(int fd_out, const char *buf, size_t size);
void packet_buf_write(struct strbuf *buf, const char *fmt, ...) __attribute__((format (printf, 2, 3)));
void packet_buf_write_len(struct strbuf *buf, const char *data, size_t len);
int packet_flush_gently(int fd);
int packet_write_fmt_gently(int fd, const char *fmt, ...) __attribute__((format (printf, 2, 3)));
int write_packetized_from_fd(int fd_in, int fd_out);
int write_packetized_from_buf(const char *src_in, size_t len, int fd_out);



































#define PACKET_READ_GENTLE_ON_EOF (1u<<0)
#define PACKET_READ_CHOMP_NEWLINE (1u<<1)
#define PACKET_READ_DIE_ON_ERR_PACKET (1u<<2)
int packet_read(int fd, char **src_buffer, size_t *src_len, char
*buffer, unsigned size, int options);







enum packet_read_status {
PACKET_READ_EOF,
PACKET_READ_NORMAL,
PACKET_READ_FLUSH,
PACKET_READ_DELIM,
};
enum packet_read_status packet_read_with_status(int fd, char **src_buffer,
size_t *src_len, char *buffer,
unsigned size, int *pktlen,
int options);








char *packet_read_line(int fd, int *size);










int packet_read_line_gently(int fd, int *size, char **dst_line);





char *packet_read_line_buf(char **src_buf, size_t *src_len, int *size);




ssize_t read_packetized_to_strbuf(int fd_in, struct strbuf *sb_out);














int recv_sideband(const char *me, int in_stream, int out);

struct packet_reader {

int fd;


char *src_buffer;
size_t src_len;


char *buffer;
unsigned buffer_size;


int options;


enum packet_read_status status;


int pktlen;


const char *line;


int line_peeked;

unsigned use_sideband : 1;
const char *me;
};





void packet_reader_init(struct packet_reader *reader, int fd,
char *src_buffer, size_t src_len,
int options);











enum packet_read_status packet_reader_read(struct packet_reader *reader);









enum packet_read_status packet_reader_peek(struct packet_reader *reader);

#define DEFAULT_PACKET_MAX 1000
#define LARGE_PACKET_MAX 65520
#define LARGE_PACKET_DATA_MAX (LARGE_PACKET_MAX - 4)
extern char packet_buffer[LARGE_PACKET_MAX];

struct packet_writer {
int dest_fd;
unsigned use_sideband : 1;
};

void packet_writer_init(struct packet_writer *writer, int dest_fd);


__attribute__((format (printf, 2, 3)))
void packet_writer_write(struct packet_writer *writer, const char *fmt, ...);
__attribute__((format (printf, 2, 3)))
void packet_writer_error(struct packet_writer *writer, const char *fmt, ...);
void packet_writer_delim(struct packet_writer *writer);
void packet_writer_flush(struct packet_writer *writer);

#endif
