



#include "nvim/tui/input.h"
#include "nvim/vim.h"
#include "nvim/api/vim.h"
#include "nvim/api/private/helpers.h"
#include "nvim/ascii.h"
#include "nvim/charset.h"
#include "nvim/main.h"
#include "nvim/aucmd.h"
#include "nvim/ex_docmd.h"
#include "nvim/option.h"
#include "nvim/os/os.h"
#include "nvim/os/input.h"
#if defined(WIN32)
#include "nvim/os/os_win_console.h"
#endif
#include "nvim/event/rstream.h"

#define KEY_BUFFER_SIZE 0xfff

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "tui/input.c.generated.h"
#endif

void tinput_init(TermInput *input, Loop *loop)
{
input->loop = loop;
input->paste = 0;
input->in_fd = STDIN_FILENO;
input->waiting_for_bg_response = 0;


input->ttimeout = (bool)p_ttimeout;
input->ttimeoutlen = p_ttm;
input->key_buffer = rbuffer_new(KEY_BUFFER_SIZE);
uv_mutex_init(&input->key_buffer_mutex);
uv_cond_init(&input->key_buffer_cond);




#if defined(WIN32)
if (!os_isatty(input->in_fd)) {
input->in_fd = os_get_conin_fd();
}
#else
if (!os_isatty(input->in_fd) && os_isatty(STDERR_FILENO)) {
input->in_fd = STDERR_FILENO;
}
#endif
input_global_fd_init(input->in_fd);

const char *term = os_getenv("TERM");
if (!term) {
term = ""; 
}

#if TERMKEY_VERSION_MAJOR > 0 || TERMKEY_VERSION_MINOR > 18
input->tk = termkey_new_abstract(term,
TERMKEY_FLAG_UTF8 | TERMKEY_FLAG_NOSTART);
termkey_hook_terminfo_getstr(input->tk, input->tk_ti_hook_fn, NULL);
termkey_start(input->tk);
#else
input->tk = termkey_new_abstract(term, TERMKEY_FLAG_UTF8);
#endif

int curflags = termkey_get_canonflags(input->tk);
termkey_set_canonflags(input->tk, curflags | TERMKEY_CANON_DELBS);


rstream_init_fd(loop, &input->read_stream, input->in_fd, 0xfff);

time_watcher_init(loop, &input->timer_handle, input);
}

void tinput_destroy(TermInput *input)
{
rbuffer_free(input->key_buffer);
uv_mutex_destroy(&input->key_buffer_mutex);
uv_cond_destroy(&input->key_buffer_cond);
time_watcher_close(&input->timer_handle, NULL);
stream_close(&input->read_stream, NULL, NULL);
termkey_destroy(input->tk);
}

void tinput_start(TermInput *input)
{
rstream_start(&input->read_stream, tinput_read_cb, input);
}

void tinput_stop(TermInput *input)
{
rstream_stop(&input->read_stream);
time_watcher_stop(&input->timer_handle);
}

static void tinput_done_event(void **argv)
{
input_done();
}

static void tinput_wait_enqueue(void **argv)
{
TermInput *input = argv[0];
RBUFFER_UNTIL_EMPTY(input->key_buffer, buf, len) {
const String keys = { .data = buf, .size = len };
if (input->paste) {
String copy = copy_string(keys);
multiqueue_put(main_loop.events, tinput_paste_event, 3,
copy.data, copy.size, (intptr_t)input->paste);
if (input->paste == 1) {

input->paste = 2;
}
rbuffer_consumed(input->key_buffer, len);
rbuffer_reset(input->key_buffer);
} else {
const size_t consumed = input_enqueue(keys);
if (consumed) {
rbuffer_consumed(input->key_buffer, consumed);
}
rbuffer_reset(input->key_buffer);
if (consumed < len) {
break;
}
}
}
uv_mutex_lock(&input->key_buffer_mutex);
input->waiting = false;
uv_cond_signal(&input->key_buffer_cond);
uv_mutex_unlock(&input->key_buffer_mutex);
}

static void tinput_paste_event(void **argv)
{
String keys = { .data = argv[0], .size = (size_t)argv[1] };
intptr_t phase = (intptr_t)argv[2];

Error err = ERROR_INIT;
nvim_paste(keys, true, phase, &err);
if (ERROR_SET(&err)) {
emsgf("paste: %s", err.msg);
api_clear_error(&err);
}

api_free_string(keys);
}

static void tinput_flush(TermInput *input, bool wait_until_empty)
{
size_t drain_boundary = wait_until_empty ? 0 : 0xff;
do {
uv_mutex_lock(&input->key_buffer_mutex);
loop_schedule_fast(&main_loop, event_create(tinput_wait_enqueue, 1, input));
input->waiting = true;
while (input->waiting) {
uv_cond_wait(&input->key_buffer_cond, &input->key_buffer_mutex);
}
uv_mutex_unlock(&input->key_buffer_mutex);
} while (rbuffer_size(input->key_buffer) > drain_boundary);
}

static void tinput_enqueue(TermInput *input, char *buf, size_t size)
{
if (rbuffer_size(input->key_buffer) >
rbuffer_capacity(input->key_buffer) - 0xff) {


tinput_flush(input, false);
}
rbuffer_write(input->key_buffer, buf, size);
}

static void forward_simple_utf8(TermInput *input, TermKeyKey *key)
{
size_t len = 0;
char buf[64];
char *ptr = key->utf8;

while (*ptr) {
if (*ptr == '<') {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "<lt>");
} else {
buf[len++] = *ptr;
}
ptr++;
}

tinput_enqueue(input, buf, len);
}

static void forward_modified_utf8(TermInput *input, TermKeyKey *key)
{
size_t len;
char buf[64];

if (key->type == TERMKEY_TYPE_KEYSYM
&& key->code.sym == TERMKEY_SYM_ESCAPE) {
len = (size_t)snprintf(buf, sizeof(buf), "<Esc>");
} else if (key->type == TERMKEY_TYPE_KEYSYM
&& key->code.sym == TERMKEY_SYM_SUSPEND) {
len = (size_t)snprintf(buf, sizeof(buf), "<C-Z>");
} else {
len = termkey_strfkey(input->tk, buf, sizeof(buf), key, TERMKEY_FORMAT_VIM);
}

tinput_enqueue(input, buf, len);
}

static void forward_mouse_event(TermInput *input, TermKeyKey *key)
{
char buf[64];
size_t len = 0;
int button, row, col;
static int last_pressed_button = 0;
TermKeyMouseEvent ev;
termkey_interpret_mouse(input->tk, key, &ev, &button, &row, &col);

if ((ev == TERMKEY_MOUSE_RELEASE || ev == TERMKEY_MOUSE_DRAG)
&& button == 0) {




button = last_pressed_button;
}

if (button == 0 || (ev != TERMKEY_MOUSE_PRESS && ev != TERMKEY_MOUSE_DRAG
&& ev != TERMKEY_MOUSE_RELEASE)) {
return;
}

row--; col--; 
buf[len++] = '<';

if (key->modifiers & TERMKEY_KEYMOD_SHIFT) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "S-");
}

if (key->modifiers & TERMKEY_KEYMOD_CTRL) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "C-");
}

if (key->modifiers & TERMKEY_KEYMOD_ALT) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "A-");
}

if (button == 1) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "Left");
} else if (button == 2) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "Middle");
} else if (button == 3) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "Right");
}

switch (ev) {
case TERMKEY_MOUSE_PRESS:
if (button == 4) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "ScrollWheelUp");
} else if (button == 5) {
len += (size_t)snprintf(buf + len, sizeof(buf) - len,
"ScrollWheelDown");
} else {
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "Mouse");
last_pressed_button = button;
}
break;
case TERMKEY_MOUSE_DRAG:
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "Drag");
break;
case TERMKEY_MOUSE_RELEASE:
len += (size_t)snprintf(buf + len, sizeof(buf) - len, "Release");
break;
case TERMKEY_MOUSE_UNKNOWN:
assert(false);
}

len += (size_t)snprintf(buf + len, sizeof(buf) - len, "><%d,%d>", col, row);
tinput_enqueue(input, buf, len);
}

static TermKeyResult tk_getkey(TermKey *tk, TermKeyKey *key, bool force)
{
return force ? termkey_getkey_force(tk, key) : termkey_getkey(tk, key);
}

static void tinput_timer_cb(TimeWatcher *watcher, void *data);

static void tk_getkeys(TermInput *input, bool force)
{
TermKeyKey key;
TermKeyResult result;

while ((result = tk_getkey(input->tk, &key, force)) == TERMKEY_RES_KEY) {
if (key.type == TERMKEY_TYPE_UNICODE && !key.modifiers) {
forward_simple_utf8(input, &key);
} else if (key.type == TERMKEY_TYPE_UNICODE
|| key.type == TERMKEY_TYPE_FUNCTION
|| key.type == TERMKEY_TYPE_KEYSYM) {
forward_modified_utf8(input, &key);
} else if (key.type == TERMKEY_TYPE_MOUSE) {
forward_mouse_event(input, &key);
}
}

if (result != TERMKEY_RES_AGAIN) {
return;
}




if (input->ttimeout && input->ttimeoutlen >= 0) {

time_watcher_stop(&input->timer_handle);
time_watcher_start(&input->timer_handle, tinput_timer_cb,
(uint64_t)input->ttimeoutlen, 0);
} else {
tk_getkeys(input, true);
}
}

static void tinput_timer_cb(TimeWatcher *watcher, void *data)
{
tk_getkeys(data, true);
tinput_flush(data, true);
}









static bool handle_focus_event(TermInput *input)
{
if (rbuffer_size(input->read_stream.buffer) > 2
&& (!rbuffer_cmp(input->read_stream.buffer, "\x1b[I", 3)
|| !rbuffer_cmp(input->read_stream.buffer, "\x1b[O", 3))) {
bool focus_gained = *rbuffer_get(input->read_stream.buffer, 2) == 'I';

rbuffer_consumed(input->read_stream.buffer, 3);
aucmd_schedule_focusgained(focus_gained);
return true;
}
return false;
}

static bool handle_bracketed_paste(TermInput *input)
{
if (rbuffer_size(input->read_stream.buffer) > 5
&& (!rbuffer_cmp(input->read_stream.buffer, "\x1b[200~", 6)
|| !rbuffer_cmp(input->read_stream.buffer, "\x1b[201~", 6))) {
bool enable = *rbuffer_get(input->read_stream.buffer, 4) == '0';
if (input->paste && enable) {
return false; 
}

rbuffer_consumed(input->read_stream.buffer, 6);
if (!!input->paste == enable) {
return true; 
}

if (enable) {

tinput_flush(input, true);

input->paste = 1;
} else if (input->paste) {

input->paste = input->paste == 2 ? 3 : -1;
tinput_flush(input, true);

input->paste = 0;
}
return true;
}
return false;
}


static bool handle_forced_escape(TermInput *input)
{
if (rbuffer_size(input->read_stream.buffer) > 1
&& !rbuffer_cmp(input->read_stream.buffer, "\x1b\x00", 2)) {

size_t rcnt;
termkey_push_bytes(input->tk, rbuffer_read_ptr(input->read_stream.buffer,
&rcnt), 1);
rbuffer_consumed(input->read_stream.buffer, 2);
tk_getkeys(input, true);
return true;
}
return false;
}

static void set_bg_deferred(void **argv)
{
char *bgvalue = argv[0];
if (!option_was_set("bg") && !strequal((char *)p_bg, bgvalue)) {

if (starting) {

do_cmdline_cmd((bgvalue[0] == 'l')
? "autocmd VimEnter * ++once ++nested set bg=light"
: "autocmd VimEnter * ++once ++nested set bg=dark");
} else {
set_option_value("bg", 0L, bgvalue, 0);
reset_option_was_set("bg");
}
}
}












static bool handle_background_color(TermInput *input)
{
if (input->waiting_for_bg_response <= 0) {
return false;
}
size_t count = 0;
size_t component = 0;
size_t header_size = 0;
size_t num_components = 0;
uint16_t rgb[] = { 0, 0, 0 };
uint16_t rgb_max[] = { 0, 0, 0 };
bool eat_backslash = false;
bool done = false;
bool bad = false;
if (rbuffer_size(input->read_stream.buffer) >= 9
&& !rbuffer_cmp(input->read_stream.buffer, "\x1b]11;rgb:", 9)) {
header_size = 9;
num_components = 3;
} else if (rbuffer_size(input->read_stream.buffer) >= 10
&& !rbuffer_cmp(input->read_stream.buffer, "\x1b]11;rgba:", 10)) {
header_size = 10;
num_components = 4;
} else {
input->waiting_for_bg_response--;
if (input->waiting_for_bg_response == 0) {
DLOG("did not get a response for terminal background query");
}
return false;
}
input->waiting_for_bg_response = 0;
rbuffer_consumed(input->read_stream.buffer, header_size);
RBUFFER_EACH(input->read_stream.buffer, c, i) {
count = i + 1;
if (eat_backslash) {
done = true;
break;
} else if (c == '\x07') {
done = true;
break;
} else if (c == '\x1b') {
eat_backslash = true;
} else if (bad) {

} else if ((c == '/') && (++component < num_components)) {

} else if (ascii_isxdigit(c)) {
if (component < 3 && rgb_max[component] != 0xffff) {
rgb_max[component] = (uint16_t)((rgb_max[component] << 4) | 0xf);
rgb[component] = (uint16_t)((rgb[component] << 4) | hex2nr(c));
}
} else {
bad = true;
}
}
rbuffer_consumed(input->read_stream.buffer, count);
if (done && !bad && rgb_max[0] && rgb_max[1] && rgb_max[2]) {
double r = (double)rgb[0] / (double)rgb_max[0];
double g = (double)rgb[1] / (double)rgb_max[1];
double b = (double)rgb[2] / (double)rgb_max[2];
double luminance = (0.299 * r) + (0.587 * g) + (0.114 * b); 
char *bgvalue = luminance < 0.5 ? "dark" : "light";
DLOG("bg response: %s", bgvalue);
loop_schedule_deferred(&main_loop,
event_create(set_bg_deferred, 1, bgvalue));
} else {
DLOG("failed to parse bg response");
return false;
}
return true;
}
#if defined(UNIT_TESTING)
bool ut_handle_background_color(TermInput *input)
{
return handle_background_color(input);
}
#endif

static void tinput_read_cb(Stream *stream, RBuffer *buf, size_t count_,
void *data, bool eof)
{
TermInput *input = data;

if (eof) {
loop_schedule_fast(&main_loop, event_create(tinput_done_event, 0));
return;
}

do {
if (handle_focus_event(input)
|| handle_bracketed_paste(input)
|| handle_forced_escape(input)
|| handle_background_color(input)) {
continue;
}






size_t count = 0;
RBUFFER_EACH(input->read_stream.buffer, c, i) {
count = i + 1;
if (c == '\x1b' && count > 1) {
count--;
break;
}
}

if (input->paste) {
RBUFFER_UNTIL_EMPTY(input->read_stream.buffer, ptr, len) {
size_t consumed = MIN(count, len);
assert(consumed <= input->read_stream.buffer->size);
tinput_enqueue(input, ptr, consumed);
rbuffer_consumed(input->read_stream.buffer, consumed);
if (!(count -= consumed)) {
break;
}
}
continue;
}

RBUFFER_UNTIL_EMPTY(input->read_stream.buffer, ptr, len) {
size_t consumed = termkey_push_bytes(input->tk, ptr, MIN(count, len));



assert(consumed <= input->read_stream.buffer->size);
rbuffer_consumed(input->read_stream.buffer, consumed);


tk_getkeys(input, false);
if (!(count -= consumed)) {
break;
}
}
} while (rbuffer_size(input->read_stream.buffer));
tinput_flush(input, true);



rbuffer_reset(input->read_stream.buffer);
}
