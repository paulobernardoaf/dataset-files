















#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#elif _MSC_VER
#if !defined(inline)
#define inline __inline
#endif
#endif

struct ipc_pipe_server;
struct ipc_pipe_client;
typedef struct ipc_pipe_server ipc_pipe_server_t;
typedef struct ipc_pipe_client ipc_pipe_client_t;

typedef void (*ipc_pipe_read_t)(void *param, uint8_t *data, size_t size);

bool ipc_pipe_server_start(ipc_pipe_server_t *pipe, const char *name,
ipc_pipe_read_t read_callback, void *param);
void ipc_pipe_server_free(ipc_pipe_server_t *pipe);

bool ipc_pipe_client_open(ipc_pipe_client_t *pipe, const char *name);
void ipc_pipe_client_free(ipc_pipe_client_t *pipe);
bool ipc_pipe_client_write(ipc_pipe_client_t *pipe, const void *data,
size_t size);
static inline bool ipc_pipe_client_valid(ipc_pipe_client_t *pipe);

#if defined(_WIN32)
#include "pipe-windows.h"
#else 
#include "pipe-posix.h"
#endif

#if defined(__cplusplus)
}
#endif
