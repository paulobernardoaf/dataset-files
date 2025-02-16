


#include <r_cons.h>
#include <r_core.h>
#include <string.h>
#include <stdlib.h>

#if __WINDOWS__
#include <windows.h>
#define printf(...) r_cons_win_printf (false, __VA_ARGS__)
#define USE_UTF8 1
static int r_line_readchar_win(ut8 *s, int slen);
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#define USE_UTF8 1
#endif

static char *r_line_nullstr = "";
static const char word_break_characters[] = "\t\n ~`!@#$%^&*()-_=+[]{}\\|;:\"'<>,./";


typedef enum {
MINOR_BREAK,
MAJOR_BREAK
} BreakMode;

bool enable_yank_pop = false;

static inline bool is_word_break_char(char ch, bool mode) {
int i;
if (mode == MAJOR_BREAK) {
return ch == ' ';
}
int len =
sizeof (word_break_characters) /
sizeof (word_break_characters[0]);
for (i = 0; i < len; i++) {
if (ch == word_break_characters[i]) {
return true;
}
}
return false;
}


static void backward_kill_word() {
int i, len;
if (I.buffer.index > 0) {
for (i = I.buffer.index; i > 0 && is_word_break_char (I.buffer.data[i], MINOR_BREAK); i--) {

}
for (; i > 0 && !is_word_break_char (I.buffer.data[i], MINOR_BREAK); i--) {

}
if (i > 0) {
i++;
} else if (i < 0) {
i = 0;
}
if (I.buffer.index > I.buffer.length) {
I.buffer.length = I.buffer.index;
}
len = I.buffer.index - i + 1;
free (I.clipboard);
I.clipboard = r_str_ndup (I.buffer.data + i, len);
r_line_clipboard_push (I.clipboard);
memmove (I.buffer.data + i, I.buffer.data + I.buffer.index,
I.buffer.length - I.buffer.index + 1);
I.buffer.length = strlen (I.buffer.data);
I.buffer.index = i;
}
}

static void backward_kill_Word() {
int i, len;
if (I.buffer.index > 0) {
for (i = I.buffer.index; i > 0 && is_word_break_char (I.buffer.data[i], MAJOR_BREAK); i--) {

}
for (; i > 0 && !is_word_break_char (I.buffer.data[i], MAJOR_BREAK); i--) {

}
if (i > 0) {
i++;
} else if (i < 0) {
i = 0;
}
if (I.buffer.index > I.buffer.length) {
I.buffer.length = I.buffer.index;
}
len = I.buffer.index - i + 1;
free (I.clipboard);
I.clipboard = r_str_ndup (I.buffer.data + i, len);
r_line_clipboard_push (I.clipboard);
memmove (I.buffer.data + i, I.buffer.data + I.buffer.index,
I.buffer.length - I.buffer.index + 1);
I.buffer.length = strlen (I.buffer.data);
I.buffer.index = i;
}
}

static void kill_word() {
int i, len;
for (i = I.buffer.index; i < I.buffer.length && is_word_break_char (I.buffer.data[i], MINOR_BREAK); i++) {

}
for (; i < I.buffer.length && !is_word_break_char (I.buffer.data[i], MINOR_BREAK); i++) {

}
if (I.buffer.index >= I.buffer.length) {
I.buffer.length = I.buffer.index;
}
len = i - I.buffer.index + 1;
free (I.clipboard);
I.clipboard = r_str_ndup (I.buffer.data + I.buffer.index, len);
r_line_clipboard_push (I.clipboard);
memmove (I.buffer.data + I.buffer.index, I.buffer.data + i, len);
I.buffer.length = strlen (I.buffer.data);
}

static void kill_Word() {
int i, len;
for (i = I.buffer.index; i < I.buffer.length && is_word_break_char (I.buffer.data[i], MAJOR_BREAK); i++) {

}
for (; i < I.buffer.length && !is_word_break_char (I.buffer.data[i], MAJOR_BREAK); i++) {

}
if (I.buffer.index >= I.buffer.length) {
I.buffer.length = I.buffer.index;
}
len = i - I.buffer.index + 1;
free (I.clipboard);
I.clipboard = r_str_ndup (I.buffer.data + I.buffer.index, len);
r_line_clipboard_push (I.clipboard);
memmove (I.buffer.data + I.buffer.index, I.buffer.data + i, len);
I.buffer.length = strlen (I.buffer.data);
}

static void paste() {
if (I.clipboard) {
char *cursor = I.buffer.data + I.buffer.index;
int dist = (I.buffer.data + I.buffer.length) - cursor;
int len = strlen (I.clipboard);
I.buffer.length += len;
memmove (cursor + len, cursor, dist);
memcpy (cursor, I.clipboard, len);
I.buffer.index += len;
enable_yank_pop = true;
}
}

static void unix_word_rubout() {
int i, len;
if (I.buffer.index > 0) {
for (i = I.buffer.index - 1; i > 0 && I.buffer.data[i] == ' '; i--) {


}
for (; i > 0 && I.buffer.data[i] != ' '; i--) {


}
if (i > 0) {
i++;
} else if (i < 0) {
i = 0;
}
if (I.buffer.index > I.buffer.length) {
I.buffer.length = I.buffer.index;
}
len = I.buffer.index - i + 1;
free (I.clipboard);
I.clipboard = r_str_ndup (I.buffer.data + i, len);
r_line_clipboard_push (I.clipboard);
memmove (I.buffer.data + i,
I.buffer.data + I.buffer.index,
I.buffer.length - I.buffer.index + 1);
I.buffer.length = strlen (I.buffer.data);
I.buffer.index = i;
}
}

static int inithist() {
ZERO_FILL (I.history);
if ((I.history.size + 1024) * sizeof (char *) < I.history.size) {
return false;
}
I.history.data = (char **) calloc ((I.history.size + 1024), sizeof(char *));
if (!I.history.data) {
return false;
}
I.history.size = R_LINE_HISTSIZE;
return true;
}


R_API int r_line_dietline_init() {
ZERO_FILL (I.completion);
if (!inithist ()) {
return false;
}
I.echo = true;
return true;
}

#if USE_UTF8

static int r_line_readchar_utf8(ut8 *s, int slen) {
#if __WINDOWS__
return r_line_readchar_win (s, slen);
#else

ssize_t len, i;
if (slen < 1) {
return 0;
}
int ch = r_cons_readchar ();
if (ch == -1) {
return -1;
}
*s = ch;
#if 0
if ((t = read (0, s, 1)) != 1) {
return t;
}
#endif
*s = r_cons_controlz (*s);
if (*s < 0x80) {
len = 1;
} else if ((s[0] & 0xe0) == 0xc0) {
len = 2;
} else if ((s[0] & 0xf0) == 0xe0) {
len = 3;
} else if ((s[0] & 0xf8) == 0xf0) {
len = 4;
} else {
return -1;
}
if (len > slen) {
return -1;
}
for (i = 1; i < len; i++) {
int ch = r_cons_readchar ();
if (ch != -1) {
s[i] = ch;
}
if ((s[i] & 0xc0) != 0x80) {
return -1;
}
}
return len;
#endif
}
#endif

#if __WINDOWS__
static int r_line_readchar_win(ut8 *s, int slen) { 
INPUT_RECORD irInBuf;
BOOL ret, bCtrl = FALSE;
DWORD mode, out;
char buf[5] = {0};
HANDLE h;
void *bed;

if (I.zerosep) {
bed = r_cons_sleep_begin ();
int rsz = read (0, s, 1);
r_cons_sleep_end (bed);
if (rsz != 1) {
return 0;
}
return 1;
}

h = GetStdHandle (STD_INPUT_HANDLE);
GetConsoleMode (h, &mode);
SetConsoleMode (h, 0); 
do_it_again:
bed = r_cons_sleep_begin ();
ret = ReadConsoleInput (h, &irInBuf, 1, &out);
r_cons_sleep_end (bed);
if (ret < 1) {
return 0;
}
if (irInBuf.EventType == KEY_EVENT) {
if (irInBuf.Event.KeyEvent.bKeyDown) {
if (irInBuf.Event.KeyEvent.uChar.UnicodeChar) {
char *tmp = r_sys_conv_win_to_utf8_l ((PTCHAR)&irInBuf.Event.KeyEvent.uChar, 1);
if (!tmp) {
return 0;
}
strncpy_s (buf, sizeof (buf), tmp, strlen (tmp));
free (tmp);
} else {
int idx = 0;
buf[idx++] = 27;
buf[idx++] = '['; 
bCtrl = irInBuf.Event.KeyEvent.dwControlKeyState & 8;
if (bCtrl) {
buf[idx++] = 0x31;
}
switch (irInBuf.Event.KeyEvent.wVirtualKeyCode) {
case VK_UP: buf[idx++] = 'A'; break;
case VK_DOWN: buf[idx++] = 'B'; break;
case VK_RIGHT: buf[idx++] = 'C'; break;
case VK_LEFT: buf[idx++] = 'D'; break;
case VK_PRIOR: buf[idx++] = '5'; break; 
case VK_NEXT: buf[idx++] = '6'; break; 
case VK_DELETE: buf[idx++] = '3'; break; 
case VK_HOME: buf[idx++] = 'H'; break; 
case VK_END: buf[idx++] = 'F'; break; 
default: buf[0] = 0; break;
}
}
}
}
if (!buf[0]) {
goto do_it_again;
}
strncpy_s ((char *)s, slen, buf, sizeof (buf));
SetConsoleMode (h, mode);
return strlen ((char *)s);
}

#endif

R_API int r_line_set_hist_callback(RLine *line, RLineHistoryUpCb up, RLineHistoryDownCb down) {
line->cb_history_up = up;
line->cb_history_down = down;
line->offset_hist_index = 0;
line->file_hist_index = 0;
line->sdbshell_hist_iter = r_list_head (line->sdbshell_hist);
return 1;
}

R_API int r_line_hist_cmd_up(RLine *line) {
if (line->hist_up) {
return line->hist_up (line->user);
}
if (!line->history.data) {
inithist ();
}
if (line->history.index > 0 && line->history.data) {
strncpy (line->buffer.data, line->history.data[--line->history.index], R_LINE_BUFSIZE - 1);
line->buffer.index = line->buffer.length = strlen (line->buffer.data);
return true;
}
return false;
}

R_API int r_line_hist_cmd_down(RLine *line) {
if (line->hist_down) {
return line->hist_down (line->user);
}
line->buffer.index = 0;
if (!line->history.data) {
inithist ();
}
if (line->history.index == line->history.top) {
return false;
}
line->history.index++;
if (line->history.index == line->history.top) {
line->buffer.data[0] = '\0';
line->buffer.index = line->buffer.length = 0;
return false;
}
if (line->history.data && line->history.data[line->history.index]) {
strncpy (line->buffer.data, line->history.data[line->history.index], R_LINE_BUFSIZE - 1);
line->buffer.index = line->buffer.length = strlen (line->buffer.data);
}
return true;
}

R_API int r_line_hist_add(const char *line) {
if (!line || !*line) {
return false;
}
if (!I.history.data) {
inithist ();
}

if (I.history.top > 0) {
const char *data = I.history.data[I.history.top - 1];
if (data && !strcmp (line, data)) {
I.history.index = I.history.top;
return false;
}
}
if (I.history.top == I.history.size) {
int i;
free (I.history.data[0]);
for (i = 0; i <= I.history.size - 2; i++) {
I.history.data[i] = I.history.data[i + 1];
}
I.history.top--;
}
I.history.data[I.history.top++] = strdup (line);
I.history.index = I.history.top;
return true;
}

static int r_line_hist_up() {
if (!I.cb_history_up) {
r_line_set_hist_callback (&I, &r_line_hist_cmd_up, &r_line_hist_cmd_down);
}
return I.cb_history_up (&I);
}

static int r_line_hist_down() {
if (!I.cb_history_down) {
r_line_set_hist_callback (&I, &r_line_hist_cmd_up, &r_line_hist_cmd_down);
}
return I.cb_history_down (&I);
}

R_API const char *r_line_hist_get(int n) {
int i = 0;
if (!I.history.data) {
inithist ();
}
n--;
if (I.history.data) {
for (i = 0; i < I.history.size && I.history.data[i]; i++) {
if (n == i) {
return I.history.data[i];
}
}
}
return NULL;
}

R_API int r_line_hist_list() {
int i = 0;
if (!I.history.data) {
inithist ();
}
if (I.history.data) {
for (i = 0; i < I.history.size && I.history.data[i]; i++) {
const char *pad = r_str_pad (' ', 32 - strlen (I.history.data[i]));
r_cons_printf ("%s %s #!%d\n", I.history.data[i], pad, i);
}
}
return i;
}

R_API void r_line_hist_free() {
int i;
if (I.history.data) {
for (i = 0; i < I.history.size; i++) {
R_FREE (I.history.data[i]);
}
}
R_FREE (I.history.data);
R_FREE (I.sdbshell_hist);
I.history.index = 0;
}


R_API int r_line_hist_load(const char *file) {
FILE *fd;
char buf[R_LINE_BUFSIZE], *path = r_str_home (file);
if (!path) {
return false;
}
if (!(fd = r_sandbox_fopen (path, "r"))) {
free (path);
return false;
}
while (fgets (buf, sizeof (buf), fd) != NULL) {
r_str_trim_tail (buf);
r_line_hist_add (buf);
}
fclose (fd);
free (path);
return true;
}

R_API int r_line_hist_save(const char *file) {
FILE *fd;
int i, ret = false;
if (!file || !*file) {
return false;
}
char *p, *path = r_str_home (file);
if (path != NULL) {
p = (char *) r_str_lastbut (path, R_SYS_DIR[0], NULL); 
if (p) {
*p = 0;
if (!r_sys_mkdirp (path)) {
eprintf ("could not save history into %s\n", path);
goto end;
}
*p = R_SYS_DIR[0];
}
fd = r_sandbox_fopen (path, "w");
if (fd != NULL) {
if (I.history.data) {
for (i = 0; i < I.history.index; i++) {
fputs (I.history.data[i], fd);
fputs ("\n", fd);
}
fclose (fd);
ret = true;
} else {
fclose (fd);
}
}
}
end:
free (path);
return ret;
}

R_API int r_line_hist_chop(const char *file, int limit) {

return 0;
}

static void selection_widget_draw() {
RCons *cons = r_cons_singleton ();
RSelWidget *sel_widget = I.sel_widget;
int y, pos_y, pos_x = r_str_ansi_len (I.prompt);
sel_widget->h = R_MIN (sel_widget->h, R_SELWIDGET_MAXH);
for (y = 0; y < sel_widget->options_len; y++) {
sel_widget->w = R_MAX (sel_widget->w, strlen (sel_widget->options[y]));
}
if (sel_widget->direction == R_SELWIDGET_DIR_UP) {
pos_y = cons->rows;
} else {
pos_y = r_cons_get_cur_line ();
if (pos_y + sel_widget->h > cons->rows) {
printf ("%s\n", r_str_pad('\n', sel_widget->h));
pos_y = cons->rows - sel_widget->h - 1;
}
}
sel_widget->w = R_MIN (sel_widget->w, R_SELWIDGET_MAXW);

char *background_color = cons->context->color_mode ? cons->context->pal.widget_bg : Color_INVERT_RESET;
char *selected_color = cons->context->color_mode ? cons->context->pal.widget_sel : Color_INVERT;
bool scrollbar = sel_widget->options_len > R_SELWIDGET_MAXH;
int scrollbar_y = 0, scrollbar_l = 0;
if (scrollbar) {
scrollbar_y = (R_SELWIDGET_MAXH * (sel_widget->selection - sel_widget->scroll)) / sel_widget->options_len;
scrollbar_l = (R_SELWIDGET_MAXH * R_SELWIDGET_MAXH) / sel_widget->options_len;
}

for (y = 0; y < sel_widget->h; y++) {
if (sel_widget->direction == R_SELWIDGET_DIR_UP) {
r_cons_gotoxy (pos_x + 1, pos_y - y - 1);
} else {
r_cons_gotoxy (pos_x + 1, pos_y + y + 1);
}
int scroll = R_MAX (0, sel_widget->selection - sel_widget->scroll);
const char *option = y < sel_widget->options_len ? sel_widget->options[y + scroll] : "";
r_cons_printf ("%s", sel_widget->selection == y + scroll ? selected_color : background_color);
r_cons_printf ("%-*.*s", sel_widget->w, sel_widget->w, option);
if (scrollbar && R_BETWEEN (scrollbar_y, y, scrollbar_y + scrollbar_l)) {
r_cons_memcat (Color_INVERT" "Color_INVERT_RESET, 10);
} else {
r_cons_memcat (" ", 1);
}
}

r_cons_gotoxy (pos_x + I.buffer.length, pos_y);
r_cons_memcat (Color_RESET_BG, 5);
r_cons_flush ();
}

static void selection_widget_up(int steps) {
RSelWidget *sel_widget = I.sel_widget;
if (sel_widget) {
if (sel_widget->direction == R_SELWIDGET_DIR_UP) {
int height = R_MIN (sel_widget->h, R_SELWIDGET_MAXH - 1);
sel_widget->selection = R_MIN (sel_widget->selection + steps, sel_widget->options_len - 1);
if (steps == 1) {
sel_widget->scroll = R_MIN (sel_widget->scroll + 1, R_SELWIDGET_MAXH - 1);
} else if (sel_widget->selection + (height - sel_widget->scroll) > sel_widget->options_len - 1) {
sel_widget->scroll = height - (sel_widget->options_len - 1 - sel_widget->selection);
}
} else {
sel_widget->selection = R_MAX (sel_widget->selection - steps, 0);
if (steps == 1) {
sel_widget->scroll = R_MAX (sel_widget->scroll - 1, 0);
} else if (sel_widget->selection - sel_widget->scroll <= 0) {
sel_widget->scroll = sel_widget->selection;
}
}
}
}

static void selection_widget_down(int steps) {
RSelWidget *sel_widget = I.sel_widget;
if (sel_widget) {
if (sel_widget->direction == R_SELWIDGET_DIR_UP) {
sel_widget->selection = R_MAX (sel_widget->selection - steps, 0);
if (steps == 1) {
sel_widget->scroll = R_MAX (sel_widget->scroll - 1, 0);
} else if (sel_widget->selection - sel_widget->scroll <= 0) {
sel_widget->scroll = sel_widget->selection;
}
} else {
int height = R_MIN (sel_widget->h, R_SELWIDGET_MAXH - 1);
sel_widget->selection = R_MIN (sel_widget->selection + steps, sel_widget->options_len - 1);
if (steps == 1) {
sel_widget->scroll = R_MIN (sel_widget->scroll + 1, R_SELWIDGET_MAXH - 1);
} else if (sel_widget->selection + (height - sel_widget->scroll) > sel_widget->options_len - 1) {
sel_widget->scroll = height - (sel_widget->options_len - 1 - sel_widget->selection);
}
}
}
}

static void print_rline_task(void *core) {
r_cons_clear_line (0);
r_cons_printf ("%s%s%s", Color_RESET, I.prompt, I.buffer.data);
r_cons_flush ();
}

static void selection_widget_erase() {
RSelWidget *sel_widget = I.sel_widget;
if (sel_widget) {
sel_widget->options_len = 0;
sel_widget->selection = -1;
selection_widget_draw ();
R_FREE (I.sel_widget);
RCons *cons = r_cons_singleton ();
if (cons->event_resize && cons->event_data) {
cons->event_resize (cons->event_data);
RCore *core = (RCore *)(cons->user);
if (core) {
cons->cb_task_oneshot (&core->tasks, print_rline_task, core);
}
}
printf ("%s", R_CONS_CLEAR_FROM_CURSOR_TO_END);
}
}

static void selection_widget_select() {
RSelWidget *sel_widget = I.sel_widget;
if (sel_widget && sel_widget->selection < sel_widget->options_len) {
char *sp = strchr (I.buffer.data, ' ');
if (sp) {
int delta = sp - I.buffer.data + 1;
I.buffer.length = R_MIN (delta + strlen (sel_widget->options[sel_widget->selection]), R_LINE_BUFSIZE - 1);
memcpy (I.buffer.data + delta, sel_widget->options[sel_widget->selection], strlen (sel_widget->options[sel_widget->selection]));
I.buffer.index = I.buffer.length;
return;
}
I.buffer.length = R_MIN (strlen (sel_widget->options[sel_widget->selection]), R_LINE_BUFSIZE - 1);
memcpy (I.buffer.data, sel_widget->options[sel_widget->selection], I.buffer.length);
I.buffer.data[I.buffer.length] = '\0';
I.buffer.index = I.buffer.length;
selection_widget_erase ();
}
}

static void selection_widget_update() {
int argc = r_pvector_len (&I.completion.args);
const char **argv = (const char **)r_pvector_data (&I.completion.args);
if (argc == 0 || (argc == 1 && I.buffer.length >= strlen (argv[0]))) {
selection_widget_erase ();
return;
}
if (!I.sel_widget) {
RSelWidget *sel_widget = R_NEW0 (RSelWidget);
I.sel_widget = sel_widget;
}
I.sel_widget->scroll = 0;
I.sel_widget->selection = 0;
I.sel_widget->options_len = argc;
I.sel_widget->options = argv;
I.sel_widget->h = R_MAX (I.sel_widget->h, I.sel_widget->options_len);

if (I.prompt_type == R_LINE_PROMPT_DEFAULT) {
I.sel_widget->direction = R_SELWIDGET_DIR_DOWN;
} else {
I.sel_widget->direction = R_SELWIDGET_DIR_UP;
}
selection_widget_draw ();
r_cons_flush ();
return;
}

R_API void r_line_autocomplete() {
char *p;
const char **argv = NULL;
int argc = 0, i, j, plen, len = 0;
bool opt = false;
int cols = (int)(r_cons_get_size (NULL) * 0.82);
RCons *cons = r_cons_singleton ();


if (I.completion.run) {
I.completion.opt = false;
I.completion.run (&I.completion, &I.buffer, I.prompt_type, I.completion.run_user);
argc = r_pvector_len (&I.completion.args);
argv = (const char **)r_pvector_data (&I.completion.args);
opt = I.completion.opt;
}
if (I.sel_widget && !I.sel_widget->complete_common) {
selection_widget_update ();
return;
}

if (opt) {
p = (char *) r_sub_str_lchr (I.buffer.data, 0, I.buffer.index, '=');
} else {
p = (char *) r_sub_str_lchr (I.buffer.data, 0, I.buffer.index, ' ');
}
if (!p) {
p = (char *) r_sub_str_lchr (I.buffer.data, 0, I.buffer.index, '@'); 
}
if (p) {
p++;
plen = sizeof (I.buffer.data) - (int) (size_t) (p - I.buffer.data);
} else {
p = I.buffer.data; 
plen = sizeof (I.buffer.data);
}

if (argc == 1) {
const char *end_word = r_sub_str_rchr (I.buffer.data,
I.buffer.index, strlen (I.buffer.data), ' ');
const char *t = end_word != NULL?
end_word: I.buffer.data + I.buffer.index;
int largv0 = strlen (argv[0]? argv[0]: "");
size_t len_t = strlen (t);
p[largv0]='\0';

if ((p - I.buffer.data) + largv0 + 1 + len_t < plen) {
if (len_t > 0) {
int tt = largv0;
if (*t != ' ') {
p[tt++] = ' ';
}
memmove (p + tt, t, len_t);
}
memcpy (p, argv[0], largv0);

if (p[largv0 - 1] != R_SYS_DIR[0]) {
p[largv0] = ' ';
if (!len_t) {
p[largv0 + 1] = '\0';
}
}
I.buffer.length = strlen (I.buffer.data);
I.buffer.index = I.buffer.length;
}
} else if (argc > 0) {
if (*p) {

const char *t = I.buffer.data + I.buffer.index;
const char *root = argv[0];
int min_common_len = strlen (root);
size_t len_t = strlen (t);


for (i = 0; i < argc; i++) {
j = 0;
if (!argv[i]) {
break;
}
while (argv[i][j] == root[j] && root[j] != '\0') j++;
if (j < min_common_len) {
min_common_len = j;
}
root = argv[i];
}
if (len_t > 0) {
int tt = min_common_len;
memmove (p + tt, t, len_t);
p[tt + len_t] = '\0';
}
memmove (p, root, min_common_len);
if (!len_t) {
p[min_common_len] = '\0';
}
I.buffer.length = strlen (I.buffer.data);
I.buffer.index = (p - I.buffer.data) + min_common_len;
}
}

if (I.prompt_type != R_LINE_PROMPT_DEFAULT || cons->show_autocomplete_widget) {
selection_widget_update ();
if (I.sel_widget) {
I.sel_widget->complete_common = false;
}
return;
}


if (argc > 1 && I.echo) {
const int sep = 3;
int slen, col = 10;
#if defined(__WINDOWS__)
r_cons_win_printf (false, "%s%s\n", I.prompt, I.buffer.data);
#else
printf ("%s%s\n", I.prompt, I.buffer.data);
#endif
for (i = 0; i < argc && argv[i]; i++) {
int l = strlen (argv[i]);
if (sep + l > col) {
col = sep + l;
}
if (col > (cols >> 1)) {
col = (cols >> 1);
break;
}
}
for (len = i = 0; i < argc && argv[i]; i++) {
if (len + col > cols) {
printf ("\n");
len = 0;
}
printf ("%-*s ", col - sep, argv[i]);
slen = strlen (argv[i]);
len += (slen > col)? (slen + sep): (col + sep);
}
printf ("\n");
}
fflush (stdout);
}

R_API const char *r_line_readline() {
return r_line_readline_cb (NULL, NULL);
}

static inline void rotate_kill_ring() {
if (enable_yank_pop) {
I.buffer.index -= strlen (r_list_get_n (I.kill_ring, I.kill_ring_ptr));
I.buffer.data[I.buffer.index] = 0;
I.kill_ring_ptr -= 1;
if (I.kill_ring_ptr < 0) {
I.kill_ring_ptr = I.kill_ring->length - 1;
}
I.clipboard = r_list_get_n (I.kill_ring, I.kill_ring_ptr);
paste ();
}
}

static inline void __delete_next_char() {
if (I.buffer.index < I.buffer.length) {
int len = r_str_utf8_charsize (I.buffer.data + I.buffer.index);
memmove (I.buffer.data + I.buffer.index,
I.buffer.data + I.buffer.index + len,
strlen (I.buffer.data + I.buffer.index + 1) + 1);
I.buffer.length -= len;
}
}

static inline void __delete_prev_char() {
if (I.buffer.index < I.buffer.length) {
if (I.buffer.index > 0) {
int len = r_str_utf8_charsize_prev (I.buffer.data + I.buffer.index, I.buffer.index);
I.buffer.index -= len;
memmove (I.buffer.data + I.buffer.index,
I.buffer.data + I.buffer.index + len,
strlen (I.buffer.data + I.buffer.index));
I.buffer.length -= len;
}
} else {
I.buffer.length -= r_str_utf8_charsize_last (I.buffer.data);
I.buffer.index = I.buffer.length;
if (I.buffer.length < 0) {
I.buffer.length = 0;
}
}
I.buffer.data[I.buffer.length] = '\0';
if (I.buffer.index < 0) {
I.buffer.index = 0;
}
}

static inline void delete_till_end() {
I.buffer.data[I.buffer.index] = '\0';
I.buffer.length = I.buffer.index;
I.buffer.index = I.buffer.index > 0 ? I.buffer.index - 1 : 0;
}

static void __print_prompt() {
RCons *cons = r_cons_singleton ();
int columns = r_cons_get_size (NULL) - 2;
int chars = R_MAX (1, strlen (I.buffer.data));
int len, i, cols = R_MAX (1, columns - r_str_ansi_len (I.prompt) - 2);
if (cons->line->prompt_type == R_LINE_PROMPT_OFFSET) {
r_cons_gotoxy (0, cons->rows);
r_cons_flush ();
}
r_cons_clear_line (0);
printf ("\r%s%s", Color_RESET, I.prompt);
fwrite (I.buffer.data, 1, R_MIN (cols, chars), stdout);
printf ("\r%s", I.prompt);
if (I.buffer.index > cols) {
printf ("< ");
i = I.buffer.index - cols;
if (i > sizeof (I.buffer.data)) {
i = sizeof (I.buffer.data) - 1;
}
} else {
i = 0;
}
len = I.buffer.index - i;
if (len > 0 && (i + len) <= I.buffer.length) {
fwrite (I.buffer.data + i, 1, len, stdout);
}
fflush (stdout);
}

static inline void __move_cursor_right() {
I.buffer.index = I.buffer.index < I.buffer.length
? I.buffer.index + r_str_utf8_charsize (I.buffer.data + I.buffer.index)
: I.buffer.length;
}

static inline void __move_cursor_left() {
I.buffer.index = I.buffer.index
? I.buffer.index - r_str_utf8_charsize_prev (I.buffer.data + I.buffer.index, I.buffer.index)
: 0;
}

static inline void vi_cmd_b() {
int i;
for (i = I.buffer.index - 2; i >= 0; i--) {
if ((is_word_break_char (I.buffer.data[i], MINOR_BREAK)
&& !is_word_break_char (I.buffer.data[i], MAJOR_BREAK))
|| (is_word_break_char (I.buffer.data[i - 1], MINOR_BREAK)
&& !is_word_break_char (I.buffer.data[i], MINOR_BREAK))) {
I.buffer.index = i;
break;
}
}
if (i < 0) {
I.buffer.index = 0;
}
}

static inline void vi_cmd_B() {
int i;
for (i = I.buffer.index - 2; i >= 0; i--) {
if ((!is_word_break_char (I.buffer.data[i], MAJOR_BREAK)
&& is_word_break_char (I.buffer.data[i-1], MAJOR_BREAK))) {
I.buffer.index = i;
break;
}
}
if (i < 0) {
I.buffer.index = 0;
}
}

static inline void vi_cmd_W() {
int i;
for (i = I.buffer.index + 1; i < I.buffer.length; i++) {
if ((!is_word_break_char (I.buffer.data[i], MAJOR_BREAK)
&& is_word_break_char (I.buffer.data[i-1], MAJOR_BREAK))) {
I.buffer.index = i;
break;
}
}
if (i >= I.buffer.length) {
I.buffer.index = I.buffer.length - 1;
}
}

static inline void vi_cmd_w() {
int i;
for (i = I.buffer.index + 1; i < I.buffer.length; i++) {
if ((!is_word_break_char (I.buffer.data[i], MINOR_BREAK)
&& is_word_break_char (I.buffer.data[i - 1], MINOR_BREAK))
|| (is_word_break_char (I.buffer.data[i], MINOR_BREAK)
&& !is_word_break_char (I.buffer.data[i], MAJOR_BREAK))) {
I.buffer.index = i;
break;
}
}
if (i >= I.buffer.length) {
I.buffer.index = I.buffer.length - 1;
}
}

static inline void vi_cmd_E() {
int i;
for (i = I.buffer.index + 1; i < I.buffer.length; i++) {
if ((!is_word_break_char (I.buffer.data[i], MAJOR_BREAK)
&& is_word_break_char (I.buffer.data[i+1], MAJOR_BREAK))) {
I.buffer.index = i;
break;
}
}
if (i >= I.buffer.length) {
I.buffer.index = I.buffer.length - 1;
}
}

static inline void vi_cmd_e() {
int i;
for (i = I.buffer.index + 1; i < I.buffer.length; i++) {
if ((!is_word_break_char (I.buffer.data[i], MINOR_BREAK)
&& is_word_break_char (I.buffer.data[i+1], MINOR_BREAK))
|| (is_word_break_char (I.buffer.data[i], MINOR_BREAK)
&& !is_word_break_char (I.buffer.data[i], MAJOR_BREAK))) {
I.buffer.index = i;
break;
}
}
if (i >= I.buffer.length) {
I.buffer.index = I.buffer.length - 1;
}
}

static void __update_prompt_color () {
RCons *cons = r_cons_singleton ();
const char *BEGIN = "", *END = "";
if (cons->context->color_mode) {
if (I.prompt_mode) {
switch (I.vi_mode) {
case CONTROL_MODE:
BEGIN = cons->context->pal.invalid;
break;
case INSERT_MODE:
default:
BEGIN = cons->context->pal.prompt;
break;
}
} else {
BEGIN = cons->context->pal.prompt;
}
END = cons->context->pal.reset;
}
char *prompt = r_str_escape (I.prompt); 
free (I.prompt);
I.prompt = r_str_newf ("%s%s%s", BEGIN, prompt, END);
}

static void __vi_mode() {
char ch;
I.vi_mode = CONTROL_MODE;
__update_prompt_color ();
const char *gcomp_line = "";
static int gcomp = 0;
for (;;) {
int rep = 0;
if (I.echo) {
__print_prompt ();
}
if (I.vi_mode != CONTROL_MODE) { 
__update_prompt_color ();
break;
}
ch = r_cons_readchar ();
while (IS_DIGIT (ch)) { 
if (ch == '0' && rep == 0) { 
break;
}
int tmp = ch - '0';
rep = (rep * 10) + tmp;
ch = r_cons_readchar ();
}
rep = rep > 0 ? rep : 1;

switch (ch) {
case 3:
if (I.hud) {
I.hud->activate = false;
I.hud->current_entry_n = -1;
}
if (I.echo) {
eprintf ("^C\n");
}
I.buffer.index = I.buffer.length = 0;
*I.buffer.data = '\0';
gcomp = 0;
return;
case 'D':
delete_till_end ();
break;
case 'r': {
char c = r_cons_readchar ();
I.buffer.data[I.buffer.index] = c;
} break;
case 'x':
while (rep--) {
__delete_next_char ();
} break;
case 'c':
I.vi_mode = INSERT_MODE; 
case 'd': {
char c = r_cons_readchar ();
while (rep--) {
switch (c) {
case 'i': {
char t = r_cons_readchar ();
if (t == 'w') { 
kill_word ();
backward_kill_word ();
} else if (t == 'W') { 
kill_Word ();
backward_kill_word ();
}
if (I.hud) {
I.hud->vi = false;
}
} break;
case 'W':
kill_Word ();
break;
case 'w':
kill_word ();
break;
case 'B':
backward_kill_Word ();
break;
case 'b':
backward_kill_word ();
break;
case 'h':
__delete_prev_char ();
break;
case 'l':
__delete_next_char ();
break;
case '$':
delete_till_end ();
break;
case '^':
case '0':
strncpy (I.buffer.data, I.buffer.data + I.buffer.index, I.buffer.length);
I.buffer.length -= I.buffer.index;
I.buffer.index = 0;
break;
} __print_prompt ();
}
} break;
case 'I':
if (I.hud) {
I.hud->vi = false;
}
I.vi_mode = INSERT_MODE;

case '^':
case '0':
if (gcomp) {
strcpy (I.buffer.data, gcomp_line);
I.buffer.length = strlen (I.buffer.data);
I.buffer.index = 0;
gcomp = false;
}
I.buffer.index = 0;
break;
case 'A':
I.vi_mode = INSERT_MODE;

case '$':
if (gcomp) {
strcpy (I.buffer.data, gcomp_line);
I.buffer.index = strlen (I.buffer.data);
I.buffer.length = I.buffer.index;
gcomp = false;
} else {
I.buffer.index = I.buffer.length;
} break;
case 'p':
while (rep--) {
paste ();
} break;
case 'a':
__move_cursor_right ();
case 'i':
I.vi_mode = INSERT_MODE;
if (I.hud) {
I.hud->vi = false;
}
break;
case 'h':
while (rep--) {
__move_cursor_left ();
} break;
case 'l':
while (rep--) {
__move_cursor_right ();
} break;
case 'E':
while (rep--) {
vi_cmd_E ();
} break;
case 'e':
while (rep--) {
vi_cmd_e ();
} break;
case 'B':
while (rep--) {
vi_cmd_B ();
} break;
case 'b':
while (rep--) {
vi_cmd_b ();
} break;
case 'W':
while (rep--) {
vi_cmd_W ();
} break;
case 'w':
while (rep--) {
vi_cmd_w ();
} break;
default: 
ch = tolower (r_cons_arrow_to_hjkl (ch));
switch (ch) {
case 'k': 
r_line_hist_up ();
break;
case 'j': 
r_line_hist_down ();
break;
case 'l': 
__move_cursor_right ();
break;
case 'h': 
__move_cursor_left ();
break;
}
break;
}
if (I.hud) {
return;
}
}
}

R_API const char *r_line_readline_cb(RLineReadCallback cb, void *user) {
int rows, columns = r_cons_get_size (&rows) - 2;
const char *gcomp_line = "";
static int gcomp_idx = 0;
static bool yank_flag = 0;
static int gcomp = 0;
signed char buf[10];
#if USE_UTF8
int utflen;
#endif
int ch, key, i = 0; 
char *tmp_ed_cmd, prev = 0;
int prev_buflen = -1;
RCons *cons = r_cons_singleton ();

if (!I.hud || (I.hud && !I.hud->activate)) {
I.buffer.index = I.buffer.length = 0;
I.buffer.data[0] = '\0';
if (I.hud) {
I.hud->activate = true;
}
}
int mouse_status = cons->mouse;
if (I.hud && I.hud->vi) {
__vi_mode ();
goto _end;
}
if (I.contents) {
memmove (I.buffer.data, I.contents,
R_MIN (strlen (I.contents) + 1, R_LINE_BUFSIZE - 1));
I.buffer.data[R_LINE_BUFSIZE - 1] = '\0';
I.buffer.index = I.buffer.length = strlen (I.contents);
}
if (I.disable) {
if (!fgets (I.buffer.data, R_LINE_BUFSIZE, stdin)) {
return NULL;
}
return (*I.buffer.data)? I.buffer.data: r_line_nullstr;
}

memset (&buf, 0, sizeof buf);
r_cons_set_raw (1);

if (I.echo) {
__print_prompt ();
}
r_cons_break_push (NULL, NULL);
r_cons_enable_mouse (I.hud);
for (;;) {
yank_flag = 0;
if (r_cons_is_breaked ()) {
break;
}
I.buffer.data[I.buffer.length] = '\0';
if (cb) {
int cbret = cb (user, I.buffer.data);
if (cbret == 0) {
I.buffer.data[0] = 0;
I.buffer.length = 0;
}
}
#if USE_UTF8
utflen = r_line_readchar_utf8 ((ut8 *) buf, sizeof (buf));
if (utflen < 1) {
r_cons_break_pop ();
return NULL;
}
buf[utflen] = 0;
#else
#if __WINDOWS__
{
int len = r_line_readchar_win ((ut8 *)buf, sizeof (buf));
if (len < 1) {
r_cons_break_pop ();
return NULL;
}
buf[len] = 0;
}
#else
ch = r_cons_readchar ();
if (ch == -1) {
r_cons_break_pop ();
return NULL;
}
buf[0] = ch;
#endif
#endif
if (I.echo) {
r_cons_clear_line (0);
}
if (columns < 1) {
columns = 40;
}
#if __WINDOWS__
if (I.echo) {
printf ("\r%*c\r", columns, ' ');
}
#else
if (I.echo) {
printf ("\r\x1b[2K\r"); 
}
#endif
switch (*buf) {
case 0: 

break;
case 1: 
if (gcomp) {
strcpy (I.buffer.data, gcomp_line);
I.buffer.length = strlen (I.buffer.data);
I.buffer.index = 0;
gcomp = false;
}
I.buffer.index = 0;
break;
case 2: 
__move_cursor_left ();
break;
case 5: 
if (gcomp) {
strcpy (I.buffer.data, gcomp_line);
I.buffer.index = strlen (I.buffer.data);
I.buffer.length = I.buffer.index;
gcomp = false;
} else if (prev == 24) {
I.buffer.data[I.buffer.length] = 0; 
tmp_ed_cmd = I.cb_editor (I.user, I.buffer.data);
if (tmp_ed_cmd) {

I.buffer.length = strlen (tmp_ed_cmd);
if (I.buffer.length < R_LINE_BUFSIZE) {
I.buffer.index = I.buffer.length;
strncpy (I.buffer.data, tmp_ed_cmd, R_LINE_BUFSIZE - 1);
I.buffer.data[R_LINE_BUFSIZE - 1] = '\0';
} else {
I.buffer.length -= strlen (tmp_ed_cmd);
}
free (tmp_ed_cmd);
}
} else {
I.buffer.index = I.buffer.length;
}
break;
case 3: 
if (I.hud) {
I.hud->activate = false;
I.hud->current_entry_n = -1;
}
if (I.echo) {
eprintf ("^C\n");
}
I.buffer.index = I.buffer.length = 0;
*I.buffer.data = '\0';
gcomp = 0;
goto _end;
case 4: 
if (!I.buffer.data[0]) {
if (I.echo) {
printf ("^D\n");
}
r_cons_set_raw (false);
r_cons_break_pop ();
return NULL;
}
if (I.buffer.index < I.buffer.length) {
__delete_next_char ();
}
break;
case 10:
r_cons_break_pop ();
return I.buffer.data;
case 11:
I.buffer.data[I.buffer.index] = '\0';
I.buffer.length = I.buffer.index;
break;
case 6: 
__move_cursor_right ();
break;
case 12:
__move_cursor_right ();
if (I.echo) {
eprintf ("\x1b[2J\x1b[0;0H");
}
fflush (stdout);
break;
case 18:
if (gcomp) {
gcomp_idx++;
}
gcomp = 1;
break;
case 19:
if (gcomp) {
gcomp--;
} else {
__move_cursor_left ();
}
break;
case 21:
free (I.clipboard);
I.clipboard = strdup (I.buffer.data);
r_line_clipboard_push (I.clipboard);
I.buffer.data[0] = '\0';
I.buffer.length = 0;
I.buffer.index = 0;
break;
#if __WINDOWS__
case 22:
{
HANDLE hClipBoard;
PTCHAR clipText;
if (OpenClipboard (NULL)) {
#if UNICODE
hClipBoard = GetClipboardData (CF_UNICODETEXT);
#else
hClipBoard = GetClipboardData (CF_TEXT);
#endif
if (hClipBoard) {
clipText = GlobalLock (hClipBoard);
if (clipText) {
char *txt = r_sys_conv_win_to_utf8 (clipText);
if (!txt) {
R_LOG_ERROR ("Failed to allocate memory\n");
break;
}
int len = strlen (txt);
I.buffer.length += len;
if (I.buffer.length < R_LINE_BUFSIZE) {
I.buffer.index = I.buffer.length;
strcat (I.buffer.data, txt);
} else {
I.buffer.length -= len;
}
free (txt);
}
GlobalUnlock (hClipBoard);
}
CloseClipboard ();
}
}
break;
#endif
case 23:
unix_word_rubout ();
break;
case 24:
if (I.buffer.index > 0) {
strncpy (I.buffer.data, I.buffer.data + I.buffer.index, I.buffer.length);
I.buffer.length -= I.buffer.index;
I.buffer.index = 0;
}
break;
case 25:
paste ();
yank_flag = 1;
break;
case 29: 
rotate_kill_ring ();
yank_flag = enable_yank_pop ? 1 : 0;
break;
case 20: 
kill_word ();
break;
case 15: 
backward_kill_word ();
break;
case 14:
if (I.hud) {
if (I.hud->top_entry_n + 1 < I.hud->current_entry_n) {
I.hud->top_entry_n++;
}
} else if (I.sel_widget) {
selection_widget_down (1);
selection_widget_draw ();
} else if (gcomp) {
if (gcomp_idx > 0) {
gcomp_idx--;
}
} else {
r_line_hist_down ();
}
break;
case 16:
if (I.hud) {
if (I.hud->top_entry_n >= 0) {
I.hud->top_entry_n--;
}
} else if (I.sel_widget) {
selection_widget_up (1);
selection_widget_draw ();
} else if (gcomp) {
gcomp_idx++;
} else {
r_line_hist_up ();
}
break;
case 27: 
#if __WINDOWS__
memmove (buf, buf + 1, strlen (buf));
if (!buf[0]) {
buf[0] = -1;
}
#else
buf[0] = r_cons_readchar_timeout (50);
#endif
switch (buf[0]) {
case 127: 
backward_kill_word ();
break;
case -1: 
if (I.enable_vi_mode) {
if (I.hud) {
I.hud->vi = true;
}
__vi_mode ();
};
if (I.sel_widget) {
selection_widget_erase ();
}
break;
case 1: 
I.buffer.index = 0;
break;
case 5: 
I.buffer.index = I.buffer.length;
break;
case 'B':
case 'b':
for (i = I.buffer.index - 2; i >= 0; i--) {
if (is_word_break_char (I.buffer.data[i], MINOR_BREAK)
&& !is_word_break_char (I.buffer.data[i + 1], MINOR_BREAK)) {
I.buffer.index = i + 1;
break;
}
}
if (i < 0) {
I.buffer.index = 0;
}
break;
case 'D':
case 'd':
kill_word ();
break;
case 'F':
case 'f':

for (i = I.buffer.index + 1; i < I.buffer.length; i++) {
if (!is_word_break_char (I.buffer.data[i], MINOR_BREAK)
&& is_word_break_char (I.buffer.data[i - 1], MINOR_BREAK)) {
I.buffer.index = i;
break;
}
}
if (i >= I.buffer.length) {
I.buffer.index = I.buffer.length;
}
break;
default:
#if !__WINDOWS__
buf[1] = r_cons_readchar_timeout (50);
if (buf[1] == -1) {
r_cons_break_pop ();
return NULL;
}
#endif
if (buf[0] == 0x5b) { 
switch (buf[1]) {
case '3': 
__delete_next_char ();
#if !__WINDOWS__
buf[1] = r_cons_readchar ();
if (buf[1] == -1) {
r_cons_break_pop ();
return NULL;
}
#endif
break;
case '5': 
#if !__WINDOWS__
buf[1] = r_cons_readchar ();
#endif
if (I.hud) {
I.hud->top_entry_n -= (rows - 1);
if (I.hud->top_entry_n < 0) {
I.hud->top_entry_n = 0;
}
}
if (I.sel_widget) {
selection_widget_up (R_MIN (I.sel_widget->h, R_SELWIDGET_MAXH));
selection_widget_draw ();
}
break;
case '6': 
#if !__WINDOWS__
buf[1] = r_cons_readchar ();
#endif
if (I.hud) {
I.hud->top_entry_n += (rows - 1);
if (I.hud->top_entry_n >= I.hud->current_entry_n) {
I.hud->top_entry_n = I.hud->current_entry_n - 1;
}
}
if (I.sel_widget) {
selection_widget_down (R_MIN (I.sel_widget->h, R_SELWIDGET_MAXH));
selection_widget_draw ();
}
break;
case '9': 
key = r_cons_readchar ();
cons->mouse_event = 1;
if (key == '6') { 
if (I.hud && I.hud->top_entry_n + 1 < I.hud->current_entry_n) {
I.hud->top_entry_n--;
}
} else if (key == '7') { 
if (I.hud && I.hud->top_entry_n >= 0) {
I.hud->top_entry_n++;
}
}
while (r_cons_readchar () != 'M') {}
break;

case 'A': 
if (I.hud) {
if (I.hud->top_entry_n > 0) {
I.hud->top_entry_n--;
}
} else if (I.sel_widget) {
selection_widget_up (1);
selection_widget_draw ();
} else if (gcomp) {
gcomp_idx++;
} else if (r_line_hist_up () == -1) {
r_cons_break_pop ();
return NULL;
}
break;
case 'B': 
if (I.hud) {
if (I.hud->top_entry_n + 1 < I.hud->current_entry_n) {
I.hud->top_entry_n++;
}
} else if (I.sel_widget) {
selection_widget_down (1);
selection_widget_draw ();
} else if (gcomp) {
if (gcomp_idx > 0) {
gcomp_idx--;
}
} else if (r_line_hist_down () == -1) {
r_cons_break_pop ();
return NULL;
}
break;
case 'C': 
__move_cursor_right ();
break;
case 'D': 
__move_cursor_left ();
break;
case 0x31: 
#if __WINDOWS__
ch = buf[2];
#else
ch = r_cons_readchar ();
if (ch == 0x7e) { 

I.buffer.index = 0;
break;
}
r_cons_readchar ();
int fkey = ch - '0';
#endif
switch (ch) {
case 0x41:

I.buffer.index = 0;
break;
case 0x44:

for (i = I.buffer.index; i > 0; i--) {
if (I.buffer.data[i] == ' ') {
I.buffer.index = i - 1;
break;
}
}
if (I.buffer.data[i] != ' ') {
I.buffer.index = 0;
}
break;
case 0x42:

I.buffer.index = I.buffer.length;
break;
case 0x43:

for (i = I.buffer.index; i < I.buffer.length; i++) {
if (I.buffer.data[i] == ' ') {
I.buffer.index = i + 1;
break;
}
}
if (I.buffer.data[i] != ' ') {
I.buffer.index = I.buffer.length;
}
break;
default:
#if __WINDOWS__

#else
{
if (I.cb_fkey) {
I.cb_fkey (I.user, fkey);
}
}
#endif
break;
}
r_cons_set_raw (1);
break;
case 0x37: 
r_cons_readchar ();
case 0x48: 
if (I.sel_widget) {
selection_widget_up (I.sel_widget->options_len - 1);
selection_widget_draw ();
break;
}
I.buffer.index = 0;
break;
case 0x34: 
case 0x38: 
r_cons_readchar ();
case 0x46: 
if (I.sel_widget) {
selection_widget_down (I.sel_widget->options_len - 1);
selection_widget_draw ();
break;
}
I.buffer.index = I.buffer.length;
break;
}
}
}
break;
case 8:
case 127:
if (I.hud && (I.buffer.index == 0)) {
I.hud->activate = false;
I.hud->current_entry_n = -1;
}
__delete_prev_char ();
break;
case 9: 
if (I.buffer.length > 0 && I.buffer.data[I.buffer.length - 1] == '@') {
strcpy (I.buffer.data + I.buffer.length, " ");
I.buffer.length++;
I.buffer.index++;
}
if (I.sel_widget) {
selection_widget_down (1);
I.sel_widget->complete_common = true;
selection_widget_draw ();
}
if (I.hud) {
if (I.hud->top_entry_n + 1 < I.hud->current_entry_n) {
I.hud->top_entry_n++;
} else {
I.hud->top_entry_n = 0;
}
} else {
r_line_autocomplete ();
}
break;
case 13: 
if (I.hud) {
I.hud->activate = false;
break;
}
if (I.sel_widget) {
selection_widget_select ();
break;
}
if (gcomp && I.buffer.length > 0) {
strncpy (I.buffer.data, gcomp_line, R_LINE_BUFSIZE - 1);
I.buffer.data[R_LINE_BUFSIZE - 1] = '\0';
I.buffer.length = strlen (gcomp_line);
}
gcomp_idx = gcomp = 0;
goto _end;
default:
if (gcomp) {
gcomp++;
}
{
#if USE_UTF8
int size = utflen;
#else
int size = 1;
#endif
if (I.buffer.length + size >= R_LINE_BUFSIZE) {
break;
}
}
if (I.buffer.index < I.buffer.length) {
#if USE_UTF8
if ((I.buffer.length + utflen) < sizeof (I.buffer.data)) {
I.buffer.length += utflen;
for (i = I.buffer.length; i > I.buffer.index; i--) {
I.buffer.data[i] = I.buffer.data[i - utflen];
}
memcpy (I.buffer.data + I.buffer.index, buf, utflen);
}
#else
for (i = ++I.buffer.length; i > I.buffer.index; i--) {
I.buffer.data[i] = I.buffer.data[i - 1];
}
I.buffer.data[I.buffer.index] = buf[0];
#endif
} else {
#if USE_UTF8
if ((I.buffer.length + utflen) < sizeof (I.buffer.data)) {
memcpy (I.buffer.data + I.buffer.length, buf, utflen);
I.buffer.length += utflen;
}
I.buffer.data[I.buffer.length] = '\0';
#else
I.buffer.data[I.buffer.length] = buf[0];
I.buffer.length++;
if (I.buffer.length > (R_LINE_BUFSIZE - 1)) {
I.buffer.length--;
}
I.buffer.data[I.buffer.length] = '\0';
#endif
}
#if USE_UTF8
if ((I.buffer.index + utflen) <= I.buffer.length) {
I.buffer.index += utflen;
}
#else
if (I.buffer.index < I.buffer.length) {
I.buffer.index++;
}
#endif
break;
}
if (I.sel_widget && I.buffer.length != prev_buflen) {
prev_buflen = I.buffer.length;
r_line_autocomplete ();
}
prev = buf[0];
if (I.echo) {
if (gcomp) {
gcomp_line = "";
int counter = 0;
if (I.history.data != NULL) {
for (i = I.history.size-1; i >= 0; i--) {
if (!I.history.data[i]) {
continue;
}
if (strstr (I.history.data[i], I.buffer.data)) {
gcomp_line = I.history.data[i];
if (++counter > gcomp_idx) {
break;
}
}
if (i == 0) {
gcomp_idx--;
}
}
}
printf ("\r (reverse-i-search (%s)): %s\r", I.buffer.data, gcomp_line);
} else {
__print_prompt ();
}
fflush (stdout);
}
enable_yank_pop = yank_flag ? 1 : 0;
if (I.hud) {
goto _end;
}
}
_end:
r_cons_break_pop ();
r_cons_set_raw (0);
r_cons_enable_mouse (mouse_status);
if (I.echo) {
printf ("\r%s%s\n", I.prompt, I.buffer.data);
fflush (stdout);
}

R_FREE (I.sel_widget);


if (!memcmp (I.buffer.data, "!history", 8)) {

r_line_hist_list ();
return r_line_nullstr;
}
return I.buffer.data[0] != '\0'? I.buffer.data: r_line_nullstr;
}
