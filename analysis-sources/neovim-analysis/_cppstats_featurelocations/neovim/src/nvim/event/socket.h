#if !defined(NVIM_EVENT_SOCKET_H)
#define NVIM_EVENT_SOCKET_H

#include <uv.h>

#include "nvim/event/loop.h"
#include "nvim/event/rstream.h"
#include "nvim/event/wstream.h"

#define ADDRESS_MAX_SIZE 256

typedef struct socket_watcher SocketWatcher;
typedef void (*socket_cb)(SocketWatcher *watcher, int result, void *data);
typedef void (*socket_close_cb)(SocketWatcher *watcher, void *data);

struct socket_watcher {

char addr[ADDRESS_MAX_SIZE];

union {
struct {
uv_tcp_t handle;
struct addrinfo *addrinfo;
} tcp;
struct {
uv_pipe_t handle;
} pipe;
} uv;
uv_stream_t *stream;
void *data;
socket_cb cb;
socket_close_cb close_cb;
MultiQueue *events;
};

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "event/socket.h.generated.h"
#endif
#endif 
