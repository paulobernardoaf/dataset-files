


#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "nvim/msgpack_rpc/channel.h"
#include "nvim/msgpack_rpc/server.h"
#include "nvim/os/os.h"
#include "nvim/event/socket.h"
#include "nvim/ascii.h"
#include "nvim/eval.h"
#include "nvim/garray.h"
#include "nvim/vim.h"
#include "nvim/main.h"
#include "nvim/memory.h"
#include "nvim/log.h"
#include "nvim/fileio.h"
#include "nvim/path.h"
#include "nvim/strings.h"

#define MAX_CONNECTIONS 32
#define LISTEN_ADDRESS_ENV_VAR "NVIM_LISTEN_ADDRESS"

static garray_T watchers = GA_EMPTY_INIT_VALUE;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "msgpack_rpc/server.c.generated.h"
#endif


bool server_init(const char *listen_addr)
{
ga_init(&watchers, sizeof(SocketWatcher *), 1);


const char *env_addr = os_getenv(LISTEN_ADDRESS_ENV_VAR);
int rv = listen_addr == NULL ? 1 : server_start(listen_addr);

if (0 != rv) {
rv = env_addr == NULL ? 1 : server_start(env_addr);
if (0 != rv) {
listen_addr = server_address_new();
if (listen_addr == NULL) {
return false;
}
rv = server_start(listen_addr);
xfree((char *)listen_addr);
}
}

return rv == 0;
}


static void close_socket_watcher(SocketWatcher **watcher)
{
socket_watcher_close(*watcher, free_server);
}



static void set_vservername(garray_T *srvs)
{
char *default_server = (srvs->ga_len > 0)
? ((SocketWatcher **)srvs->ga_data)[0]->addr
: NULL;
set_vim_var_string(VV_SEND_SERVER, default_server, -1);
}


void server_teardown(void)
{
GA_DEEP_CLEAR(&watchers, SocketWatcher *, close_socket_watcher);
}









char *server_address_new(void)
{
#if defined(WIN32)
static uint32_t count = 0;
char template[ADDRESS_MAX_SIZE];
snprintf(template, ADDRESS_MAX_SIZE,
"\\\\.\\pipe\\nvim-%" PRIu64 "-%" PRIu32, os_get_pid(), count++);
return xstrdup(template);
#else
return (char *)vim_tempname();
#endif
}



bool server_owns_pipe_address(const char *path)
{
for (int i = 0; i < watchers.ga_len; i++) {
if (!strcmp(path, ((SocketWatcher **)watchers.ga_data)[i]->addr)) {
return true;
}
}
return false;
}














int server_start(const char *endpoint)
{
if (endpoint == NULL || endpoint[0] == '\0') {
WLOG("Empty or NULL endpoint");
return 1;
}

SocketWatcher *watcher = xmalloc(sizeof(SocketWatcher));

int result = socket_watcher_init(&main_loop, watcher, endpoint);
if (result < 0) {
xfree(watcher);
return result;
}


for (int i = 0; i < watchers.ga_len; i++) {
if (!strcmp(watcher->addr, ((SocketWatcher **)watchers.ga_data)[i]->addr)) {
ELOG("Already listening on %s", watcher->addr);
if (watcher->stream->type == UV_TCP) {
uv_freeaddrinfo(watcher->uv.tcp.addrinfo);
}
socket_watcher_close(watcher, free_server);
return 2;
}
}

result = socket_watcher_start(watcher, MAX_CONNECTIONS, connection_cb);
if (result < 0) {
WLOG("Failed to start server: %s: %s", uv_strerror(result), watcher->addr);
socket_watcher_close(watcher, free_server);
return result;
}


const char *listen_address = os_getenv(LISTEN_ADDRESS_ENV_VAR);
if (listen_address == NULL) {
os_setenv(LISTEN_ADDRESS_ENV_VAR, watcher->addr, 1);
}


ga_grow(&watchers, 1);
((SocketWatcher **)watchers.ga_data)[watchers.ga_len++] = watcher;


if (STRLEN(get_vim_var_str(VV_SEND_SERVER)) == 0) {
set_vservername(&watchers);
}

return 0;
}




bool server_stop(char *endpoint)
{
SocketWatcher *watcher;
bool watcher_found = false;
char addr[ADDRESS_MAX_SIZE];


xstrlcpy(addr, endpoint, sizeof(addr));

int i = 0; 
for (; i < watchers.ga_len; i++) {
watcher = ((SocketWatcher **)watchers.ga_data)[i];
if (strcmp(addr, watcher->addr) == 0) {
watcher_found = true;
break;
}
}

if (!watcher_found) {
WLOG("Not listening on %s", addr);
return false;
}


const char *listen_address = os_getenv(LISTEN_ADDRESS_ENV_VAR);
if (listen_address && STRCMP(addr, listen_address) == 0) {
os_unsetenv(LISTEN_ADDRESS_ENV_VAR);
}

socket_watcher_close(watcher, free_server);


if (i != watchers.ga_len - 1) {
((SocketWatcher **)watchers.ga_data)[i] =
((SocketWatcher **)watchers.ga_data)[watchers.ga_len - 1];
}
watchers.ga_len--;


if (STRCMP(addr, get_vim_var_str(VV_SEND_SERVER)) == 0) {
set_vservername(&watchers);
}

return true;
}



char **server_address_list(size_t *size)
FUNC_ATTR_NONNULL_ALL
{
if ((*size = (size_t)watchers.ga_len) == 0) {
return NULL;
}

char **addrs = xcalloc((size_t)watchers.ga_len, sizeof(const char *));
for (int i = 0; i < watchers.ga_len; i++) {
addrs[i] = xstrdup(((SocketWatcher **)watchers.ga_data)[i]->addr);
}
return addrs;
}

static void connection_cb(SocketWatcher *watcher, int result, void *data)
{
if (result) {
ELOG("Failed to accept connection: %s", uv_strerror(result));
return;
}

channel_from_connection(watcher);
}

static void free_server(SocketWatcher *watcher, void *data)
{
xfree(watcher);
}
