












#include "vim.h"





#if defined(HAVE_LOCALTIME_R) && defined(HAVE_TZSET)
static char tz_cache[64];
#endif











static struct tm *
vim_localtime(
const time_t *timep, 
struct tm *result UNUSED) 
{
#if defined(HAVE_LOCALTIME_R)
#if defined(HAVE_TZSET)
char *tz; 

tz = (char *)mch_getenv((char_u *)"TZ");
if (tz == NULL)
tz = "";
if (STRNCMP(tz_cache, tz, sizeof(tz_cache) - 1) != 0)
{
tzset();
vim_strncpy((char_u *)tz_cache, (char_u *)tz, sizeof(tz_cache) - 1);
}
#endif 
return localtime_r(timep, result);
#else
return localtime(timep);
#endif 
}





time_T
vim_time(void)
{
#if defined(FEAT_EVAL)
return time_for_testing == 0 ? time(NULL) : time_for_testing;
#else
return time(NULL);
#endif
}








char *
get_ctime(time_t thetime, int add_newline)
{
static char buf[50];
#if defined(HAVE_STRFTIME)
struct tm tmval;
struct tm *curtime;

curtime = vim_localtime(&thetime, &tmval);

if (curtime == NULL)
vim_strncpy((char_u *)buf, (char_u *)_("(Invalid)"), sizeof(buf) - 1);
else
{
(void)strftime(buf, sizeof(buf) - 1, _("%a %b %d %H:%M:%S %Y"),
curtime);
#if defined(MSWIN)
if (enc_codepage >= 0 && (int)GetACP() != enc_codepage)
{
char_u *to_free = NULL;
int len;

acp_to_enc((char_u *)buf, (int)strlen(buf), &to_free, &len);
if (to_free != NULL)
{
STRCPY(buf, to_free);
vim_free(to_free);
}
}
#endif
}
#else
STRCPY(buf, "(unknown)");
#endif
if (add_newline)
STRCAT(buf, "\n");
return buf;
}

#if defined(FEAT_EVAL) || defined(PROTO)

#if defined(MACOS_X)
#include <time.h> 
#endif




void
f_localtime(typval_T *argvars UNUSED, typval_T *rettv)
{
rettv->vval.v_number = (varnumber_T)time(NULL);
}

#if defined(FEAT_RELTIME)




static int
list2proftime(typval_T *arg, proftime_T *tm)
{
long n1, n2;
int error = FALSE;

if (arg->v_type != VAR_LIST || arg->vval.v_list == NULL
|| arg->vval.v_list->lv_len != 2)
return FAIL;
n1 = list_find_nr(arg->vval.v_list, 0L, &error);
n2 = list_find_nr(arg->vval.v_list, 1L, &error);
#if defined(MSWIN)
tm->HighPart = n1;
tm->LowPart = n2;
#else
tm->tv_sec = n1;
tm->tv_usec = n2;
#endif
return error ? FAIL : OK;
}
#endif 




void
f_reltime(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_RELTIME)
proftime_T res;
proftime_T start;

if (argvars[0].v_type == VAR_UNKNOWN)
{

profile_start(&res);
}
else if (argvars[1].v_type == VAR_UNKNOWN)
{
if (list2proftime(&argvars[0], &res) == FAIL)
return;
profile_end(&res);
}
else
{

if (list2proftime(&argvars[0], &start) == FAIL
|| list2proftime(&argvars[1], &res) == FAIL)
return;
profile_sub(&res, &start);
}

if (rettv_list_alloc(rettv) == OK)
{
long n1, n2;

#if defined(MSWIN)
n1 = res.HighPart;
n2 = res.LowPart;
#else
n1 = res.tv_sec;
n2 = res.tv_usec;
#endif
list_append_number(rettv->vval.v_list, (varnumber_T)n1);
list_append_number(rettv->vval.v_list, (varnumber_T)n2);
}
#endif
}

#if defined(FEAT_FLOAT)



void
f_reltimefloat(typval_T *argvars UNUSED, typval_T *rettv)
{
#if defined(FEAT_RELTIME)
proftime_T tm;
#endif

rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = 0;
#if defined(FEAT_RELTIME)
if (list2proftime(&argvars[0], &tm) == OK)
rettv->vval.v_float = profile_float(&tm);
#endif
}
#endif




void
f_reltimestr(typval_T *argvars UNUSED, typval_T *rettv)
{
#if defined(FEAT_RELTIME)
proftime_T tm;
#endif

rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
#if defined(FEAT_RELTIME)
if (list2proftime(&argvars[0], &tm) == OK)
rettv->vval.v_string = vim_strsave((char_u *)profile_msg(&tm));
#endif
}

#if defined(HAVE_STRFTIME) || defined(PROTO)



void
f_strftime(typval_T *argvars, typval_T *rettv)
{
char_u result_buf[256];
struct tm tmval;
struct tm *curtime;
time_t seconds;
char_u *p;

rettv->v_type = VAR_STRING;

p = tv_get_string(&argvars[0]);
if (argvars[1].v_type == VAR_UNKNOWN)
seconds = time(NULL);
else
seconds = (time_t)tv_get_number(&argvars[1]);
curtime = vim_localtime(&seconds, &tmval);

if (curtime == NULL)
rettv->vval.v_string = vim_strsave((char_u *)_("(Invalid)"));
else
{
vimconv_T conv;
char_u *enc;

conv.vc_type = CONV_NONE;
enc = enc_locale();
convert_setup(&conv, p_enc, enc);
if (conv.vc_type != CONV_NONE)
p = string_convert(&conv, p, NULL);
if (p != NULL)
(void)strftime((char *)result_buf, sizeof(result_buf),
(char *)p, curtime);
else
result_buf[0] = NUL;

if (conv.vc_type != CONV_NONE)
vim_free(p);
convert_setup(&conv, enc, p_enc);
if (conv.vc_type != CONV_NONE)
rettv->vval.v_string = string_convert(&conv, result_buf, NULL);
else
rettv->vval.v_string = vim_strsave(result_buf);


convert_setup(&conv, NULL, NULL);
vim_free(enc);
}
}
#endif

#if defined(HAVE_STRPTIME) || defined(PROTO)



void
f_strptime(typval_T *argvars, typval_T *rettv)
{
struct tm tmval;
char_u *fmt;
char_u *str;
vimconv_T conv;
char_u *enc;

vim_memset(&tmval, NUL, sizeof(tmval));
fmt = tv_get_string(&argvars[0]);
str = tv_get_string(&argvars[1]);

conv.vc_type = CONV_NONE;
enc = enc_locale();
convert_setup(&conv, p_enc, enc);
if (conv.vc_type != CONV_NONE)
fmt = string_convert(&conv, fmt, NULL);
if (fmt == NULL
|| strptime((char *)str, (char *)fmt, &tmval) == NULL
|| (rettv->vval.v_number = mktime(&tmval)) == -1)
rettv->vval.v_number = 0;

if (conv.vc_type != CONV_NONE)
vim_free(fmt);
convert_setup(&conv, NULL, NULL);
vim_free(enc);
}
#endif

#if defined(FEAT_TIMERS) || defined(PROTO)
static timer_T *first_timer = NULL;
static long last_timer_id = 0;




long
proftime_time_left(proftime_T *due, proftime_T *now)
{
#if defined(MSWIN)
LARGE_INTEGER fr;

if (now->QuadPart > due->QuadPart)
return 0;
QueryPerformanceFrequency(&fr);
return (long)(((double)(due->QuadPart - now->QuadPart)
/ (double)fr.QuadPart) * 1000);
#else
if (now->tv_sec > due->tv_sec)
return 0;
return (due->tv_sec - now->tv_sec) * 1000
+ (due->tv_usec - now->tv_usec) / 1000;
#endif
}




static void
insert_timer(timer_T *timer)
{
timer->tr_next = first_timer;
timer->tr_prev = NULL;
if (first_timer != NULL)
first_timer->tr_prev = timer;
first_timer = timer;
did_add_timer = TRUE;
}




static void
remove_timer(timer_T *timer)
{
if (timer->tr_prev == NULL)
first_timer = timer->tr_next;
else
timer->tr_prev->tr_next = timer->tr_next;
if (timer->tr_next != NULL)
timer->tr_next->tr_prev = timer->tr_prev;
}

static void
free_timer(timer_T *timer)
{
free_callback(&timer->tr_callback);
vim_free(timer);
}





timer_T *
create_timer(long msec, int repeat)
{
timer_T *timer = ALLOC_CLEAR_ONE(timer_T);
long prev_id = last_timer_id;

if (timer == NULL)
return NULL;
if (++last_timer_id <= prev_id)

last_timer_id = 0;
timer->tr_id = last_timer_id;
insert_timer(timer);
if (repeat != 0)
timer->tr_repeat = repeat - 1;
timer->tr_interval = msec;

profile_setlimit(msec, &timer->tr_due);
return timer;
}




static void
timer_callback(timer_T *timer)
{
typval_T rettv;
typval_T argv[2];

argv[0].v_type = VAR_NUMBER;
argv[0].vval.v_number = (varnumber_T)timer->tr_id;
argv[1].v_type = VAR_UNKNOWN;

call_callback(&timer->tr_callback, -1, &rettv, 1, argv);
clear_tv(&rettv);
}






long
check_due_timer(void)
{
timer_T *timer;
timer_T *timer_next;
long this_due;
long next_due = -1;
proftime_T now;
int did_one = FALSE;
int need_update_screen = FALSE;
long current_id = last_timer_id;


if (exiting || aborting())
return next_due;

profile_start(&now);
for (timer = first_timer; timer != NULL && !got_int; timer = timer_next)
{
timer_next = timer->tr_next;

if (timer->tr_id == -1 || timer->tr_firing || timer->tr_paused)
continue;
this_due = proftime_time_left(&timer->tr_due, &now);
if (this_due <= 1)
{


int save_timer_busy = timer_busy;
int save_vgetc_busy = vgetc_busy;
int save_did_emsg = did_emsg;
int save_called_emsg = called_emsg;
int save_must_redraw = must_redraw;
int save_trylevel = trylevel;
int save_did_throw = did_throw;
int save_ex_pressedreturn = get_pressedreturn();
int save_may_garbage_collect = may_garbage_collect;
except_T *save_current_exception = current_exception;
vimvars_save_T vvsave;



timer_busy = timer_busy > 0 || vgetc_busy > 0;
vgetc_busy = 0;
called_emsg = 0;
did_emsg = FALSE;
did_uncaught_emsg = FALSE;
must_redraw = 0;
trylevel = 0;
did_throw = FALSE;
current_exception = NULL;
may_garbage_collect = FALSE;
save_vimvars(&vvsave);

timer->tr_firing = TRUE;
timer_callback(timer);
timer->tr_firing = FALSE;

timer_next = timer->tr_next;
did_one = TRUE;
timer_busy = save_timer_busy;
vgetc_busy = save_vgetc_busy;
if (did_uncaught_emsg)
++timer->tr_emsg_count;
did_emsg = save_did_emsg;
called_emsg = save_called_emsg;
trylevel = save_trylevel;
did_throw = save_did_throw;
current_exception = save_current_exception;
restore_vimvars(&vvsave);
if (must_redraw != 0)
need_update_screen = TRUE;
must_redraw = must_redraw > save_must_redraw
? must_redraw : save_must_redraw;
set_pressedreturn(save_ex_pressedreturn);
may_garbage_collect = save_may_garbage_collect;



if (timer->tr_repeat != 0 && timer->tr_id != -1
&& timer->tr_emsg_count < 3)
{
profile_setlimit(timer->tr_interval, &timer->tr_due);
this_due = proftime_time_left(&timer->tr_due, &now);
if (this_due < 1)
this_due = 1;
if (timer->tr_repeat > 0)
--timer->tr_repeat;
}
else
{
this_due = -1;
remove_timer(timer);
free_timer(timer);
}
}
if (this_due > 0 && (next_due == -1 || next_due > this_due))
next_due = this_due;
}

if (did_one)
redraw_after_callback(need_update_screen);

#if defined(FEAT_BEVAL_TERM)
if (bevalexpr_due_set)
{
this_due = proftime_time_left(&bevalexpr_due, &now);
if (this_due <= 1)
{
bevalexpr_due_set = FALSE;
if (balloonEval == NULL)
{
balloonEval = ALLOC_CLEAR_ONE(BalloonEval);
balloonEvalForTerm = TRUE;
}
if (balloonEval != NULL)
{
general_beval_cb(balloonEval, 0);
setcursor();
out_flush();
}
}
else if (next_due == -1 || next_due > this_due)
next_due = this_due;
}
#endif
#if defined(FEAT_TERMINAL)

next_due = term_check_timers(next_due, &now);
#endif

return current_id != last_timer_id ? 1 : next_due;
}




static timer_T *
find_timer(long id)
{
timer_T *timer;

if (id >= 0)
{
for (timer = first_timer; timer != NULL; timer = timer->tr_next)
if (timer->tr_id == id)
return timer;
}
return NULL;
}





void
stop_timer(timer_T *timer)
{
if (timer->tr_firing)

timer->tr_id = -1;
else
{
remove_timer(timer);
free_timer(timer);
}
}

static void
stop_all_timers(void)
{
timer_T *timer;
timer_T *timer_next;

for (timer = first_timer; timer != NULL; timer = timer_next)
{
timer_next = timer->tr_next;
stop_timer(timer);
}
}

static void
add_timer_info(typval_T *rettv, timer_T *timer)
{
list_T *list = rettv->vval.v_list;
dict_T *dict = dict_alloc();
dictitem_T *di;
long remaining;
proftime_T now;

if (dict == NULL)
return;
list_append_dict(list, dict);

dict_add_number(dict, "id", timer->tr_id);
dict_add_number(dict, "time", (long)timer->tr_interval);

profile_start(&now);
remaining = proftime_time_left(&timer->tr_due, &now);
dict_add_number(dict, "remaining", (long)remaining);

dict_add_number(dict, "repeat",
(long)(timer->tr_repeat < 0 ? -1 : timer->tr_repeat + 1));
dict_add_number(dict, "paused", (long)(timer->tr_paused));

di = dictitem_alloc((char_u *)"callback");
if (di != NULL)
{
if (dict_add(dict, di) == FAIL)
vim_free(di);
else
put_callback(&timer->tr_callback, &di->di_tv);
}
}

static void
add_timer_info_all(typval_T *rettv)
{
timer_T *timer;

for (timer = first_timer; timer != NULL; timer = timer->tr_next)
if (timer->tr_id != -1)
add_timer_info(rettv, timer);
}




int
set_ref_in_timer(int copyID)
{
int abort = FALSE;
timer_T *timer;
typval_T tv;

for (timer = first_timer; !abort && timer != NULL; timer = timer->tr_next)
{
if (timer->tr_callback.cb_partial != NULL)
{
tv.v_type = VAR_PARTIAL;
tv.vval.v_partial = timer->tr_callback.cb_partial;
}
else
{
tv.v_type = VAR_FUNC;
tv.vval.v_string = timer->tr_callback.cb_name;
}
abort = abort || set_ref_in_item(&tv, copyID, NULL, NULL);
}
return abort;
}

#if defined(EXITFREE) || defined(PROTO)
void
timer_free_all()
{
timer_T *timer;

while (first_timer != NULL)
{
timer = first_timer;
remove_timer(timer);
free_timer(timer);
}
}
#endif




void
f_timer_info(typval_T *argvars, typval_T *rettv)
{
timer_T *timer = NULL;

if (rettv_list_alloc(rettv) != OK)
return;
if (argvars[0].v_type != VAR_UNKNOWN)
{
if (argvars[0].v_type != VAR_NUMBER)
emsg(_(e_number_exp));
else
{
timer = find_timer((int)tv_get_number(&argvars[0]));
if (timer != NULL)
add_timer_info(rettv, timer);
}
}
else
add_timer_info_all(rettv);
}




void
f_timer_pause(typval_T *argvars, typval_T *rettv UNUSED)
{
timer_T *timer = NULL;
int paused = (int)tv_get_number(&argvars[1]);

if (argvars[0].v_type != VAR_NUMBER)
emsg(_(e_number_exp));
else
{
timer = find_timer((int)tv_get_number(&argvars[0]));
if (timer != NULL)
timer->tr_paused = paused;
}
}




void
f_timer_start(typval_T *argvars, typval_T *rettv)
{
long msec = (long)tv_get_number(&argvars[0]);
timer_T *timer;
int repeat = 0;
callback_T callback;
dict_T *dict;

rettv->vval.v_number = -1;
if (check_secure())
return;
if (argvars[2].v_type != VAR_UNKNOWN)
{
if (argvars[2].v_type != VAR_DICT
|| (dict = argvars[2].vval.v_dict) == NULL)
{
semsg(_(e_invarg2), tv_get_string(&argvars[2]));
return;
}
if (dict_find(dict, (char_u *)"repeat", -1) != NULL)
repeat = dict_get_number(dict, (char_u *)"repeat");
}

callback = get_callback(&argvars[1]);
if (callback.cb_name == NULL)
return;

timer = create_timer(msec, repeat);
if (timer == NULL)
free_callback(&callback);
else
{
set_callback(&timer->tr_callback, &callback);
rettv->vval.v_number = (varnumber_T)timer->tr_id;
}
}




void
f_timer_stop(typval_T *argvars, typval_T *rettv UNUSED)
{
timer_T *timer;

if (argvars[0].v_type != VAR_NUMBER)
{
emsg(_(e_number_exp));
return;
}
timer = find_timer((int)tv_get_number(&argvars[0]));
if (timer != NULL)
stop_timer(timer);
}




void
f_timer_stopall(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
stop_all_timers();
}

#endif 

#if defined(STARTUPTIME) || defined(PROTO)
static struct timeval prev_timeval;

#if defined(MSWIN)



static int
gettimeofday(struct timeval *tv, char *dummy UNUSED)
{
long t = clock();
tv->tv_sec = t / CLOCKS_PER_SEC;
tv->tv_usec = (t - tv->tv_sec * CLOCKS_PER_SEC) * 1000000 / CLOCKS_PER_SEC;
return 0;
}
#endif





void
time_push(void *tv_rel, void *tv_start)
{
*((struct timeval *)tv_rel) = prev_timeval;
gettimeofday(&prev_timeval, NULL);
((struct timeval *)tv_rel)->tv_usec = prev_timeval.tv_usec
- ((struct timeval *)tv_rel)->tv_usec;
((struct timeval *)tv_rel)->tv_sec = prev_timeval.tv_sec
- ((struct timeval *)tv_rel)->tv_sec;
if (((struct timeval *)tv_rel)->tv_usec < 0)
{
((struct timeval *)tv_rel)->tv_usec += 1000000;
--((struct timeval *)tv_rel)->tv_sec;
}
*(struct timeval *)tv_start = prev_timeval;
}







void
time_pop(
void *tp) 
{
prev_timeval.tv_usec -= ((struct timeval *)tp)->tv_usec;
prev_timeval.tv_sec -= ((struct timeval *)tp)->tv_sec;
if (prev_timeval.tv_usec < 0)
{
prev_timeval.tv_usec += 1000000;
--prev_timeval.tv_sec;
}
}

static void
time_diff(struct timeval *then, struct timeval *now)
{
long usec;
long msec;

usec = now->tv_usec - then->tv_usec;
msec = (now->tv_sec - then->tv_sec) * 1000L + usec / 1000L,
usec = usec % 1000L;
fprintf(time_fd, "%03ld.%03ld", msec, usec >= 0 ? usec : usec + 1000L);
}

void
time_msg(
char *mesg,
void *tv_start) 

{
static struct timeval start;
struct timeval now;

if (time_fd != NULL)
{
if (strstr(mesg, "STARTING") != NULL)
{
gettimeofday(&start, NULL);
prev_timeval = start;
fprintf(time_fd, "\n\ntimes in msec\n");
fprintf(time_fd, " clock self+sourced self: sourced script\n");
fprintf(time_fd, " clock elapsed: other lines\n\n");
}
gettimeofday(&now, NULL);
time_diff(&start, &now);
if (((struct timeval *)tv_start) != NULL)
{
fprintf(time_fd, " ");
time_diff(((struct timeval *)tv_start), &now);
}
fprintf(time_fd, " ");
time_diff(&prev_timeval, &now);
prev_timeval = now;
fprintf(time_fd, ": %s\n", mesg);
}
}
#endif 
#endif 

#if defined(FEAT_SPELL) || defined(FEAT_PERSISTENT_UNDO) || defined(PROTO)




time_T
get8ctime(FILE *fd)
{
int c;
time_T n = 0;
int i;

for (i = 0; i < 8; ++i)
{
c = getc(fd);
if (c == EOF) return -1;
n = (n << 8) + c;
}
return n;
}

#if defined(_MSC_VER)
#if (_MSC_VER <= 1200)


#pragma optimize("", off)
#endif
#endif





int
put_time(FILE *fd, time_T the_time)
{
char_u buf[8];

time_to_bytes(the_time, buf);
return fwrite(buf, (size_t)8, (size_t)1, fd) == 1 ? OK : FAIL;
}




void
time_to_bytes(time_T the_time, char_u *buf)
{
int c;
int i;
int bi = 0;
time_T wtime = the_time;








for (i = 7; i >= 0; --i)
{
if (i + 1 > (int)sizeof(time_T))

buf[bi++] = 0;
else
{
#if defined(SIZEOF_TIME_T) && SIZEOF_TIME_T > 4
c = (int)(wtime >> (i * 8));
#else
c = (int)((long_u)wtime >> (i * 8));
#endif
buf[bi++] = c;
}
}
}

#if defined(_MSC_VER)
#if (_MSC_VER <= 1200)
#pragma optimize("", on)
#endif
#endif

#endif




void
add_time(char_u *buf, size_t buflen, time_t tt)
{
#if defined(HAVE_STRFTIME)
struct tm tmval;
struct tm *curtime;

if (vim_time() - tt >= 100)
{
curtime = vim_localtime(&tt, &tmval);
if (vim_time() - tt < (60L * 60L * 12L))

(void)strftime((char *)buf, buflen, "%H:%M:%S", curtime);
else

(void)strftime((char *)buf, buflen, "%Y/%m/%d %H:%M:%S", curtime);
}
else
#endif
{
long seconds = (long)(vim_time() - tt);

vim_snprintf((char *)buf, buflen,
NGETTEXT("%ld second ago", "%ld seconds ago", seconds),
seconds);
}
}
