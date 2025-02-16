#include <stdbool.h>
#include <termkey.h>
#include "nvim/event/stream.h"
#include "nvim/event/time.h"
typedef struct term_input {
int in_fd;
int8_t paste;
bool waiting;
bool ttimeout;
int8_t waiting_for_bg_response;
long ttimeoutlen;
TermKey *tk;
#if TERMKEY_VERSION_MAJOR > 0 || TERMKEY_VERSION_MINOR > 18
TermKey_Terminfo_Getstr_Hook *tk_ti_hook_fn; 
#endif
TimeWatcher timer_handle;
Loop *loop;
Stream read_stream;
RBuffer *key_buffer;
uv_mutex_t key_buffer_mutex;
uv_cond_t key_buffer_cond;
} TermInput;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "tui/input.h.generated.h"
#endif
#if defined(UNIT_TESTING)
bool ut_handle_background_color(TermInput *input);
#endif
