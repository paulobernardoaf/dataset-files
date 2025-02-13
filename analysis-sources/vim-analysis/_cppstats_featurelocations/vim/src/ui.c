
















#include "vim.h"

void
ui_write(char_u *s, int len)
{
#if defined(FEAT_GUI)
if (gui.in_use && !gui.dying && !gui.starting)
{
gui_write(s, len);
if (p_wd)
gui_wait_for_chars(p_wd, typebuf.tb_change_cnt);
return;
}
#endif
#if !defined(NO_CONSOLE)

if (!(silent_mode && p_verbose == 0))
{
#if !defined(MSWIN)
char_u *tofree = NULL;

if (output_conv.vc_type != CONV_NONE)
{

tofree = string_convert(&output_conv, s, &len);
if (tofree != NULL)
s = tofree;
}
#endif

mch_write(s, len);

#if !defined(MSWIN)
if (output_conv.vc_type != CONV_NONE)
vim_free(tofree);
#endif
}
#endif
}

#if defined(UNIX) || defined(VMS) || defined(PROTO) || defined(MSWIN)





static char_u *ta_str = NULL;
static int ta_off; 
static int ta_len; 

void
ui_inchar_undo(char_u *s, int len)
{
char_u *new;
int newlen;

newlen = len;
if (ta_str != NULL)
newlen += ta_len - ta_off;
new = alloc(newlen);
if (new != NULL)
{
if (ta_str != NULL)
{
mch_memmove(new, ta_str + ta_off, (size_t)(ta_len - ta_off));
mch_memmove(new + ta_len - ta_off, s, (size_t)len);
vim_free(ta_str);
}
else
mch_memmove(new, s, (size_t)len);
ta_str = new;
ta_len = newlen;
ta_off = 0;
}
}
#endif














int
ui_inchar(
char_u *buf,
int maxlen,
long wtime, 
int tb_change_cnt)
{
int retval = 0;

#if defined(FEAT_GUI) && (defined(UNIX) || defined(VMS))



if (ta_str != NULL)
{
if (maxlen >= ta_len - ta_off)
{
mch_memmove(buf, ta_str + ta_off, (size_t)ta_len);
VIM_CLEAR(ta_str);
return ta_len;
}
mch_memmove(buf, ta_str + ta_off, (size_t)maxlen);
ta_off += maxlen;
return maxlen;
}
#endif

#if defined(FEAT_PROFILE)
if (do_profiling == PROF_YES && wtime != 0)
prof_inchar_enter();
#endif

#if defined(NO_CONSOLE_INPUT)




if (no_console_input())
{
static int count = 0;

#if !defined(NO_CONSOLE)
retval = mch_inchar(buf, maxlen, wtime, tb_change_cnt);
if (retval > 0 || typebuf_changed(tb_change_cnt) || wtime >= 0)
goto theend;
#endif
if (wtime == -1 && ++count == 1000)
read_error_exit();
buf[0] = CAR;
retval = 1;
goto theend;
}
#endif


if (wtime == -1 || wtime > 100L)
{

(void)vim_handle_signal(SIGNAL_UNBLOCK);



if ((mapped_ctrl_c | curbuf->b_mapped_ctrl_c) & get_real_state())
ctrl_c_interrupts = FALSE;
}











































#if defined(FEAT_GUI)
if (gui.in_use)
retval = gui_inchar(buf, maxlen, wtime, tb_change_cnt);
#endif
#if !defined(NO_CONSOLE)
#if defined(FEAT_GUI)
else
#endif
retval = mch_inchar(buf, maxlen, wtime, tb_change_cnt);
#endif

if (wtime == -1 || wtime > 100L)

(void)vim_handle_signal(SIGNAL_BLOCK);

ctrl_c_interrupts = TRUE;

#if defined(NO_CONSOLE_INPUT)
theend:
#endif
#if defined(FEAT_PROFILE)
if (do_profiling == PROF_YES && wtime != 0)
prof_inchar_exit();
#endif
return retval;
}

#if defined(UNIX) || defined(VMS) || defined(FEAT_GUI) || defined(PROTO)













int
inchar_loop(
char_u *buf,
int maxlen,
long wtime, 
int tb_change_cnt,
int (*wait_func)(long wtime, int *interrupted, int ignore_input),
int (*resize_func)(int check_only))
{
int len;
int interrupted = FALSE;
int did_call_wait_func = FALSE;
int did_start_blocking = FALSE;
long wait_time;
long elapsed_time = 0;
#if defined(ELAPSED_FUNC)
elapsed_T start_tv;

ELAPSED_INIT(start_tv);
#endif


for (;;)
{


if (resize_func != NULL)
resize_func(FALSE);

#if defined(MESSAGE_QUEUE)

if (wtime != 0)
{
parse_queued_messages();

if (typebuf_changed(tb_change_cnt))
return 0;
}
#endif
if (wtime < 0 && did_start_blocking)

wait_time = -1;
else
{
if (wtime >= 0)
wait_time = wtime;
else

wait_time = p_ut;
#if defined(ELAPSED_FUNC)
elapsed_time = ELAPSED_FUNC(start_tv);
#endif
wait_time -= elapsed_time;




if (wait_time <= 0 && did_call_wait_func)
{
if (wtime >= 0)

return 0;


did_start_blocking = TRUE;
if (trigger_cursorhold() && maxlen >= 3
&& !typebuf_changed(tb_change_cnt))
{

if (buf == NULL)
{
char_u ibuf[3];

ibuf[0] = CSI;
ibuf[1] = KS_EXTRA;
ibuf[2] = (int)KE_CURSORHOLD;
add_to_input_buf(ibuf, 3);
}
else
{
buf[0] = K_SPECIAL;
buf[1] = KS_EXTRA;
buf[2] = (int)KE_CURSORHOLD;
}
return 3;
}




before_blocking();
continue;
}
}

#if defined(FEAT_JOB_CHANNEL)
if (wait_time < 0 || wait_time > 100L)
{


if (has_pending_job())
wait_time = 100L;



if (channel_any_readahead())
wait_time = 10L;
}
#endif
#if defined(FEAT_BEVAL_GUI)
if (p_beval && wait_time > 100L)


wait_time = 100L;
#endif



did_call_wait_func = TRUE;
if (wait_func(wait_time, &interrupted, FALSE))
{

if (typebuf_changed(tb_change_cnt))
return 0;


if (buf == NULL)


return input_available();

len = read_from_input_buf(buf, (long)maxlen);
if (len > 0)
return len;
continue;
}


#if !defined(ELAPSED_FUNC)

elapsed_time += wait_time;
#endif

if ((resize_func != NULL && resize_func(TRUE))
#if defined(FEAT_CLIENTSERVER) && defined(UNIX)
|| server_waiting()
#endif
#if defined(MESSAGE_QUEUE)
|| interrupted
#endif
|| wait_time > 0
|| (wtime < 0 && !did_start_blocking))

continue;


break;
}
return 0;
}
#endif

#if defined(FEAT_TIMERS) || defined(PROTO)





int
ui_wait_for_chars_or_timer(
long wtime,
int (*wait_func)(long wtime, int *interrupted, int ignore_input),
int *interrupted,
int ignore_input)
{
int due_time;
long remaining = wtime;
int tb_change_cnt = typebuf.tb_change_cnt;
#if defined(FEAT_JOB_CHANNEL)
int brief_wait = FALSE;
#endif


if (wtime >= 0 && wtime < 10L)
return wait_func(wtime, NULL, ignore_input);

while (wtime < 0 || remaining > 0)
{


due_time = check_due_timer();
if (typebuf.tb_change_cnt != tb_change_cnt)
{

return FAIL;
}
if (due_time <= 0 || (wtime > 0 && due_time > remaining))
due_time = remaining;
#if defined(FEAT_JOB_CHANNEL) || defined(FEAT_SOUND_CANBERRA)
if ((due_time < 0 || due_time > 10L) && (
#if defined(FEAT_JOB_CHANNEL)
(
#if defined(FEAT_GUI)
!gui.in_use &&
#endif
(has_pending_job() || channel_any_readahead()))
#if defined(FEAT_SOUND_CANBERRA)
||
#endif
#endif
#if defined(FEAT_SOUND_CANBERRA)
has_any_sound_callback()
#endif
))
{


due_time = 10L;
#if defined(FEAT_JOB_CHANNEL)
brief_wait = TRUE;
#endif
}
#endif
if (wait_func(due_time, interrupted, ignore_input))
return OK;
if ((interrupted != NULL && *interrupted)
#if defined(FEAT_JOB_CHANNEL)
|| brief_wait
#endif
)


return FAIL;
if (wtime > 0)
remaining -= due_time;
}
return FAIL;
}
#endif




int
ui_char_avail(void)
{
#if defined(FEAT_GUI)
if (gui.in_use)
{
gui_mch_update();
return input_available();
}
#endif
#if !defined(NO_CONSOLE)
#if defined(NO_CONSOLE_INPUT)
if (no_console_input())
return 0;
#endif
return mch_char_avail();
#else
return 0;
#endif
}





void
ui_delay(long msec, int ignoreinput)
{
#if defined(FEAT_JOB_CHANNEL)
ch_log(NULL, "ui_delay(%ld)", msec);
#endif
#if defined(FEAT_GUI)
if (gui.in_use && !ignoreinput)
gui_wait_for_chars(msec, typebuf.tb_change_cnt);
else
#endif
mch_delay(msec, ignoreinput);
}






void
ui_suspend(void)
{
#if defined(FEAT_GUI)
if (gui.in_use)
{
gui_mch_iconify();
return;
}
#endif
mch_suspend();
}

#if !defined(UNIX) || !defined(SIGTSTP) || defined(PROTO) || defined(__BEOS__)




void
suspend_shell(void)
{
if (*p_sh == NUL)
emsg(_(e_shellempty));
else
{
msg_puts(_("new shell started\n"));
do_shell(NULL, 0);
}
}
#endif






int
ui_get_shellsize(void)
{
int retval;

#if defined(FEAT_GUI)
if (gui.in_use)
retval = gui_get_shellsize();
else
#endif
retval = mch_get_shellsize();

check_shellsize();


if (retval == OK)
{
set_number_default("lines", Rows);
set_number_default("columns", Columns);
}
return retval;
}






void
ui_set_shellsize(
int mustset UNUSED) 
{
#if defined(FEAT_GUI)
if (gui.in_use)
gui_set_shellsize(mustset, TRUE, RESIZE_BOTH);
else
#endif
mch_set_shellsize();
}





void
ui_new_shellsize(void)
{
if (full_screen && !exiting)
{
#if defined(FEAT_GUI)
if (gui.in_use)
gui_new_shellsize();
else
#endif
mch_new_shellsize();
}
}

#if ((defined(FEAT_EVAL) || defined(FEAT_TERMINAL)) && (defined(FEAT_GUI) || defined(MSWIN) || (defined(HAVE_TGETENT) && defined(FEAT_TERMRESPONSE)))) || defined(PROTO)








int
ui_get_winpos(int *x, int *y, varnumber_T timeout UNUSED)
{
#if defined(FEAT_GUI)
if (gui.in_use)
return gui_mch_get_winpos(x, y);
#endif
#if defined(MSWIN) && (!defined(FEAT_GUI) || defined(VIMDLL))
return mch_get_winpos(x, y);
#else
#if defined(HAVE_TGETENT) && defined(FEAT_TERMRESPONSE)
return term_get_winpos(x, y, timeout);
#else
return FAIL;
#endif
#endif
}
#endif

void
ui_breakcheck(void)
{
ui_breakcheck_force(FALSE);
}





void
ui_breakcheck_force(int force)
{
static int recursive = FALSE;
int save_updating_screen = updating_screen;




if (recursive)
return;
recursive = TRUE;


++updating_screen;

#if defined(FEAT_GUI)
if (gui.in_use)
gui_mch_update();
else
#endif
mch_breakcheck(force);

if (save_updating_screen)
updating_screen = TRUE;
else
after_updating_screen(FALSE);

recursive = FALSE;
}












#if defined(USE_INPUT_BUF) || defined(PROTO)






#if defined(FEAT_JOB_CHANNEL) || defined(FEAT_CLIENTSERVER)





#define INBUFLEN 4096
#else
#define INBUFLEN 250
#endif

static char_u inbuf[INBUFLEN + MAX_KEY_CODE_LEN];
static int inbufcount = 0; 







int
vim_is_input_buf_full(void)
{
return (inbufcount >= INBUFLEN);
}

int
vim_is_input_buf_empty(void)
{
return (inbufcount == 0);
}

#if defined(FEAT_OLE) || defined(PROTO)
int
vim_free_in_input_buf(void)
{
return (INBUFLEN - inbufcount);
}
#endif

#if defined(FEAT_GUI_GTK) || defined(PROTO)
int
vim_used_in_input_buf(void)
{
return inbufcount;
}
#endif





char_u *
get_input_buf(void)
{
garray_T *gap;


gap = ALLOC_ONE(garray_T);
if (gap != NULL)
{

gap->ga_data = alloc(inbufcount + 1);
if (gap->ga_data != NULL)
mch_memmove(gap->ga_data, inbuf, (size_t)inbufcount);
gap->ga_len = inbufcount;
}
trash_input_buf();
return (char_u *)gap;
}





void
set_input_buf(char_u *p)
{
garray_T *gap = (garray_T *)p;

if (gap != NULL)
{
if (gap->ga_data != NULL)
{
mch_memmove(inbuf, gap->ga_data, gap->ga_len);
inbufcount = gap->ga_len;
vim_free(gap->ga_data);
}
vim_free(gap);
}
}






void
add_to_input_buf(char_u *s, int len)
{
if (inbufcount + len > INBUFLEN + MAX_KEY_CODE_LEN)
return; 

while (len--)
inbuf[inbufcount++] = *s++;
}




void
add_to_input_buf_csi(char_u *str, int len)
{
int i;
char_u buf[2];

for (i = 0; i < len; ++i)
{
add_to_input_buf(str + i, 1);
if (str[i] == CSI)
{

buf[0] = KS_EXTRA;
buf[1] = (int)KE_CSI;
add_to_input_buf(buf, 2);
}
}
}




void
trash_input_buf(void)
{
inbufcount = 0;
}





int
read_from_input_buf(char_u *buf, long maxlen)
{
if (inbufcount == 0) 
fill_input_buf(TRUE);
if (maxlen > inbufcount)
maxlen = inbufcount;
mch_memmove(buf, inbuf, (size_t)maxlen);
inbufcount -= maxlen;
if (inbufcount)
mch_memmove(inbuf, inbuf + maxlen, (size_t)inbufcount);
return (int)maxlen;
}

void
fill_input_buf(int exit_on_error UNUSED)
{
#if defined(UNIX) || defined(VMS) || defined(MACOS_X)
int len;
int try;
static int did_read_something = FALSE;
static char_u *rest = NULL; 
static int restlen = 0;
int unconverted;
#endif

#if defined(FEAT_GUI)
if (gui.in_use
#if defined(NO_CONSOLE_INPUT)


&& !no_console_input()
#endif
)
{
gui_mch_update();
return;
}
#endif
#if defined(UNIX) || defined(VMS) || defined(MACOS_X)
if (vim_is_input_buf_full())
return;






#if defined(__BEOS__)




while (!vim_is_input_buf_full() && RealWaitForChar(read_cmd_fd, 0, NULL))
;
len = inbufcount;
inbufcount = 0;
#else

if (rest != NULL)
{


if (restlen > INBUFLEN - inbufcount)
unconverted = INBUFLEN - inbufcount;
else
unconverted = restlen;
mch_memmove(inbuf + inbufcount, rest, unconverted);
if (unconverted == restlen)
VIM_CLEAR(rest);
else
{
restlen -= unconverted;
mch_memmove(rest, rest + unconverted, restlen);
}
inbufcount += unconverted;
}
else
unconverted = 0;

len = 0; 
for (try = 0; try < 100; ++try)
{
size_t readlen = (size_t)((INBUFLEN - inbufcount)
/ input_conv.vc_factor);
#if defined(VMS)
len = vms_read((char *)inbuf + inbufcount, readlen);
#else
len = read(read_cmd_fd, (char *)inbuf + inbufcount, readlen);
#endif

if (len > 0 || got_int)
break;




if (!did_read_something && !isatty(read_cmd_fd) && read_cmd_fd == 0)
{
int m = cur_tmode;




settmode(TMODE_COOK);
#if defined(HAVE_DUP)

close(0);
vim_ignored = dup(2);
#else
read_cmd_fd = 2; 
#endif
settmode(m);
}
if (!exit_on_error)
return;
}
#endif
if (len <= 0 && !got_int)
read_error_exit();
if (len > 0)
did_read_something = TRUE;
if (got_int)
{

inbuf[0] = 3;
inbufcount = 1;
}
else
{








if (input_conv.vc_type != CONV_NONE)
{
inbufcount -= unconverted;
len = convert_input_safe(inbuf + inbufcount,
len + unconverted, INBUFLEN - inbufcount,
rest == NULL ? &rest : NULL, &restlen);
}
while (len-- > 0)
{




if (ctrl_c_interrupts && (inbuf[inbufcount] == 3
|| (len >= 9 && STRNCMP(inbuf + inbufcount,
"\033[27;5;99~", 10) == 0)))
{

mch_memmove(inbuf, inbuf + inbufcount, (size_t)(len + 1));
inbufcount = 0;
got_int = TRUE;
}
++inbufcount;
}
}
#endif 
}
#endif 




void
read_error_exit(void)
{
if (silent_mode) 
getout(0);
STRCPY(IObuff, _("Vim: Error reading input, exiting...\n"));
preserve_exit();
}

#if defined(CURSOR_SHAPE) || defined(PROTO)



void
ui_cursor_shape_forced(int forced)
{
#if defined(FEAT_GUI)
if (gui.in_use)
gui_update_cursor_later();
else
#endif
term_cursor_mode(forced);

#if defined(MCH_CURSOR_SHAPE)
mch_update_cursor();
#endif

#if defined(FEAT_CONCEAL)
conceal_check_cursor_line();
#endif
}

void
ui_cursor_shape(void)
{
ui_cursor_shape_forced(FALSE);
}
#endif




int
check_col(int col)
{
if (col < 0)
return 0;
if (col >= (int)screen_Columns)
return (int)screen_Columns - 1;
return col;
}




int
check_row(int row)
{
if (row < 0)
return 0;
if (row >= (int)screen_Rows)
return (int)screen_Rows - 1;
return row;
}

#if defined(FEAT_GUI) || defined(MSWIN) || defined(PROTO)




void
ui_focus_change(
int in_focus) 
{
static time_t last_time = (time_t)0;
int need_redraw = FALSE;




if (in_focus && last_time + 2 < time(NULL))
{
need_redraw = check_timestamps(
#if defined(FEAT_GUI)
gui.in_use
#else
FALSE
#endif
);
last_time = time(NULL);
}




need_redraw |= apply_autocmds(in_focus ? EVENT_FOCUSGAINED
: EVENT_FOCUSLOST, NULL, NULL, FALSE, curbuf);

if (need_redraw)
{


need_wait_return = FALSE;

if (State & CMDLINE)
redrawcmdline();
else if (State == HITRETURN || State == SETWSIZE || State == ASKMORE
|| State == EXTERNCMD || State == CONFIRM || exmode_active)
repeat_message();
else if ((State & NORMAL) || (State & INSERT))
{
if (must_redraw != 0)
update_screen(0);
setcursor();
}
cursor_on(); 
out_flush_cursor(FALSE, TRUE);
#if defined(FEAT_GUI)
if (gui.in_use)
gui_update_scrollbars(FALSE);
#endif
}
#if defined(FEAT_TITLE)

if (need_maketitle)
maketitle();
#endif
}
#endif

#if defined(HAVE_INPUT_METHOD) || defined(PROTO)



void
im_save_status(long *psave)
{







if (!p_imdisable && KeyTyped && !KeyStuffed
#if defined(FEAT_XIM)
&& xic != NULL
#endif
#if defined(FEAT_GUI)
&& (!gui.in_use || gui.in_focus)
#endif
)
{

if (vgetc_im_active)
*psave = B_IMODE_IM;
else if (*psave == B_IMODE_IM)
*psave = B_IMODE_NONE;
}
}
#endif
