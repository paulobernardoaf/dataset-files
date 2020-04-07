#if !defined(NVIM_EVENT_STREAM_H)
#define NVIM_EVENT_STREAM_H

#include <stdbool.h>
#include <stddef.h>

#include <uv.h>

#include "nvim/event/loop.h"
#include "nvim/rbuffer.h"

typedef struct stream Stream;







typedef void (*stream_read_cb)(Stream *stream, RBuffer *buf, size_t count,
void *data, bool eof);







typedef void (*stream_write_cb)(Stream *stream, void *data, int status);
typedef void (*stream_close_cb)(Stream *stream, void *data);

struct stream {
bool closed;
bool did_eof;
union {
uv_pipe_t pipe;
uv_tcp_t tcp;
uv_idle_t idle;
#if defined(WIN32)
uv_tty_t tty;
#endif
} uv;
uv_stream_t *uvstream;
uv_buf_t uvbuf;
RBuffer *buffer;
uv_file fd;
stream_read_cb read_cb;
stream_write_cb write_cb;
void *cb_data;
stream_close_cb close_cb, internal_close_cb;
void *close_cb_data, *internal_data;
size_t fpos;
size_t curmem;
size_t maxmem;
size_t pending_reqs;
size_t num_bytes;
MultiQueue *events;
};

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "event/stream.h.generated.h"
#endif
#endif 
