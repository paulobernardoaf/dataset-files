



















#include "vim.h"

#if defined(FEAT_MZSCHEME)
#include "if_mzsch.h"
#endif

#include "os_unixx.h" 

#if defined(USE_XSMP)
#include <X11/SM/SMlib.h>
#endif

#if defined(HAVE_SELINUX)
#include <selinux/selinux.h>
static int selinux_enabled = -1;
#endif

#if defined(HAVE_SMACK)
#include <attr/xattr.h>
#include <linux/xattr.h>
#if !defined(SMACK_LABEL_LEN)
#define SMACK_LABEL_LEN 1024
#endif
#endif

#if defined(__BEOS__)
#undef select
#define select beos_select
#endif

#if defined(__CYGWIN__)
#if !defined(MSWIN)
#include <cygwin/version.h>
#include <sys/cygwin.h> 

#if defined(FEAT_CYGWIN_WIN32_CLIPBOARD)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "winclip.pro"
#endif
#endif
#endif

#if defined(FEAT_MOUSE_GPM)
#include <gpm.h>




#define KG_SHIFT 0
#define KG_CTRL 2
#define KG_ALT 3
#define KG_ALTGR 1
#define KG_SHIFTL 4
#define KG_SHIFTR 5
#define KG_CTRLL 6
#define KG_CTRLR 7
#define KG_CAPSSHIFT 8

static void gpm_close(void);
static int gpm_open(void);
static int mch_gpm_process(void);
#endif

#if defined(FEAT_SYSMOUSE)
#include <sys/consio.h>
#include <sys/fbio.h>

static int sysmouse_open(void);
static void sysmouse_close(void);
static RETSIGTYPE sig_sysmouse SIGPROTOARG;
#endif







#if defined(ESIX) || defined(M_UNIX) && !defined(SCO)
#if defined(SIGWINCH)
#undef SIGWINCH
#endif
#if defined(TIOCGWINSZ)
#undef TIOCGWINSZ
#endif
#endif

#if defined(SIGWINDOW) && !defined(SIGWINCH) 
#define SIGWINCH SIGWINDOW
#endif

#if defined(FEAT_X11)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#if defined(FEAT_XCLIPBOARD)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
static Widget xterm_Shell = (Widget)0;
static void clip_update(void);
static void xterm_update(void);
#endif

#if defined(FEAT_XCLIPBOARD) || defined(FEAT_TITLE)
Window x11_window = 0;
#endif
Display *x11_display = NULL;
#endif

#if defined(FEAT_TITLE)
static int get_x11_title(int);

static char_u *oldtitle = NULL;
static volatile sig_atomic_t oldtitle_outdated = FALSE;
static int unix_did_set_title = FALSE;
static char_u *oldicon = NULL;
static int did_set_icon = FALSE;
#endif

static void may_core_dump(void);

#if defined(HAVE_UNION_WAIT)
typedef union wait waitstatus;
#else
typedef int waitstatus;
#endif
static int WaitForChar(long msec, int *interrupted, int ignore_input);
static int WaitForCharOrMouse(long msec, int *interrupted, int ignore_input);
#if defined(__BEOS__) || defined(VMS)
int RealWaitForChar(int, long, int *, int *interrupted);
#else
static int RealWaitForChar(int, long, int *, int *interrupted);
#endif

#if defined(FEAT_XCLIPBOARD)
static int do_xterm_trace(void);
#define XT_TRACE_DELAY 50 
#endif

static void handle_resize(void);

#if defined(SIGWINCH)
static RETSIGTYPE sig_winch SIGPROTOARG;
#endif
#if defined(SIGINT)
static RETSIGTYPE catch_sigint SIGPROTOARG;
#endif
#if defined(SIGPWR)
static RETSIGTYPE catch_sigpwr SIGPROTOARG;
#endif
#if defined(SIGALRM) && defined(FEAT_X11) && defined(FEAT_TITLE) && !defined(FEAT_GUI_GTK)

#define SET_SIG_ALARM
static RETSIGTYPE sig_alarm SIGPROTOARG;

static volatile sig_atomic_t sig_alarm_called;
#endif
static RETSIGTYPE deathtrap SIGPROTOARG;

static void catch_int_signal(void);
static void set_signals(void);
static void catch_signals(RETSIGTYPE (*func_deadly)(), RETSIGTYPE (*func_other)());
#if defined(HAVE_SIGPROCMASK)
#define SIGSET_DECL(set) sigset_t set;
#define BLOCK_SIGNALS(set) block_signals(set)
#define UNBLOCK_SIGNALS(set) unblock_signals(set)
#else
#define SIGSET_DECL(set)
#define BLOCK_SIGNALS(set) do { } while (0)
#define UNBLOCK_SIGNALS(set) do { } while (0)
#endif
static int have_wildcard(int, char_u **);
static int have_dollars(int, char_u **);

static int save_patterns(int num_pat, char_u **pat, int *num_file, char_u ***file);

#if !defined(SIG_ERR)
#define SIG_ERR ((RETSIGTYPE (*)())-1)
#endif


static volatile sig_atomic_t do_resize = FALSE;
static char_u *extra_shell_arg = NULL;
static int show_shell_mess = TRUE;

static volatile sig_atomic_t deadly_signal = 0; 

static volatile sig_atomic_t in_mch_delay = FALSE; 

#if defined(FEAT_JOB_CHANNEL) && !defined(USE_SYSTEM)
static int dont_check_job_ended = 0;
#endif

static int curr_tmode = TMODE_COOK; 

#if defined(USE_XSMP)
typedef struct
{
SmcConn smcconn; 
IceConn iceconn; 
char *clientid; 
Bool save_yourself; 
Bool shutdown; 
} xsmp_config_T;

static xsmp_config_T xsmp;
#endif

#if defined(SYS_SIGLIST_DECLARED)









#endif

static struct signalinfo
{
int sig; 
char *name; 
char deadly; 
} signal_info[] =
{
#if defined(SIGHUP)
{SIGHUP, "HUP", TRUE},
#endif
#if defined(SIGQUIT)
{SIGQUIT, "QUIT", TRUE},
#endif
#if defined(SIGILL)
{SIGILL, "ILL", TRUE},
#endif
#if defined(SIGTRAP)
{SIGTRAP, "TRAP", TRUE},
#endif
#if defined(SIGABRT)
{SIGABRT, "ABRT", TRUE},
#endif
#if defined(SIGEMT)
{SIGEMT, "EMT", TRUE},
#endif
#if defined(SIGFPE)
{SIGFPE, "FPE", TRUE},
#endif
#if defined(SIGBUS)
{SIGBUS, "BUS", TRUE},
#endif
#if defined(SIGSEGV) && !defined(FEAT_MZSCHEME)

{SIGSEGV, "SEGV", TRUE},
#endif
#if defined(SIGSYS)
{SIGSYS, "SYS", TRUE},
#endif
#if defined(SIGALRM)
{SIGALRM, "ALRM", FALSE}, 
#endif
#if defined(SIGTERM)
{SIGTERM, "TERM", TRUE},
#endif
#if defined(SIGVTALRM) && !defined(FEAT_RUBY)
{SIGVTALRM, "VTALRM", TRUE},
#endif
#if defined(SIGPROF) && !defined(FEAT_MZSCHEME) && !defined(WE_ARE_PROFILING)


{SIGPROF, "PROF", TRUE},
#endif
#if defined(SIGXCPU)
{SIGXCPU, "XCPU", TRUE},
#endif
#if defined(SIGXFSZ)
{SIGXFSZ, "XFSZ", TRUE},
#endif
#if defined(SIGUSR1)
{SIGUSR1, "USR1", TRUE},
#endif
#if defined(SIGUSR2) && !defined(FEAT_SYSMOUSE)

{SIGUSR2, "USR2", TRUE},
#endif
#if defined(SIGINT)
{SIGINT, "INT", FALSE},
#endif
#if defined(SIGWINCH)
{SIGWINCH, "WINCH", FALSE},
#endif
#if defined(SIGTSTP)
{SIGTSTP, "TSTP", FALSE},
#endif
#if defined(SIGPIPE)
{SIGPIPE, "PIPE", FALSE},
#endif
{-1, "Unknown!", FALSE}
};

int
mch_chdir(char *path)
{
if (p_verbose >= 5)
{
verbose_enter();
smsg("chdir(%s)", path);
verbose_leave();
}
#if defined(VMS)
return chdir(vms_fixfilename(path));
#else
return chdir(path);
#endif
}


#if defined(ECHOE) && defined(ICANON) && (defined(HAVE_TERMIO_H) || defined(HAVE_TERMIOS_H)) && !defined(__NeXT__)


#define NEW_TTY_SYSTEM
#endif




void
mch_write(char_u *s, int len)
{
vim_ignored = (int)write(1, (char *)s, len);
if (p_wd) 
RealWaitForChar(read_cmd_fd, p_wd, NULL, NULL);
}






static int
resize_func(int check_only)
{
if (check_only)
return do_resize;
while (do_resize)
handle_resize();
return FALSE;
}









int
mch_inchar(
char_u *buf,
int maxlen,
long wtime, 
int tb_change_cnt)
{
return inchar_loop(buf, maxlen, wtime, tb_change_cnt,
WaitForChar, resize_func);
}

static void
handle_resize(void)
{
do_resize = FALSE;
shell_resized();
}




int
mch_char_avail(void)
{
return WaitForChar(0L, NULL, FALSE);
}

#if defined(FEAT_TERMINAL) || defined(PROTO)



int
mch_check_messages(void)
{
return WaitForChar(0L, NULL, TRUE);
}
#endif

#if defined(HAVE_TOTAL_MEM) || defined(PROTO)
#if defined(HAVE_SYS_RESOURCE_H)
#include <sys/resource.h>
#endif
#if defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL)
#include <sys/sysctl.h>
#endif
#if defined(HAVE_SYS_SYSINFO_H) && defined(HAVE_SYSINFO)
#include <sys/sysinfo.h>
#endif
#if defined(MACOS_X)
#include <mach/mach_host.h>
#include <mach/mach_port.h>
#endif





long_u
mch_total_mem(int special UNUSED)
{
long_u mem = 0;
long_u shiftright = 10; 

#if defined(MACOS_X)
{

mach_port_t host = mach_host_self();
kern_return_t kret;
#if defined(HOST_VM_INFO64)
struct vm_statistics64 vm_stat;
natural_t count = HOST_VM_INFO64_COUNT;

kret = host_statistics64(host, HOST_VM_INFO64,
(host_info64_t)&vm_stat, &count);
#else
struct vm_statistics vm_stat;
natural_t count = HOST_VM_INFO_COUNT;

kret = host_statistics(host, HOST_VM_INFO,
(host_info_t)&vm_stat, &count);
#endif
if (kret == KERN_SUCCESS)

mem = (long_u)(vm_stat.free_count + vm_stat.active_count
+ vm_stat.inactive_count
#if defined(MAC_OS_X_VERSION_10_9)
+ vm_stat.compressor_page_count
#endif
) * sysconf(_SC_PAGESIZE);
mach_port_deallocate(mach_task_self(), host);
}
#endif

#if defined(HAVE_SYSCTL)
if (mem == 0)
{

int mib[2];
size_t len = sizeof(long_u);
#if defined(HW_USERMEM64)
long_u physmem;
#else

union {
int_u u32;
long_u u64;
} physmem;
#endif

mib[0] = CTL_HW;
#if defined(HW_USERMEM64)
mib[1] = HW_USERMEM64;
#else
mib[1] = HW_USERMEM;
#endif
if (sysctl(mib, 2, &physmem, &len, NULL, 0) == 0)
{
#if defined(HW_USERMEM64)
mem = (long_u)physmem;
#else
if (len == sizeof(physmem.u64))
mem = (long_u)physmem.u64;
else
mem = (long_u)physmem.u32;
#endif
}
}
#endif

#if defined(HAVE_SYS_SYSINFO_H) && defined(HAVE_SYSINFO)
if (mem == 0)
{
struct sysinfo sinfo;


if (sysinfo(&sinfo) == 0)
{
#if defined(HAVE_SYSINFO_MEM_UNIT)

while (shiftright > 0 && (sinfo.mem_unit & 1) == 0)
{
sinfo.mem_unit = sinfo.mem_unit >> 1;
--shiftright;
}
mem = sinfo.totalram * sinfo.mem_unit;
#else
mem = sinfo.totalram;
#endif
}
}
#endif

#if defined(HAVE_SYSCONF)
if (mem == 0)
{
long pagesize, pagecount;


pagesize = sysconf(_SC_PAGESIZE);
pagecount = sysconf(_SC_PHYS_PAGES);
if (pagesize > 0 && pagecount > 0)
{

while (shiftright > 0 && (pagesize & 1) == 0)
{
pagesize = (long_u)pagesize >> 1;
--shiftright;
}
mem = (long_u)pagesize * pagecount;
}
}
#endif



#if defined(HAVE_SYS_RESOURCE_H) && defined(HAVE_GETRLIMIT)
{
struct rlimit rlp;

if (getrlimit(RLIMIT_DATA, &rlp) == 0
&& rlp.rlim_cur < ((rlim_t)1 << (sizeof(long_u) * 8 - 1))
#if defined(RLIM_INFINITY)
&& rlp.rlim_cur != RLIM_INFINITY
#endif
&& ((long_u)rlp.rlim_cur >> 10) < (mem >> shiftright)
)
{
mem = (long_u)rlp.rlim_cur;
shiftright = 10;
}
}
#endif

if (mem > 0)
return mem >> shiftright;
return (long_u)0x1fffff;
}
#endif

void
mch_delay(long msec, int ignoreinput)
{
int old_tmode;
#if defined(FEAT_MZSCHEME)
long total = msec; 
#endif

if (ignoreinput)
{



in_mch_delay = TRUE;
old_tmode = curr_tmode;
if (curr_tmode == TMODE_RAW)
settmode(TMODE_SLEEP);






#if defined(FEAT_MZSCHEME)
do
{

if (total > p_mzq)
msec = p_mzq;
else
msec = total;
total -= msec;
#endif
#if defined(HAVE_NANOSLEEP)
{
struct timespec ts;

ts.tv_sec = msec / 1000;
ts.tv_nsec = (msec % 1000) * 1000000;
(void)nanosleep(&ts, NULL);
}
#else
#if defined(HAVE_USLEEP)
while (msec >= 1000)
{
usleep((unsigned int)(999 * 1000));
msec -= 999;
}
usleep((unsigned int)(msec * 1000));
#else
#if !defined(HAVE_SELECT)
poll(NULL, 0, (int)msec);
#else
{
struct timeval tv;

tv.tv_sec = msec / 1000;
tv.tv_usec = (msec % 1000) * 1000;




select(0, NULL, NULL, NULL, &tv);
}
#endif 
#endif 
#endif 
#if defined(FEAT_MZSCHEME)
}
while (total > 0);
#endif

settmode(old_tmode);
in_mch_delay = FALSE;
}
else
WaitForChar(msec, NULL, FALSE);
}

#if defined(HAVE_STACK_LIMIT) || (!defined(HAVE_SIGALTSTACK) && defined(HAVE_SIGSTACK))

#define HAVE_CHECK_STACK_GROWTH








static int stack_grows_downwards;





static void
check_stack_growth(char *p)
{
int i;

stack_grows_downwards = (p > (char *)&i);
}
#endif

#if defined(HAVE_STACK_LIMIT) || defined(PROTO)
static char *stack_limit = NULL;

#if defined(_THREAD_SAFE) && defined(HAVE_PTHREAD_NP_H)
#include <pthread.h>
#include <pthread_np.h>
#endif






static void
get_stack_limit(void)
{
struct rlimit rlp;
int i;
long lim;



if (getrlimit(RLIMIT_STACK, &rlp) == 0
&& rlp.rlim_cur < ((rlim_t)1 << (sizeof(long_u) * 8 - 1))
#if defined(RLIM_INFINITY)
&& rlp.rlim_cur != RLIM_INFINITY
#endif
)
{
lim = (long)rlp.rlim_cur;
#if defined(_THREAD_SAFE) && defined(HAVE_PTHREAD_NP_H)
{
pthread_attr_t attr;
size_t size;



pthread_attr_init(&attr);
if (pthread_attr_get_np(pthread_self(), &attr) == 0)
{
pthread_attr_getstacksize(&attr, &size);
if (lim > (long)size)
lim = (long)size;
}
pthread_attr_destroy(&attr);
}
#endif
if (stack_grows_downwards)
{
stack_limit = (char *)((long)&i - (lim / 16L * 15L));
if (stack_limit >= (char *)&i)

stack_limit = (char *)((long)&i / 16L);
}
else
{
stack_limit = (char *)((long)&i + (lim / 16L * 15L));
if (stack_limit <= (char *)&i)
stack_limit = NULL; 
}
}
}





int
mch_stackcheck(char *p)
{
if (stack_limit != NULL)
{
if (stack_grows_downwards)
{
if (p < stack_limit)
return FAIL;
}
else if (p > stack_limit)
return FAIL;
}
return OK;
}
#endif

#if defined(HAVE_SIGALTSTACK) || defined(HAVE_SIGSTACK)







#if !defined(SIGSTKSZ)
#define SIGSTKSZ 8000 
#endif

#if defined(HAVE_SIGALTSTACK)
static stack_t sigstk; 
#else
static struct sigstack sigstk; 
#endif

static char *signal_stack;

static void
init_signal_stack(void)
{
if (signal_stack != NULL)
{
#if defined(HAVE_SIGALTSTACK)
#if defined(HAVE_SS_BASE)
sigstk.ss_base = signal_stack;
#else
sigstk.ss_sp = signal_stack;
#endif
sigstk.ss_size = SIGSTKSZ;
sigstk.ss_flags = 0;
(void)sigaltstack(&sigstk, NULL);
#else
sigstk.ss_sp = signal_stack;
if (stack_grows_downwards)
sigstk.ss_sp += SIGSTKSZ - 1;
sigstk.ss_onstack = 0;
(void)sigstack(&sigstk, NULL);
#endif
}
}
#endif






#if defined(SIGWINCH)
static RETSIGTYPE
sig_winch SIGDEFARG(sigarg)
{

signal(SIGWINCH, (RETSIGTYPE (*)())sig_winch);
do_resize = TRUE;
SIGRETURN;
}
#endif

#if defined(SIGINT)
static RETSIGTYPE
catch_sigint SIGDEFARG(sigarg)
{

signal(SIGINT, (RETSIGTYPE (*)())catch_sigint);
got_int = TRUE;
SIGRETURN;
}
#endif

#if defined(SIGPWR)
static RETSIGTYPE
catch_sigpwr SIGDEFARG(sigarg)
{

signal(SIGPWR, (RETSIGTYPE (*)())catch_sigpwr);





ml_sync_all(FALSE, FALSE);
SIGRETURN;
}
#endif

#if defined(SET_SIG_ALARM)



static RETSIGTYPE
sig_alarm SIGDEFARG(sigarg)
{

sig_alarm_called = TRUE;
SIGRETURN;
}
#endif

#if (defined(HAVE_SETJMP_H) && ((defined(FEAT_X11) && defined(FEAT_XCLIPBOARD)) || defined(FEAT_LIBCALL))) || defined(PROTO)



#define USING_SETJMP 1


static JMP_BUF lc_jump_env;

#if defined(SIGHASARG)


static volatile sig_atomic_t lc_signal;
#endif



static volatile sig_atomic_t lc_active INIT(= FALSE);























static void
mch_startjmp(void)
{
#if defined(SIGHASARG)
lc_signal = 0;
#endif
lc_active = TRUE;
}

static void
mch_endjmp(void)
{
lc_active = FALSE;
}

static void
mch_didjmp(void)
{
#if defined(HAVE_SIGALTSTACK) || defined(HAVE_SIGSTACK)


init_signal_stack();
#endif
}
#endif








static RETSIGTYPE
deathtrap SIGDEFARG(sigarg)
{
static int entered = 0; 


#if defined(SIGHASARG)
int i;
#endif

#if defined(USING_SETJMP)





if (lc_active)
{
#if defined(SIGHASARG)
lc_signal = sigarg;
#endif
lc_active = FALSE; 
LONGJMP(lc_jump_env, 1);

}
#endif

#if defined(SIGHASARG)
#if defined(SIGQUIT)



if (in_mch_delay && sigarg == SIGQUIT)
SIGRETURN;
#endif




if (entered == 0
&& (0
#if defined(SIGHUP)
|| sigarg == SIGHUP
#endif
#if defined(SIGQUIT)
|| sigarg == SIGQUIT
#endif
#if defined(SIGTERM)
|| sigarg == SIGTERM
#endif
#if defined(SIGPWR)
|| sigarg == SIGPWR
#endif
#if defined(SIGUSR1)
|| sigarg == SIGUSR1
#endif
#if defined(SIGUSR2)
|| sigarg == SIGUSR2
#endif
)
&& !vim_handle_signal(sigarg))
SIGRETURN;
#endif


++entered;



block_autocmds();

#if defined(FEAT_EVAL)

set_vim_var_nr(VV_DYING, (long)entered);
#endif
v_dying = entered;

#if defined(HAVE_STACK_LIMIT)


get_stack_limit();
#endif

#if 0



{
#define VI_GDB_FILE "/tmp/vimgdb"
#define VIM_NAME "/usr/bin/vim"
FILE *fp = fopen(VI_GDB_FILE, "w");
if (fp)
{
fprintf(fp,
"file %s\n"
"attach %d\n"
"set height 1000\n"
"bt full\n"
, VIM_NAME, getpid());
fclose(fp);
system("xterm -e gdb -x "VI_GDB_FILE);
unlink(VI_GDB_FILE);
}
}
#endif

#if defined(SIGHASARG)

for (i = 0; signal_info[i].sig != -1; i++)
if (sigarg == signal_info[i].sig)
break;
deadly_signal = sigarg;
#endif

full_screen = FALSE; 










if (entered >= 3)
{
reset_signals(); 
may_core_dump();
if (entered >= 4)
_exit(8);
exit(7);
}
if (entered == 2)
{

OUT_STR("Vim: Double signal, exiting\n");
out_flush();
getout(1);
}


#if defined(SIGHASARG)
sprintf((char *)IObuff, "Vim: Caught deadly signal %s\n",
signal_info[i].name);
#else
sprintf((char *)IObuff, "Vim: Caught deadly signal\n");
#endif



preserve_exit();



#if defined(NBDEBUG)
reset_signals();
may_core_dump();
abort();
#endif

SIGRETURN;
}





static void
after_sigcont(void)
{
#if defined(FEAT_TITLE)


oldtitle_outdated = TRUE;
#endif
settmode(TMODE_RAW);
need_check_timestamps = TRUE;
did_check_timestamps = FALSE;
}

#if defined(SIGCONT)
static RETSIGTYPE sigcont_handler SIGPROTOARG;
static volatile sig_atomic_t in_mch_suspend = FALSE;












static volatile sig_atomic_t sigcont_received;
static RETSIGTYPE sigcont_handler SIGPROTOARG;




static RETSIGTYPE
sigcont_handler SIGDEFARG(sigarg)
{
if (in_mch_suspend)
{
sigcont_received = TRUE;
}
else
{




after_sigcont();
redraw_later(CLEAR);
cursor_on_force();
out_flush();
}

SIGRETURN;
}
#endif

#if defined(FEAT_CLIPBOARD) && defined(FEAT_X11)
#if defined(USE_SYSTEM)
static void *clip_star_save = NULL;
static void *clip_plus_save = NULL;
#endif






static void
loose_clipboard(void)
{
if (clip_star.owned || clip_plus.owned)
{
x11_export_final_selection();
if (clip_star.owned)
clip_lose_selection(&clip_star);
if (clip_plus.owned)
clip_lose_selection(&clip_plus);
if (x11_display != NULL)
XFlush(x11_display);
}
}

#if defined(USE_SYSTEM)



static void
save_clipboard(void)
{
if (clip_star.owned)
clip_star_save = get_register('*', TRUE);
if (clip_plus.owned)
clip_plus_save = get_register('+', TRUE);
}




static void
restore_clipboard(void)
{
if (clip_star_save != NULL)
{
if (!clip_gen_owner_exists(&clip_star))
put_register('*', clip_star_save);
else
free_register(clip_star_save);
clip_star_save = NULL;
}
if (clip_plus_save != NULL)
{
if (!clip_gen_owner_exists(&clip_plus))
put_register('+', clip_plus_save);
else
free_register(clip_plus_save);
clip_plus_save = NULL;
}
}
#endif
#endif





void
mch_suspend(void)
{

#if defined(SIGTSTP) && !defined(__BEOS__)
in_mch_suspend = TRUE;

out_flush(); 
settmode(TMODE_COOK);
out_flush(); 

#if defined(FEAT_CLIPBOARD) && defined(FEAT_X11)
loose_clipboard();
#endif
#if defined(SIGCONT)
sigcont_received = FALSE;
#endif

kill(0, SIGTSTP); 

#if defined(SIGCONT)









{
long wait_time;

for (wait_time = 0; !sigcont_received && wait_time <= 3L; wait_time++)
mch_delay(wait_time, FALSE);
}
#endif
in_mch_suspend = FALSE;

after_sigcont();
#else
suspend_shell();
#endif
}

void
mch_init(void)
{
Columns = 80;
Rows = 24;

out_flush();
set_signals();

#if defined(MACOS_CONVERT)
mac_conv_init();
#endif
#if defined(FEAT_CYGWIN_WIN32_CLIPBOARD)
win_clip_init();
#endif
}

static void
set_signals(void)
{
#if defined(SIGWINCH)



signal(SIGWINCH, (RETSIGTYPE (*)())sig_winch);
#endif





#if defined(SIGTSTP)
signal(SIGTSTP, restricted ? SIG_IGN : SIG_DFL);
#endif
#if defined(SIGCONT)
signal(SIGCONT, sigcont_handler);
#endif




#if defined(SIGPIPE)
signal(SIGPIPE, SIG_IGN);
#endif

#if defined(SIGINT)
catch_int_signal();
#endif




#if defined(SIGALRM)
signal(SIGALRM, SIG_IGN);
#endif





#if defined(SIGPWR)
signal(SIGPWR, (RETSIGTYPE (*)())catch_sigpwr);
#endif




catch_signals(deathtrap, SIG_ERR);

#if defined(FEAT_GUI) && defined(SIGHUP)



if (gui.in_use)
signal(SIGHUP, SIG_IGN);
#endif
}

#if defined(SIGINT) || defined(PROTO)



static void
catch_int_signal(void)
{
signal(SIGINT, (RETSIGTYPE (*)())catch_sigint);
}
#endif

void
reset_signals(void)
{
catch_signals(SIG_DFL, SIG_DFL);
#if defined(SIGCONT)

signal(SIGCONT, SIG_DFL);
#endif
}

static void
catch_signals(
RETSIGTYPE (*func_deadly)(),
RETSIGTYPE (*func_other)())
{
int i;

for (i = 0; signal_info[i].sig != -1; i++)
if (signal_info[i].deadly)
{
#if defined(HAVE_SIGALTSTACK) && defined(HAVE_SIGACTION)
struct sigaction sa;


sa.sa_handler = func_deadly;
sigemptyset(&sa.sa_mask);
#if defined(__linux__) && defined(_REENTRANT)





sa.sa_flags = 0;
#else
sa.sa_flags = SA_ONSTACK;
#endif
sigaction(signal_info[i].sig, &sa, NULL);
#else
#if defined(HAVE_SIGALTSTACK) && defined(HAVE_SIGVEC)
struct sigvec sv;


sv.sv_handler = func_deadly;
sv.sv_mask = 0;
sv.sv_flags = SV_ONSTACK;
sigvec(signal_info[i].sig, &sv, NULL);
#else
signal(signal_info[i].sig, func_deadly);
#endif
#endif
}
else if (func_other != SIG_ERR)
signal(signal_info[i].sig, func_other);
}

#if defined(HAVE_SIGPROCMASK)
static void
block_signals(sigset_t *set)
{
sigset_t newset;
int i;

sigemptyset(&newset);

for (i = 0; signal_info[i].sig != -1; i++)
sigaddset(&newset, signal_info[i].sig);

#if defined(SIGCONT)

sigaddset(&newset, SIGCONT);
#endif

sigprocmask(SIG_BLOCK, &newset, set);
}

static void
unblock_signals(sigset_t *set)
{
sigprocmask(SIG_SETMASK, set, NULL);
}
#endif










int
vim_handle_signal(int sig)
{
static int got_signal = 0;
static int blocked = TRUE;

switch (sig)
{
case SIGNAL_BLOCK: blocked = TRUE;
break;

case SIGNAL_UNBLOCK: blocked = FALSE;
if (got_signal != 0)
{
kill(getpid(), got_signal);
got_signal = 0;
}
break;

default: if (!blocked)
return TRUE; 
got_signal = sig;
#if defined(SIGPWR)
if (sig != SIGPWR)
#endif
got_int = TRUE; 
break;
}
return FALSE;
}




int
mch_check_win(int argc UNUSED, char **argv UNUSED)
{
if (isatty(1))
return OK;
return FAIL;
}




int
mch_input_isatty(void)
{
if (isatty(read_cmd_fd))
return TRUE;
return FALSE;
}

#if defined(FEAT_X11)

#if defined(ELAPSED_TIMEVAL) && (defined(FEAT_XCLIPBOARD) || defined(FEAT_TITLE))





static void
xopen_message(long elapsed_msec)
{
smsg(_("Opening the X display took %ld msec"), elapsed_msec);
}
#endif
#endif

#if defined(FEAT_X11) && (defined(FEAT_TITLE) || defined(FEAT_XCLIPBOARD))




static int got_x_error = FALSE;




static int
x_error_handler(Display *dpy, XErrorEvent *error_event)
{
XGetErrorText(dpy, error_event->error_code, (char *)IObuff, IOSIZE);
STRCAT(IObuff, _("\nVim: Got X error\n"));




preserve_exit(); 

return 0; 
}




static int
x_error_check(Display *dpy UNUSED, XErrorEvent *error_event UNUSED)
{
got_x_error = TRUE;
return 0;
}




static int
x_connect_to_server(void)
{

if (exiting || v_dying)
return FALSE;

#if defined(FEAT_CLIENTSERVER)
if (x_force_connect)
return TRUE;
#endif
if (x_no_connect)
return FALSE;


if (clip_exclude_prog != NULL)
{



if (regprog_in_use(clip_exclude_prog))
return FALSE;

if (vim_regexec_prog(&clip_exclude_prog, FALSE, T_NAME, (colnr_T)0))
return FALSE;
}
return TRUE;
}

#if defined(FEAT_X11) && defined(FEAT_XCLIPBOARD)
#if defined(USING_SETJMP)



static int
x_IOerror_check(Display *dpy UNUSED)
{

LONGJMP(lc_jump_env, 1);
#if defined(VMS) || defined(__CYGWIN__)
return 0; 
#endif
}
#endif




static int xterm_dpy_retry_count = 0;

static int
x_IOerror_handler(Display *dpy UNUSED)
{
xterm_dpy = NULL;
xterm_dpy_retry_count = 5; 
x11_window = 0;
x11_display = NULL;
xterm_Shell = (Widget)0;


LONGJMP(x_jump_env, 1);
#if defined(VMS) || defined(__CYGWIN__)
return 0; 
#endif
}






static void
may_restore_clipboard(void)
{

if (!exiting && !v_dying && xterm_dpy_retry_count > 0)
{
--xterm_dpy_retry_count;

#if !defined(LESSTIF_VERSION)

if (app_context != (XtAppContext)NULL)
{
XtDestroyApplicationContext(app_context);
app_context = (XtAppContext)NULL;
x11_display = NULL; 
}
#endif

setup_term_clip();
get_x11_title(FALSE);
}
}

void
ex_xrestore(exarg_T *eap)
{
if (eap->arg != NULL && STRLEN(eap->arg) > 0)
{
if (xterm_display_allocated)
vim_free(xterm_display);
xterm_display = (char *)vim_strsave(eap->arg);
xterm_display_allocated = TRUE;
}
smsg(_("restoring display %s"), xterm_display == NULL
? (char *)mch_getenv((char_u *)"DISPLAY") : xterm_display);

clear_xterm_clip();
x11_window = 0;
xterm_dpy_retry_count = 5; 
may_restore_clipboard();
}
#endif








static int
test_x11_window(Display *dpy)
{
int (*old_handler)();
XTextProperty text_prop;

old_handler = XSetErrorHandler(x_error_check);
got_x_error = FALSE;
if (XGetWMName(dpy, x11_window, &text_prop))
XFree((void *)text_prop.value);
XSync(dpy, False);
(void)XSetErrorHandler(old_handler);

if (p_verbose > 0 && got_x_error)
verb_msg(_("Testing the X display failed"));

return (got_x_error ? FAIL : OK);
}
#endif

#if defined(FEAT_TITLE)

#if defined(FEAT_X11)

static int get_x11_thing(int get_title, int test_only);






static int
get_x11_windis(void)
{
char *winid;
static int result = -1;
#define XD_NONE 0 
#define XD_HERE 1 
#define XD_GUI 2 
#define XD_XTERM 3 
static int x11_display_from = XD_NONE;
static int did_set_error_handler = FALSE;

if (!did_set_error_handler)
{

(void)XSetErrorHandler(x_error_handler);
did_set_error_handler = TRUE;
}

#if defined(FEAT_GUI_X11) || defined(FEAT_GUI_GTK)
if (gui.in_use)
{




if (x11_display_from == XD_HERE && x11_display != NULL)
{
XCloseDisplay(x11_display);
x11_display_from = XD_NONE;
}
if (gui_get_x11_windis(&x11_window, &x11_display) == OK)
{
x11_display_from = XD_GUI;
return OK;
}
x11_display = NULL;
return FAIL;
}
else if (x11_display_from == XD_GUI)
{

x11_window = 0;
x11_display = NULL;
x11_display_from = XD_NONE;
}
#endif


if (!x_connect_to_server())
return FAIL;







if (x11_window == 0 && (winid = getenv("WINDOWID")) != NULL)
x11_window = (Window)atol(winid);

#if defined(FEAT_XCLIPBOARD)
if (xterm_dpy == x11_display)

x11_display_from = XD_XTERM;

if (xterm_dpy != NULL && x11_window != 0)
{


if (x11_display_from != XD_XTERM)
{




if (x11_display_from == XD_HERE && x11_display != NULL)
XCloseDisplay(x11_display);
x11_display = xterm_dpy;
x11_display_from = XD_XTERM;
}
if (test_x11_window(x11_display) == FAIL)
{

x11_window = 0;
x11_display = NULL;
x11_display_from = XD_NONE;
return FAIL;
}
return OK;
}
#endif

if (x11_window == 0 || x11_display == NULL)
result = -1;

if (result != -1) 
return result; 

if (x11_window != 0 && x11_display == NULL)
{
#if defined(SET_SIG_ALARM)
RETSIGTYPE (*sig_save)();
#endif
#if defined(ELAPSED_FUNC)
elapsed_T start_tv;

if (p_verbose > 0)
ELAPSED_INIT(start_tv);
#endif

#if defined(SET_SIG_ALARM)




sig_alarm_called = FALSE;
sig_save = (RETSIGTYPE (*)())signal(SIGALRM,
(RETSIGTYPE (*)())sig_alarm);
alarm(2);
#endif
x11_display = XOpenDisplay(NULL);

#if defined(SET_SIG_ALARM)
alarm(0);
signal(SIGALRM, (RETSIGTYPE (*)())sig_save);
if (p_verbose > 0 && sig_alarm_called)
verb_msg(_("Opening the X display timed out"));
#endif
if (x11_display != NULL)
{
#if defined(ELAPSED_FUNC)
if (p_verbose > 0)
{
verbose_enter();
xopen_message(ELAPSED_FUNC(start_tv));
verbose_leave();
}
#endif
if (test_x11_window(x11_display) == FAIL)
{

x11_window = 0;
XCloseDisplay(x11_display);
x11_display = NULL;
}
else
x11_display_from = XD_HERE;
}
}
if (x11_window == 0 || x11_display == NULL)
return (result = FAIL);

#if defined(FEAT_EVAL)
set_vim_var_nr(VV_WINDOWID, (long)x11_window);
#endif

return (result = OK);
}




static int
get_x11_title(int test_only)
{
return get_x11_thing(TRUE, test_only);
}




static int
get_x11_icon(int test_only)
{
int retval = FALSE;

retval = get_x11_thing(FALSE, test_only);


if (oldicon == NULL && !test_only)
{
if (STRNCMP(T_NAME, "builtin_", 8) == 0)
oldicon = vim_strsave(T_NAME + 8);
else
oldicon = vim_strsave(T_NAME);
}

return retval;
}

static int
get_x11_thing(
int get_title, 
int test_only)
{
XTextProperty text_prop;
int retval = FALSE;
Status status;

if (get_x11_windis() == OK)
{

if (get_title)
status = XGetWMName(x11_display, x11_window, &text_prop);
else
status = XGetWMIconName(x11_display, x11_window, &text_prop);










{
Window root;
Window parent;
Window win = x11_window;
Window *children;
unsigned int num_children;

while (!status || text_prop.value == NULL)
{
if (!XQueryTree(x11_display, win, &root, &parent, &children,
&num_children))
break;
if (children)
XFree((void *)children);
if (parent == root || parent == 0)
break;

win = parent;
if (get_title)
status = XGetWMName(x11_display, win, &text_prop);
else
status = XGetWMIconName(x11_display, win, &text_prop);
}
}
if (status && text_prop.value != NULL)
{
retval = TRUE;
if (!test_only)
{
if (get_title)
vim_free(oldtitle);
else
vim_free(oldicon);
if (text_prop.encoding == XA_STRING && !has_mbyte)
{
if (get_title)
oldtitle = vim_strsave((char_u *)text_prop.value);
else
oldicon = vim_strsave((char_u *)text_prop.value);
}
else
{
char **cl;
Status transform_status;
int n = 0;

transform_status = XmbTextPropertyToTextList(x11_display,
&text_prop,
&cl, &n);
if (transform_status >= Success && n > 0 && cl[0])
{
if (get_title)
oldtitle = vim_strsave((char_u *) cl[0]);
else
oldicon = vim_strsave((char_u *) cl[0]);
XFreeStringList(cl);
}
else
{
if (get_title)
oldtitle = vim_strsave((char_u *)text_prop.value);
else
oldicon = vim_strsave((char_u *)text_prop.value);
}
}
}
XFree((void *)text_prop.value);
}
}
return retval;
}





#if defined(X_HAVE_UTF8_STRING)
#if X_HAVE_UTF8_STRING && HAVE_XUTF8SETWMPROPERTIES
#define USE_UTF8_STRING
#endif
#endif






static void
set_x11_title(char_u *title)
{



#if defined(USE_UTF8_STRING)
if (enc_utf8)
Xutf8SetWMProperties(x11_display, x11_window, (const char *)title,
NULL, NULL, 0, NULL, NULL, NULL);
else
#endif
{
#if XtSpecificationRelease >= 4
#if defined(FEAT_XFONTSET)
XmbSetWMProperties(x11_display, x11_window, (const char *)title,
NULL, NULL, 0, NULL, NULL, NULL);
#else
XTextProperty text_prop;
char *c_title = (char *)title;


(void)XStringListToTextProperty(&c_title, 1, &text_prop);
XSetWMProperties(x11_display, x11_window, &text_prop,
NULL, NULL, 0, NULL, NULL, NULL);
#endif
#else
XStoreName(x11_display, x11_window, (char *)title);
#endif
}
XFlush(x11_display);
}






static void
set_x11_icon(char_u *icon)
{

#if defined(USE_UTF8_STRING)
if (enc_utf8)
Xutf8SetWMProperties(x11_display, x11_window, NULL, (const char *)icon,
NULL, 0, NULL, NULL, NULL);
else
#endif
{
#if XtSpecificationRelease >= 4
#if defined(FEAT_XFONTSET)
XmbSetWMProperties(x11_display, x11_window, NULL, (const char *)icon,
NULL, 0, NULL, NULL, NULL);
#else
XTextProperty text_prop;
char *c_icon = (char *)icon;

(void)XStringListToTextProperty(&c_icon, 1, &text_prop);
XSetWMProperties(x11_display, x11_window, NULL, &text_prop,
NULL, 0, NULL, NULL, NULL);
#endif
#else
XSetIconName(x11_display, x11_window, (char *)icon);
#endif
}
XFlush(x11_display);
}

#else 

static int
get_x11_title(int test_only UNUSED)
{
return FALSE;
}

static int
get_x11_icon(int test_only)
{
if (!test_only)
{
if (STRNCMP(T_NAME, "builtin_", 8) == 0)
oldicon = vim_strsave(T_NAME + 8);
else
oldicon = vim_strsave(T_NAME);
}
return FALSE;
}

#endif 

int
mch_can_restore_title(void)
{
return get_x11_title(TRUE);
}

int
mch_can_restore_icon(void)
{
return get_x11_icon(TRUE);
}




void
mch_settitle(char_u *title, char_u *icon)
{
int type = 0;
static int recursive = 0;

if (T_NAME == NULL) 
return;
if (title == NULL && icon == NULL) 
return;



if (recursive)
return;
++recursive;




#if defined(FEAT_X11)
if (get_x11_windis() == OK)
type = 1;
#else
#if defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_MAC) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_HAIKU)

if (gui.in_use)
type = 1;
#endif
#endif





if ((type || *T_TS != NUL) && title != NULL)
{
if (oldtitle_outdated)
{
oldtitle_outdated = FALSE;
VIM_CLEAR(oldtitle);
}
if (oldtitle == NULL
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
) 
(void)get_x11_title(FALSE);

if (*T_TS != NUL) 
term_settitle(title);
#if defined(FEAT_X11)
else
#if defined(FEAT_GUI_GTK)
if (!gui.in_use) 
#endif
set_x11_title(title); 
#endif
#if defined(FEAT_GUI_GTK) || defined(FEAT_GUI_HAIKU) || defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_MAC)

else
gui_mch_settitle(title, icon);
#endif
unix_did_set_title = TRUE;
}

if ((type || *T_CIS != NUL) && icon != NULL)
{
if (oldicon == NULL
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
) 
get_x11_icon(FALSE);

if (*T_CIS != NUL)
{
out_str(T_CIS); 
out_str_nf(icon);
out_str(T_CIE); 
out_flush();
}
#if defined(FEAT_X11)
else
#if defined(FEAT_GUI_GTK)
if (!gui.in_use) 
#endif
set_x11_icon(icon); 
#endif
did_set_icon = TRUE;
}
--recursive;
}








void
mch_restore_title(int which)
{
int do_push_pop = unix_did_set_title || did_set_icon;


mch_settitle(((which & SAVE_RESTORE_TITLE) && unix_did_set_title) ?
(oldtitle ? oldtitle : p_titleold) : NULL,
((which & SAVE_RESTORE_ICON) && did_set_icon) ? oldicon : NULL);

if (do_push_pop)
{

term_pop_title(which);
term_push_title(which);
}
}

#endif 





int
vim_is_xterm(char_u *name)
{
if (name == NULL)
return FALSE;
return (STRNICMP(name, "xterm", 5) == 0
|| STRNICMP(name, "nxterm", 6) == 0
|| STRNICMP(name, "kterm", 5) == 0
|| STRNICMP(name, "mlterm", 6) == 0
|| STRNICMP(name, "rxvt", 4) == 0
|| STRNICMP(name, "screen.xterm", 12) == 0
|| STRCMP(name, "builtin_xterm") == 0);
}

#if defined(FEAT_MOUSE_XTERM) || defined(PROTO)





int
use_xterm_like_mouse(char_u *name)
{
return (name != NULL
&& (term_is_xterm
|| STRNICMP(name, "screen", 6) == 0
|| STRNICMP(name, "tmux", 4) == 0
|| STRICMP(name, "st") == 0
|| STRNICMP(name, "st-", 3) == 0
|| STRNICMP(name, "stterm", 6) == 0));
}
#endif








int
use_xterm_mouse(void)
{
if (ttym_flags == TTYM_SGR)
return 4;
if (ttym_flags == TTYM_URXVT)
return 3;
if (ttym_flags == TTYM_XTERM2)
return 2;
if (ttym_flags == TTYM_XTERM)
return 1;
return 0;
}

int
vim_is_iris(char_u *name)
{
if (name == NULL)
return FALSE;
return (STRNICMP(name, "iris-ansi", 9) == 0
|| STRCMP(name, "builtin_iris-ansi") == 0);
}

int
vim_is_vt300(char_u *name)
{
if (name == NULL)
return FALSE; 

return ((STRNICMP(name, "vt", 2) == 0
&& vim_strchr((char_u *)"12345", name[2]) != NULL)
|| STRCMP(name, "builtin_vt320") == 0);
}





int
vim_is_fastterm(char_u *name)
{
if (name == NULL)
return FALSE;
if (vim_is_xterm(name) || vim_is_vt300(name) || vim_is_iris(name))
return TRUE;
return ( STRNICMP(name, "hpterm", 6) == 0
|| STRNICMP(name, "sun-cmd", 7) == 0
|| STRNICMP(name, "screen", 6) == 0
|| STRNICMP(name, "tmux", 4) == 0
|| STRNICMP(name, "dtterm", 6) == 0);
}





int
mch_get_user_name(char_u *s, int len)
{
#if defined(VMS)
vim_strncpy(s, (char_u *)cuserid(NULL), len - 1);
return OK;
#else
return mch_get_uname(getuid(), s, len);
#endif
}





int
mch_get_uname(uid_t uid, char_u *s, int len)
{
#if defined(HAVE_PWD_H) && defined(HAVE_GETPWUID)
struct passwd *pw;

if ((pw = getpwuid(uid)) != NULL
&& pw->pw_name != NULL && *(pw->pw_name) != NUL)
{
vim_strncpy(s, (char_u *)pw->pw_name, len - 1);
return OK;
}
#endif
sprintf((char *)s, "%d", (int)uid); 
return FAIL; 
}





#if defined(HAVE_SYS_UTSNAME_H)
void
mch_get_host_name(char_u *s, int len)
{
struct utsname vutsname;

if (uname(&vutsname) < 0)
*s = NUL;
else
vim_strncpy(s, (char_u *)vutsname.nodename, len - 1);
}
#else 

#if defined(HAVE_SYS_SYSTEMINFO_H)
#define gethostname(nam, len) sysinfo(SI_HOSTNAME, nam, len)
#endif

void
mch_get_host_name(char_u *s, int len)
{
#if defined(VAXC)
vaxc$gethostname((char *)s, len);
#else
gethostname((char *)s, len);
#endif
s[len - 1] = NUL; 
}
#endif 




long
mch_get_pid(void)
{
return (long)getpid();
}




int
mch_process_running(long pid)
{

return kill(pid, 0) == 0;
}

#if !defined(HAVE_STRERROR) && defined(USE_GETCWD)
static char *
strerror(int err)
{
extern int sys_nerr;
extern char *sys_errlist[];
static char er[20];

if (err > 0 && err < sys_nerr)
return (sys_errlist[err]);
sprintf(er, "Error %d", err);
return er;
}
#endif






int
mch_dirname(char_u *buf, int len)
{
#if defined(USE_GETCWD)
if (getcwd((char *)buf, len) == NULL)
{
STRCPY(buf, strerror(errno));
return FAIL;
}
return OK;
#else
return (getwd((char *)buf) != NULL ? OK : FAIL);
#endif
}






int
mch_FullName(
char_u *fname,
char_u *buf,
int len,
int force) 
{
int l;
#if defined(HAVE_FCHDIR)
int fd = -1;
static int dont_fchdir = FALSE; 
#endif
char_u olddir[MAXPATHL];
char_u *p;
int retval = OK;
#if defined(__CYGWIN__)
char_u posix_fname[MAXPATHL]; 

#endif

#if defined(VMS)
fname = vms_fixfilename(fname);
#endif

#if defined(__CYGWIN__)



#if CYGWIN_VERSION_DLL_MAJOR >= 1007


cygwin_conv_path(CCP_WIN_A_TO_POSIX | CCP_RELATIVE,
fname, posix_fname, MAXPATHL);
#else
cygwin_conv_to_posix_path(fname, posix_fname);
#endif
fname = posix_fname;
#endif



if ((force || !mch_isFullName(fname))
&& ((p = vim_strrchr(fname, '/')) == NULL || p != fname))
{





if (p != NULL)
{
#if defined(HAVE_FCHDIR)





if (!dont_fchdir)
{
fd = open(".", O_RDONLY | O_EXTRA, 0);
if (fd >= 0 && fchdir(fd) < 0)
{
close(fd);
fd = -1;
dont_fchdir = TRUE; 
}
}
#endif



if (
#if defined(HAVE_FCHDIR)
fd < 0 &&
#endif
(mch_dirname(olddir, MAXPATHL) == FAIL
|| mch_chdir((char *)olddir) != 0))
{
p = NULL; 
retval = FAIL;
}
else
{



if (p - fname >= len)
retval = FAIL;
else
{
vim_strncpy(buf, fname, p - fname);
if (mch_chdir((char *)buf))
retval = FAIL;
else
fname = p + 1;
*buf = NUL;
}
}
}
if (mch_dirname(buf, len) == FAIL)
{
retval = FAIL;
*buf = NUL;
}
if (p != NULL)
{
#if defined(HAVE_FCHDIR)
if (fd >= 0)
{
if (p_verbose >= 5)
{
verbose_enter();
msg("fchdir() to previous dir");
verbose_leave();
}
l = fchdir(fd);
close(fd);
}
else
#endif
l = mch_chdir((char *)olddir);
if (l != 0)
emsg(_(e_prev_dir));
}

l = STRLEN(buf);
if (l >= len - 1)
retval = FAIL; 
#if !defined(VMS)
else if (l > 0 && buf[l - 1] != '/' && *fname != NUL
&& STRCMP(fname, ".") != 0)
STRCAT(buf, "/");
#endif
}


if (retval == FAIL || (int)(STRLEN(buf) + STRLEN(fname)) >= len)
return FAIL;


if (STRCMP(fname, ".") != 0)
STRCAT(buf, fname);

return OK;
}




int
mch_isFullName(char_u *fname)
{
#if defined(VMS)
return ( fname[0] == '/' || fname[0] == '.' ||
strchr((char *)fname,':') || strchr((char *)fname,'"') ||
(strchr((char *)fname,'[') && strchr((char *)fname,']'))||
(strchr((char *)fname,'<') && strchr((char *)fname,'>')) );
#else
return (*fname == '/' || *fname == '~');
#endif
}

#if defined(USE_FNAME_CASE) || defined(PROTO)





void
fname_case(
char_u *name,
int len UNUSED) 
{
struct stat st;
char_u *slash, *tail;
DIR *dirp;
struct dirent *dp;

if (mch_lstat((char *)name, &st) >= 0)
{

slash = vim_strrchr(name, '/');
if (slash == NULL)
{
dirp = opendir(".");
tail = name;
}
else
{
*slash = NUL;
dirp = opendir((char *)name);
*slash = '/';
tail = slash + 1;
}

if (dirp != NULL)
{
while ((dp = readdir(dirp)) != NULL)
{


if (STRICMP(tail, dp->d_name) == 0
&& STRLEN(tail) == STRLEN(dp->d_name))
{
char_u newname[MAXPATHL + 1];
struct stat st2;


vim_strncpy(newname, name, MAXPATHL);
vim_strncpy(newname + (tail - name), (char_u *)dp->d_name,
MAXPATHL - (tail - name));
if (mch_lstat((char *)newname, &st2) >= 0
&& st.st_ino == st2.st_ino
&& st.st_dev == st2.st_dev)
{
STRCPY(tail, dp->d_name);
break;
}
}
}

closedir(dirp);
}
}
}
#endif





long
mch_getperm(char_u *name)
{
struct stat statb;


#if defined(VMS)
if (stat((char *)vms_fixfilename(name), &statb))
#else
if (stat((char *)name, &statb))
#endif
return -1;
#if defined(__INTERIX)


return statb.st_mode & ~S_ADDACE;
#else
return statb.st_mode;
#endif
}





int
mch_setperm(char_u *name, long perm)
{
return (chmod((char *)
#if defined(VMS)
vms_fixfilename(name),
#else
name,
#endif
(mode_t)perm) == 0 ? OK : FAIL);
}

#if defined(HAVE_FCHMOD) || defined(PROTO)




int
mch_fsetperm(int fd, long perm)
{
return (fchmod(fd, (mode_t)perm) == 0 ? OK : FAIL);
}
#endif

#if defined(HAVE_ACL) || defined(PROTO)
#if defined(HAVE_SYS_ACL_H)
#include <sys/acl.h>
#endif
#if defined(HAVE_SYS_ACCESS_H)
#include <sys/access.h>
#endif

#if defined(HAVE_SOLARIS_ACL)
typedef struct vim_acl_solaris_T {
int acl_cnt;
aclent_t *acl_entry;
} vim_acl_solaris_T;
#endif

#if defined(HAVE_SELINUX) || defined(PROTO)



void
mch_copy_sec(char_u *from_file, char_u *to_file)
{
if (from_file == NULL)
return;

if (selinux_enabled == -1)
selinux_enabled = is_selinux_enabled();

if (selinux_enabled > 0)
{
security_context_t from_context = NULL;
security_context_t to_context = NULL;

if (getfilecon((char *)from_file, &from_context) < 0)
{



if (errno == EOPNOTSUPP)
return;

msg_puts(_("\nCould not get security context for "));
msg_outtrans(from_file);
msg_putchar('\n');
return;
}
if (getfilecon((char *)to_file, &to_context) < 0)
{
msg_puts(_("\nCould not get security context for "));
msg_outtrans(to_file);
msg_putchar('\n');
freecon (from_context);
return ;
}
if (strcmp(from_context, to_context) != 0)
{
if (setfilecon((char *)to_file, from_context) < 0)
{
msg_puts(_("\nCould not set security context for "));
msg_outtrans(to_file);
msg_putchar('\n');
}
}
freecon(to_context);
freecon(from_context);
}
}
#endif 

#if defined(HAVE_SMACK) && !defined(PROTO)



void
mch_copy_sec(char_u *from_file, char_u *to_file)
{
static const char * const smack_copied_attributes[] =
{
XATTR_NAME_SMACK,
XATTR_NAME_SMACKEXEC,
XATTR_NAME_SMACKMMAP
};

char buffer[SMACK_LABEL_LEN];
const char *name;
int index;
int ret;
ssize_t size;

if (from_file == NULL)
return;

for (index = 0 ; index < (int)(sizeof(smack_copied_attributes)
/ sizeof(smack_copied_attributes)[0]) ; index++)
{

name = smack_copied_attributes[index];


size = getxattr((char*)from_file, name, buffer, sizeof(buffer));
if (size >= 0)
{

ret = setxattr((char*)to_file, name, buffer, (size_t)size, 0);
if (ret < 0)
{
vim_snprintf((char *)IObuff, IOSIZE,
_("Could not set security context %s for %s"),
name, to_file);
msg_outtrans(IObuff);
msg_putchar('\n');
}
}
else
{

switch (errno)
{
case ENOTSUP:


return; 

case ERANGE:
default:

vim_snprintf((char *)IObuff, IOSIZE,
_("Could not get security context %s for %s. Removing it!"),
name, from_file);
msg_puts((char *)IObuff);
msg_putchar('\n');


case ENODATA:

ret = removexattr((char*)to_file, name);


break;
}
}
}
}
#endif 





vim_acl_T
mch_get_acl(char_u *fname UNUSED)
{
vim_acl_T ret = NULL;
#if defined(HAVE_POSIX_ACL)
ret = (vim_acl_T)acl_get_file((char *)fname, ACL_TYPE_ACCESS);
#else
#if defined(HAVE_SOLARIS_ZFS_ACL)
acl_t *aclent;

if (acl_get((char *)fname, 0, &aclent) < 0)
return NULL;
ret = (vim_acl_T)aclent;
#else
#if defined(HAVE_SOLARIS_ACL)
vim_acl_solaris_T *aclent;

aclent = malloc(sizeof(vim_acl_solaris_T));
if ((aclent->acl_cnt = acl((char *)fname, GETACLCNT, 0, NULL)) < 0)
{
free(aclent);
return NULL;
}
aclent->acl_entry = malloc(aclent->acl_cnt * sizeof(aclent_t));
if (acl((char *)fname, GETACL, aclent->acl_cnt, aclent->acl_entry) < 0)
{
free(aclent->acl_entry);
free(aclent);
return NULL;
}
ret = (vim_acl_T)aclent;
#else
#if defined(HAVE_AIX_ACL)
int aclsize;
struct acl *aclent;

aclsize = sizeof(struct acl);
aclent = malloc(aclsize);
if (statacl((char *)fname, STX_NORMAL, aclent, aclsize) < 0)
{
if (errno == ENOSPC)
{
aclsize = aclent->acl_len;
aclent = realloc(aclent, aclsize);
if (statacl((char *)fname, STX_NORMAL, aclent, aclsize) < 0)
{
free(aclent);
return NULL;
}
}
else
{
free(aclent);
return NULL;
}
}
ret = (vim_acl_T)aclent;
#endif 
#endif 
#endif 
#endif 
return ret;
}




void
mch_set_acl(char_u *fname UNUSED, vim_acl_T aclent)
{
if (aclent == NULL)
return;
#if defined(HAVE_POSIX_ACL)
acl_set_file((char *)fname, ACL_TYPE_ACCESS, (acl_t)aclent);
#else
#if defined(HAVE_SOLARIS_ZFS_ACL)
acl_set((char *)fname, (acl_t *)aclent);
#else
#if defined(HAVE_SOLARIS_ACL)
acl((char *)fname, SETACL, ((vim_acl_solaris_T *)aclent)->acl_cnt,
((vim_acl_solaris_T *)aclent)->acl_entry);
#else
#if defined(HAVE_AIX_ACL)
chacl((char *)fname, aclent, ((struct acl *)aclent)->acl_len);
#endif 
#endif 
#endif 
#endif 
}

void
mch_free_acl(vim_acl_T aclent)
{
if (aclent == NULL)
return;
#if defined(HAVE_POSIX_ACL)
acl_free((acl_t)aclent);
#else
#if defined(HAVE_SOLARIS_ZFS_ACL)
acl_free((acl_t *)aclent);
#else
#if defined(HAVE_SOLARIS_ACL)
free(((vim_acl_solaris_T *)aclent)->acl_entry);
free(aclent);
#else
#if defined(HAVE_AIX_ACL)
free(aclent);
#endif 
#endif 
#endif 
#endif 
}
#endif




void
mch_hide(char_u *name UNUSED)
{

}






int
mch_isdir(char_u *name)
{
struct stat statb;

if (*name == NUL) 
return FALSE;
if (stat((char *)name, &statb))
return FALSE;
return (S_ISDIR(statb.st_mode) ? TRUE : FALSE);
}






int
mch_isrealdir(char_u *name)
{
struct stat statb;

if (*name == NUL) 
return FALSE;
if (mch_lstat((char *)name, &statb))
return FALSE;
return (S_ISDIR(statb.st_mode) ? TRUE : FALSE);
}




static int
executable_file(char_u *name)
{
struct stat st;

if (stat((char *)name, &st))
return 0;
#if defined(VMS)







int vms_executable = 0;
if (S_ISREG(st.st_mode) && mch_access((char *)name, X_OK) == 0)
{
if (strstr(vms_tolower((char*)name),".exe") != NULL
|| strstr(vms_tolower((char*)name),".com")!= NULL)
vms_executable = 1;
}
return vms_executable;
#else
return S_ISREG(st.st_mode) && mch_access((char *)name, X_OK) == 0;
#endif
}






int
mch_can_exe(char_u *name, char_u **path, int use_path)
{
char_u *buf;
char_u *p, *e;
int retval;



if (!use_path || gettail(name) != name)
{


if ((use_path || gettail(name) != name) && executable_file(name))
{
if (path != NULL)
{
if (name[0] != '/')
*path = FullName_save(name, TRUE);
else
*path = vim_strsave(name);
}
return TRUE;
}
return FALSE;
}

p = (char_u *)getenv("PATH");
if (p == NULL || *p == NUL)
return -1;
buf = alloc(STRLEN(name) + STRLEN(p) + 2);
if (buf == NULL)
return -1;





for (;;)
{
e = (char_u *)strchr((char *)p, ':');
if (e == NULL)
e = p + STRLEN(p);
if (e - p <= 1) 
STRCPY(buf, "./");
else
{
vim_strncpy(buf, p, e - p);
add_pathsep(buf);
}
STRCAT(buf, name);
retval = executable_file(buf);
if (retval == 1)
{
if (path != NULL)
{
if (buf[0] != '/')
*path = FullName_save(buf, TRUE);
else
*path = vim_strsave(buf);
}
break;
}

if (*e != ':')
break;
p = e + 1;
}

vim_free(buf);
return retval;
}







int
mch_nodetype(char_u *name)
{
struct stat st;

if (stat((char *)name, &st))
return NODE_NORMAL;
if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))
return NODE_NORMAL;
if (S_ISBLK(st.st_mode)) 
return NODE_OTHER;

return NODE_WRITABLE;
}

void
mch_early_init(void)
{
#if defined(HAVE_CHECK_STACK_GROWTH)
int i;

check_stack_growth((char *)&i);

#if defined(HAVE_STACK_LIMIT)
get_stack_limit();
#endif

#endif







#if defined(HAVE_SIGALTSTACK) || defined(HAVE_SIGSTACK)
signal_stack = alloc(SIGSTKSZ);
init_signal_stack();
#endif
}

#if defined(EXITFREE) || defined(PROTO)
void
mch_free_mem(void)
{
#if defined(FEAT_CLIPBOARD) && defined(FEAT_X11)
if (clip_star.owned)
clip_lose_selection(&clip_star);
if (clip_plus.owned)
clip_lose_selection(&clip_plus);
#endif
#if defined(FEAT_X11) && defined(FEAT_XCLIPBOARD)
if (xterm_Shell != (Widget)0)
XtDestroyWidget(xterm_Shell);
#if !defined(LESSTIF_VERSION)

if (xterm_dpy != NULL)
XtCloseDisplay(xterm_dpy);
if (app_context != (XtAppContext)NULL)
{
XtDestroyApplicationContext(app_context);
#if defined(FEAT_X11)
x11_display = NULL; 
#endif
}
#endif
#endif
#if defined(FEAT_X11)
if (x11_display != NULL
#if defined(FEAT_XCLIPBOARD)
&& x11_display != xterm_dpy
#endif
)
XCloseDisplay(x11_display);
#endif
#if defined(HAVE_SIGALTSTACK) || defined(HAVE_SIGSTACK)
VIM_CLEAR(signal_stack);
#endif
#if defined(FEAT_TITLE)
vim_free(oldtitle);
vim_free(oldicon);
#endif
}
#endif





static void
exit_scroll(void)
{
if (silent_mode)
return;
if (newline_on_exit || msg_didout)
{
if (msg_use_printf())
{
if (info_message)
mch_msg("\n");
else
mch_errmsg("\r\n");
}
else
out_char('\n');
}
else
{
restore_cterm_colors(); 
msg_clr_eos_force(); 
windgoto((int)Rows - 1, 0); 
}
}

void
mch_exit(int r)
{
exiting = TRUE;

#if defined(FEAT_X11) && defined(FEAT_CLIPBOARD)
x11_export_final_selection();
#endif

#if defined(FEAT_GUI)
if (!gui.in_use)
#endif
{
settmode(TMODE_COOK);
#if defined(FEAT_TITLE)

mch_restore_title(SAVE_RESTORE_BOTH);
term_pop_title(SAVE_RESTORE_BOTH);
#endif






if (swapping_screen() && !newline_on_exit)
exit_scroll();



stoptermcap();





if (!swapping_screen() || newline_on_exit)
exit_scroll();



if (full_screen)
cursor_on();
}
out_flush();
ml_close_all(TRUE); 
may_core_dump();
#if defined(FEAT_GUI)
if (gui.in_use)
gui_exit(r);
#endif

#if defined(MACOS_CONVERT)
mac_conv_cleanup();
#endif

#if defined(__QNX__)


if (deadly_signal != 0)
return;
#endif

#if defined(FEAT_NETBEANS_INTG)
netbeans_send_disconnect();
#endif

#if defined(EXITFREE)
free_all_mem();
#endif

exit(r);
}

static void
may_core_dump(void)
{
if (deadly_signal != 0)
{
signal(deadly_signal, SIG_DFL);
kill(getpid(), deadly_signal); 
}
}

#if !defined(VMS)




static int
get_tty_fd(int fd)
{
int tty_fd = fd;

#if defined(HAVE_SVR4_PTYS) && defined(SUN_SYSTEM)


if (mch_isatty(fd) > 1)
{
char *name;

name = ptsname(fd);
if (name == NULL)
return -1;

tty_fd = open(name, O_RDONLY | O_NOCTTY | O_EXTRA, 0);
if (tty_fd < 0)
return -1;
}
#endif
return tty_fd;
}

static int
mch_tcgetattr(int fd, void *term)
{
int tty_fd;
int retval = -1;

tty_fd = get_tty_fd(fd);
if (tty_fd >= 0)
{
#if defined(NEW_TTY_SYSTEM)
#if defined(HAVE_TERMIOS_H)
retval = tcgetattr(tty_fd, (struct termios *)term);
#else
retval = ioctl(tty_fd, TCGETA, (struct termio *)term);
#endif
#else

retval = ioctl(tty_fd, TIOCGETP, (struct sgttyb *)term);
#endif
if (tty_fd != fd)
close(tty_fd);
}
return retval;
}

void
mch_settmode(int tmode)
{
static int first = TRUE;

#if defined(NEW_TTY_SYSTEM)
#if defined(HAVE_TERMIOS_H)
static struct termios told;
struct termios tnew;
#else
static struct termio told;
struct termio tnew;
#endif

if (first)
{
first = FALSE;
mch_tcgetattr(read_cmd_fd, &told);
}

tnew = told;
if (tmode == TMODE_RAW)
{



tnew.c_iflag &= ~ICRNL;
tnew.c_lflag &= ~(ICANON | ECHO | ISIG | ECHOE
#if defined(IEXTEN) && !defined(__MINT__)
| IEXTEN 

#endif
);
#if defined(ONLCR)


#if defined(XTABS)
tnew.c_oflag &= ~(ONLCR | XTABS);
#else
#if defined(TAB3)
tnew.c_oflag &= ~(ONLCR | TAB3);
#else
tnew.c_oflag &= ~ONLCR;
#endif
#endif
#endif
tnew.c_cc[VMIN] = 1; 
tnew.c_cc[VTIME] = 0; 
}
else if (tmode == TMODE_SLEEP)
{


tnew.c_lflag &= ~(ICANON | ECHO);
tnew.c_cc[VMIN] = 1; 
tnew.c_cc[VTIME] = 0; 
}

#if defined(HAVE_TERMIOS_H)
{
int n = 10;



while (tcsetattr(read_cmd_fd, TCSANOW, &tnew) == -1
&& errno == EINTR && n > 0)
--n;
}
#else
ioctl(read_cmd_fd, TCSETA, &tnew);
#endif

#else



#if !defined(TIOCSETN)
#define TIOCSETN TIOCSETP 
#endif
static struct sgttyb ttybold;
struct sgttyb ttybnew;

if (first)
{
first = FALSE;
mch_tcgetattr(read_cmd_fd, &ttybold);
}

ttybnew = ttybold;
if (tmode == TMODE_RAW)
{
ttybnew.sg_flags &= ~(CRMOD | ECHO);
ttybnew.sg_flags |= RAW;
}
else if (tmode == TMODE_SLEEP)
ttybnew.sg_flags &= ~(ECHO);
ioctl(read_cmd_fd, TIOCSETN, &ttybnew);
#endif
curr_tmode = tmode;
}










void
get_stty(void)
{
ttyinfo_T info;
char_u buf[2];
char_u *p;

if (get_tty_info(read_cmd_fd, &info) == OK)
{
intr_char = info.interrupt;
buf[0] = info.backspace;
buf[1] = NUL;
add_termcode((char_u *)"kb", buf, FALSE);


p = find_termcode((char_u *)"kD");
if (p != NULL && p[0] == buf[0] && p[1] == buf[1])
do_fixdel(NULL);
}
}





int
get_tty_info(int fd, ttyinfo_T *info)
{
#if defined(NEW_TTY_SYSTEM)
#if defined(HAVE_TERMIOS_H)
struct termios keys;
#else
struct termio keys;
#endif

if (mch_tcgetattr(fd, &keys) != -1)
{
info->backspace = keys.c_cc[VERASE];
info->interrupt = keys.c_cc[VINTR];
if (keys.c_iflag & ICRNL)
info->enter = NL;
else
info->enter = CAR;
if (keys.c_oflag & ONLCR)
info->nl_does_cr = TRUE;
else
info->nl_does_cr = FALSE;
return OK;
}
#else

struct sgttyb keys;

if (mch_tcgetattr(fd, &keys) != -1)
{
info->backspace = keys.sg_erase;
info->interrupt = keys.sg_kill;
info->enter = CAR;
info->nl_does_cr = TRUE;
return OK;
}
#endif
return FAIL;
}

#endif 

static int mouse_ison = FALSE;




void
mch_setmouse(int on)
{
#if defined(FEAT_BEVAL_TERM)
static int bevalterm_ison = FALSE;
#endif
int xterm_mouse_vers;

#if defined(FEAT_X11) && defined(FEAT_XCLIPBOARD)
if (!on)


stop_xterm_trace();
#endif

if (on == mouse_ison
#if defined(FEAT_BEVAL_TERM)
&& p_bevalterm == bevalterm_ison
#endif
)

return;

xterm_mouse_vers = use_xterm_mouse();

#if defined(FEAT_MOUSE_URXVT)
if (ttym_flags == TTYM_URXVT)
{
out_str_nf((char_u *)
(on
? IF_EB("\033[?1015h", ESC_STR "[?1015h")
: IF_EB("\033[?1015l", ESC_STR "[?1015l")));
mouse_ison = on;
}
#endif

if (ttym_flags == TTYM_SGR)
{

out_str_nf((char_u *)
(on
? IF_EB("\033[?1006h", ESC_STR "[?1006h")
: IF_EB("\033[?1006l", ESC_STR "[?1006l")));
mouse_ison = on;
}

#if defined(FEAT_BEVAL_TERM)
if (bevalterm_ison != (p_bevalterm && on))
{
bevalterm_ison = (p_bevalterm && on);
if (xterm_mouse_vers > 1 && !bevalterm_ison)

out_str_nf((char_u *)
(IF_EB("\033[?1003l", ESC_STR "[?1003l")));
}
#endif

if (xterm_mouse_vers > 0)
{
if (on) 
out_str_nf((char_u *)
(xterm_mouse_vers > 1
? (
#if defined(FEAT_BEVAL_TERM)
bevalterm_ison
? IF_EB("\033[?1003h", ESC_STR "[?1003h") :
#endif
IF_EB("\033[?1002h", ESC_STR "[?1002h"))
: IF_EB("\033[?1000h", ESC_STR "[?1000h")));
else 
out_str_nf((char_u *)
(xterm_mouse_vers > 1
? IF_EB("\033[?1002l", ESC_STR "[?1002l")
: IF_EB("\033[?1000l", ESC_STR "[?1000l")));
mouse_ison = on;
}

#if defined(FEAT_MOUSE_DEC)
else if (ttym_flags == TTYM_DEC)
{
if (on) 
out_str_nf((char_u *)"\033[1;2'z\033[1;3'{");
else 
out_str_nf((char_u *)"\033['z");
mouse_ison = on;
}
#endif

#if defined(FEAT_MOUSE_GPM)
else
{
if (on)
{
if (gpm_open())
mouse_ison = TRUE;
}
else
{
gpm_close();
mouse_ison = FALSE;
}
}
#endif

#if defined(FEAT_SYSMOUSE)
else
{
if (on)
{
if (sysmouse_open() == OK)
mouse_ison = TRUE;
}
else
{
sysmouse_close();
mouse_ison = FALSE;
}
}
#endif

#if defined(FEAT_MOUSE_JSB)
else
{
if (on)
{














#if defined(JSBTERM_MOUSE_NONADVANCED)

out_str_nf((char_u *)IF_EB("\033[0~ZwLMRK1Q\033\\",
ESC_STR "[0~ZwLMRK1Q" ESC_STR "\\"));
#else
out_str_nf((char_u *)IF_EB("\033[0~ZwLMRK+1Q\033\\",
ESC_STR "[0~ZwLMRK+1Q" ESC_STR "\\"));
#endif
mouse_ison = TRUE;
}
else
{
out_str_nf((char_u *)IF_EB("\033[0~ZwQ\033\\",
ESC_STR "[0~ZwQ" ESC_STR "\\"));
mouse_ison = FALSE;
}
}
#endif
#if defined(FEAT_MOUSE_PTERM)
else
{

if (on)
out_str_nf("\033[>1h\033[>6h\033[>7h\033[>1h\033[>9l");
else
out_str_nf("\033[>1l\033[>6l\033[>7l\033[>1l\033[>9h");
mouse_ison = on;
}
#endif
}

#if defined(FEAT_BEVAL_TERM) || defined(PROTO)



void
mch_bevalterm_changed(void)
{
mch_setmouse(mouse_ison);
}
#endif




void
check_mouse_termcode(void)
{
#if defined(FEAT_MOUSE_XTERM)
if (use_xterm_mouse()
#if defined(FEAT_MOUSE_URXVT)
&& use_xterm_mouse() != 3
#endif
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
{
set_mouse_termcode(KS_MOUSE, (char_u *)(term_is_8bit(T_NAME)
? IF_EB("\233M", CSI_STR "M")
: IF_EB("\033[M", ESC_STR "[M")));
if (*p_mouse != NUL)
{


mch_setmouse(FALSE);
setmouse();
}
}
else
del_mouse_termcode(KS_MOUSE);
#endif

#if defined(FEAT_MOUSE_GPM)
if (!use_xterm_mouse()
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
set_mouse_termcode(KS_GPM_MOUSE,
(char_u *)IF_EB("\033MG", ESC_STR "MG"));
else
del_mouse_termcode(KS_GPM_MOUSE);
#endif

#if defined(FEAT_SYSMOUSE)
if (!use_xterm_mouse()
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
set_mouse_termcode(KS_MOUSE, (char_u *)IF_EB("\033MS", ESC_STR "MS"));
#endif

#if defined(FEAT_MOUSE_JSB)

if (!use_xterm_mouse()
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
set_mouse_termcode(KS_JSBTERM_MOUSE,
(char_u *)IF_EB("\033[0~zw", ESC_STR "[0~zw"));
else
del_mouse_termcode(KS_JSBTERM_MOUSE);
#endif

#if defined(FEAT_MOUSE_NET)


if (!use_xterm_mouse()
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
set_mouse_termcode(KS_NETTERM_MOUSE,
(char_u *)IF_EB("\033}", ESC_STR "}"));
else
del_mouse_termcode(KS_NETTERM_MOUSE);
#endif

#if defined(FEAT_MOUSE_DEC)

if (!use_xterm_mouse()
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
set_mouse_termcode(KS_DEC_MOUSE, (char_u *)(term_is_8bit(T_NAME)
? IF_EB("\233", CSI_STR) : IF_EB("\033[", ESC_STR "[")));
else
del_mouse_termcode(KS_DEC_MOUSE);
#endif
#if defined(FEAT_MOUSE_PTERM)

if (!use_xterm_mouse()
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
set_mouse_termcode(KS_PTERM_MOUSE,
(char_u *) IF_EB("\033[", ESC_STR "["));
else
del_mouse_termcode(KS_PTERM_MOUSE);
#endif
#if defined(FEAT_MOUSE_URXVT)
if (use_xterm_mouse() == 3
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
{
set_mouse_termcode(KS_URXVT_MOUSE, (char_u *)(term_is_8bit(T_NAME)
? IF_EB("\233*M", CSI_STR "*M")
: IF_EB("\033[*M", ESC_STR "[*M")));

if (*p_mouse != NUL)
{
mch_setmouse(FALSE);
setmouse();
}
}
else
del_mouse_termcode(KS_URXVT_MOUSE);
#endif
if (use_xterm_mouse() == 4
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
{
set_mouse_termcode(KS_SGR_MOUSE, (char_u *)(term_is_8bit(T_NAME)
? IF_EB("\233<*M", CSI_STR "<*M")
: IF_EB("\033[<*M", ESC_STR "[<*M")));

set_mouse_termcode(KS_SGR_MOUSE_RELEASE, (char_u *)(term_is_8bit(T_NAME)
? IF_EB("\233<*m", CSI_STR "<*m")
: IF_EB("\033[<*m", ESC_STR "[<*m")));

if (*p_mouse != NUL)
{
mch_setmouse(FALSE);
setmouse();
}
}
else
{
del_mouse_termcode(KS_SGR_MOUSE);
del_mouse_termcode(KS_SGR_MOUSE_RELEASE);
}
}

#if !defined(VMS)









int
mch_get_shellsize(void)
{
long rows = 0;
long columns = 0;
char_u *p;







#if defined(TIOCGWINSZ)
{
struct winsize ws;
int fd = 1;


if (!isatty(fd) && isatty(read_cmd_fd))
fd = read_cmd_fd;
if (ioctl(fd, TIOCGWINSZ, &ws) == 0)
{
columns = ws.ws_col;
rows = ws.ws_row;
}
}
#else 
#if defined(TIOCGSIZE)
{
struct ttysize ts;
int fd = 1;


if (!isatty(fd) && isatty(read_cmd_fd))
fd = read_cmd_fd;
if (ioctl(fd, TIOCGSIZE, &ts) == 0)
{
columns = ts.ts_cols;
rows = ts.ts_lines;
}
}
#endif 
#endif 






if (columns == 0 || rows == 0 || vim_strchr(p_cpo, CPO_TSIZE) != NULL)
{
if ((p = (char_u *)getenv("LINES")))
rows = atoi((char *)p);
if ((p = (char_u *)getenv("COLUMNS")))
columns = atoi((char *)p);
}

#if defined(HAVE_TGETENT)



if (columns == 0 || rows == 0)
getlinecol(&columns, &rows);
#endif




if (columns <= 0 || rows <= 0)
return FAIL;

Rows = rows;
Columns = columns;
limit_screen_size();
return OK;
}

#if defined(FEAT_TERMINAL) || defined(PROTO)



int
mch_report_winsize(int fd, int rows, int cols)
{
int tty_fd;
int retval = -1;

tty_fd = get_tty_fd(fd);
if (tty_fd >= 0)
{
#if defined(TIOCSWINSZ)
struct winsize ws;

ws.ws_col = cols;
ws.ws_row = rows;
ws.ws_xpixel = cols * 5;
ws.ws_ypixel = rows * 10;
retval = ioctl(tty_fd, TIOCSWINSZ, &ws);
ch_log(NULL, "ioctl(TIOCSWINSZ) %s",
retval == 0 ? "success" : "failed");
#elif defined(TIOCSSIZE)
struct ttysize ts;

ts.ts_cols = cols;
ts.ts_lines = rows;
retval = ioctl(tty_fd, TIOCSSIZE, &ts);
ch_log(NULL, "ioctl(TIOCSSIZE) %s",
retval == 0 ? "success" : "failed");
#endif
if (tty_fd != fd)
close(tty_fd);
}
return retval == 0 ? OK : FAIL;
}
#endif




void
mch_set_shellsize(void)
{
if (*T_CWS)
{





term_set_winsize((int)Rows, (int)Columns);
out_flush();
screen_start(); 
}
}

#endif 




void
mch_new_shellsize(void)
{

}





static pid_t
wait4pid(pid_t child, waitstatus *status)
{
pid_t wait_pid = 0;
long delay_msec = 1;

while (wait_pid != child)
{




#if defined(__NeXT__)
wait_pid = wait4(child, status, WNOHANG, (struct rusage *)0);
#else
wait_pid = waitpid(child, status, WNOHANG);
#endif
if (wait_pid == 0)
{

mch_delay(delay_msec, TRUE);
if (++delay_msec > 10)
delay_msec = 10;
continue;
}
if (wait_pid <= 0
#if defined(ECHILD)
&& errno == ECHILD
#endif
)
break;
}
return wait_pid;
}

#if !defined(USE_SYSTEM) || defined(FEAT_JOB_CHANNEL)



static void
set_child_environment(
long rows,
long columns,
char *term,
int is_terminal UNUSED)
{
#if defined(HAVE_SETENV)
char envbuf[50];
#else
static char envbuf_Term[30];
static char envbuf_Rows[20];
static char envbuf_Lines[20];
static char envbuf_Columns[20];
static char envbuf_Colors[20];
#if defined(FEAT_TERMINAL)
static char envbuf_Version[20];
#endif
#if defined(FEAT_CLIENTSERVER)
static char envbuf_Servername[60];
#endif
#endif
long colors =
#if defined(FEAT_GUI)
gui.in_use ? 256*256*256 :
#endif
t_colors;

#if defined(HAVE_SETENV)
setenv("TERM", term, 1);
sprintf((char *)envbuf, "%ld", rows);
setenv("ROWS", (char *)envbuf, 1);
sprintf((char *)envbuf, "%ld", rows);
setenv("LINES", (char *)envbuf, 1);
sprintf((char *)envbuf, "%ld", columns);
setenv("COLUMNS", (char *)envbuf, 1);
sprintf((char *)envbuf, "%ld", colors);
setenv("COLORS", (char *)envbuf, 1);
#if defined(FEAT_TERMINAL)
if (is_terminal)
{
sprintf((char *)envbuf, "%ld", (long)get_vim_var_nr(VV_VERSION));
setenv("VIM_TERMINAL", (char *)envbuf, 1);
}
#endif
#if defined(FEAT_CLIENTSERVER)
setenv("VIM_SERVERNAME", serverName == NULL ? "" : (char *)serverName, 1);
#endif
#else





vim_snprintf(envbuf_Term, sizeof(envbuf_Term), "TERM=%s", term);
putenv(envbuf_Term);
vim_snprintf(envbuf_Rows, sizeof(envbuf_Rows), "ROWS=%ld", rows);
putenv(envbuf_Rows);
vim_snprintf(envbuf_Lines, sizeof(envbuf_Lines), "LINES=%ld", rows);
putenv(envbuf_Lines);
vim_snprintf(envbuf_Columns, sizeof(envbuf_Columns),
"COLUMNS=%ld", columns);
putenv(envbuf_Columns);
vim_snprintf(envbuf_Colors, sizeof(envbuf_Colors), "COLORS=%ld", colors);
putenv(envbuf_Colors);
#if defined(FEAT_TERMINAL)
if (is_terminal)
{
vim_snprintf(envbuf_Version, sizeof(envbuf_Version),
"VIM_TERMINAL=%ld", (long)get_vim_var_nr(VV_VERSION));
putenv(envbuf_Version);
}
#endif
#if defined(FEAT_CLIENTSERVER)
vim_snprintf(envbuf_Servername, sizeof(envbuf_Servername),
"VIM_SERVERNAME=%s", serverName == NULL ? "" : (char *)serverName);
putenv(envbuf_Servername);
#endif
#endif
}

static void
set_default_child_environment(int is_terminal)
{
set_child_environment(Rows, Columns, "dumb", is_terminal);
}
#endif

#if defined(FEAT_GUI) || defined(FEAT_JOB_CHANNEL)






static void
open_pty(int *pty_master_fd, int *pty_slave_fd, char_u **name1, char_u **name2)
{
char *tty_name;

if (name1 != NULL)
*name1 = NULL;
if (name2 != NULL)
*name2 = NULL;

*pty_master_fd = mch_openpty(&tty_name); 
if (*pty_master_fd >= 0)
{



#if defined(O_NOCTTY)
*pty_slave_fd = open(tty_name, O_RDWR | O_NOCTTY | O_EXTRA, 0);
#else
*pty_slave_fd = open(tty_name, O_RDWR | O_EXTRA, 0);
#endif
if (*pty_slave_fd < 0)
{
close(*pty_master_fd);
*pty_master_fd = -1;
}
else
{
if (name1 != NULL)
*name1 = vim_strsave((char_u *)tty_name);
if (name2 != NULL)
*name2 = vim_strsave((char_u *)tty_name);
}
}
}
#endif




static void
may_send_sigint(int c UNUSED, pid_t pid UNUSED, pid_t wpid UNUSED)
{
#if defined(SIGINT)
if (c == Ctrl_C || c == intr_char)
{
#if defined(HAVE_SETSID)
kill(-pid, SIGINT);
#else
kill(0, SIGINT);
#endif
if (wpid > 0)
kill(wpid, SIGINT);
}
#endif
}

#if !defined(USE_SYSTEM) || defined(FEAT_TERMINAL) || defined(PROTO)






int
unix_build_argv(
char_u *cmd,
char ***argvp,
char_u **sh_tofree,
char_u **shcf_tofree)
{
char **argv = NULL;
int argc;

*sh_tofree = vim_strsave(p_sh);
if (*sh_tofree == NULL) 
return FAIL;

if (mch_parse_cmd(*sh_tofree, TRUE, &argv, &argc) == FAIL)
return FAIL;
*argvp = argv;

if (cmd != NULL)
{
char_u *s;
char_u *p;

if (extra_shell_arg != NULL)
argv[argc++] = (char *)extra_shell_arg;



*shcf_tofree = alloc(STRLEN(p_shcf) + 1);
if (*shcf_tofree == NULL) 
return FAIL;
s = *shcf_tofree;
p = p_shcf;
while (*p != NUL)
{
argv[argc++] = (char *)s;
while (*p && *p != ' ' && *p != TAB)
*s++ = *p++;
*s++ = NUL;
p = skipwhite(p);
}

argv[argc++] = (char *)cmd;
}
argv[argc] = NULL;
return OK;
}
#endif

#if defined(FEAT_GUI) && defined(FEAT_TERMINAL)



static int
mch_call_shell_terminal(
char_u *cmd,
int options UNUSED) 
{
jobopt_T opt;
char **argv = NULL;
char_u *tofree1 = NULL;
char_u *tofree2 = NULL;
int retval = -1;
buf_T *buf;
job_T *job;
aco_save_T aco;
oparg_T oa; 

if (unix_build_argv(cmd, &argv, &tofree1, &tofree2) == FAIL)
goto theend;

init_job_options(&opt);
ch_log(NULL, "starting terminal for system command '%s'", cmd);
buf = term_start(NULL, argv, &opt, TERM_START_SYSTEM);
if (buf == NULL)
goto theend;

job = term_getjob(buf->b_term);
++job->jv_refcount;


aucmd_prepbuf(&aco, buf);

clear_oparg(&oa);
while (term_use_loop())
{
if (oa.op_type == OP_NOP && oa.regname == NUL && !VIsual_active)
{


if (terminal_loop(TRUE) == OK)
normal_cmd(&oa, TRUE);
}
else
normal_cmd(&oa, TRUE);
}
retval = job->jv_exitval;
ch_log(NULL, "system command finished");

job_unref(job);


aucmd_restbuf(&aco);

wait_return(TRUE);
do_buffer(DOBUF_WIPE, DOBUF_FIRST, FORWARD, buf->b_fnum, TRUE);

theend:
vim_free(argv);
vim_free(tofree1);
vim_free(tofree2);
return retval;
}
#endif

#if defined(USE_SYSTEM)



static int
mch_call_shell_system(
char_u *cmd,
int options) 
{
#if defined(VMS)
char *ifn = NULL;
char *ofn = NULL;
#endif
int tmode = cur_tmode;
char_u *newcmd; 
int x;

out_flush();

if (options & SHELL_COOKED)
settmode(TMODE_COOK); 

#if defined(FEAT_CLIPBOARD) && defined(FEAT_X11)
save_clipboard();
loose_clipboard();
#endif

if (cmd == NULL)
x = system((char *)p_sh);
else
{
#if defined(VMS)
if (ofn = strchr((char *)cmd, '>'))
*ofn++ = '\0';
if (ifn = strchr((char *)cmd, '<'))
{
char *p;

*ifn++ = '\0';
p = strchr(ifn,' '); 
if (p)
*p = '\0';
}
if (ofn)
x = vms_sys((char *)cmd, ofn, ifn);
else
x = system((char *)cmd);
#else
newcmd = alloc(STRLEN(p_sh)
+ (extra_shell_arg == NULL ? 0 : STRLEN(extra_shell_arg))
+ STRLEN(p_shcf) + STRLEN(cmd) + 4);
if (newcmd == NULL)
x = 0;
else
{
sprintf((char *)newcmd, "%s %s %s %s", p_sh,
extra_shell_arg == NULL ? "" : (char *)extra_shell_arg,
(char *)p_shcf,
(char *)cmd);
x = system((char *)newcmd);
vim_free(newcmd);
}
#endif
}
#if defined(VMS)
x = vms_sys_status(x);
#endif
if (emsg_silent)
;
else if (x == 127)
msg_puts(_("\nCannot execute shell sh\n"));
else if (x && !(options & SHELL_SILENT))
{
msg_puts(_("\nshell returned "));
msg_outnum((long)x);
msg_putchar('\n');
}

if (tmode == TMODE_RAW)
settmode(TMODE_RAW); 
#if defined(FEAT_TITLE)
resettitle();
#endif
#if defined(FEAT_CLIPBOARD) && defined(FEAT_X11)
restore_clipboard();
#endif
return x;
}

#else 

#define EXEC_FAILED 122 

#define OPEN_NULL_FAILED 123 




static int
mch_call_shell_fork(
char_u *cmd,
int options) 
{
int tmode = cur_tmode;
pid_t pid;
pid_t wpid = 0;
pid_t wait_pid = 0;
#if defined(HAVE_UNION_WAIT)
union wait status;
#else
int status = -1;
#endif
int retval = -1;
char **argv = NULL;
char_u *tofree1 = NULL;
char_u *tofree2 = NULL;
int i;
int pty_master_fd = -1; 
#if defined(FEAT_GUI)
int pty_slave_fd = -1;
#endif
int fd_toshell[2]; 
int fd_fromshell[2];
int pipe_error = FALSE;
int did_settmode = FALSE; 

out_flush();
if (options & SHELL_COOKED)
settmode(TMODE_COOK); 

if (unix_build_argv(cmd, &argv, &tofree1, &tofree2) == FAIL)
goto error;






if ((options & (SHELL_READ|SHELL_WRITE))
#if defined(FEAT_GUI)
|| (gui.in_use && show_shell_mess)
#endif
)
{
#if defined(FEAT_GUI)





if (p_guipty && !(options & (SHELL_READ|SHELL_WRITE)))
open_pty(&pty_master_fd, &pty_slave_fd, NULL, NULL);



if (pty_master_fd < 0)
#endif
{
pipe_error = (pipe(fd_toshell) < 0);
if (!pipe_error) 
{
pipe_error = (pipe(fd_fromshell) < 0);
if (pipe_error) 
{
close(fd_toshell[0]);
close(fd_toshell[1]);
}
}
if (pipe_error)
{
msg_puts(_("\nCannot create pipes\n"));
out_flush();
}
}
}

if (!pipe_error) 
{
SIGSET_DECL(curset)

#if defined(__BEOS__) && USE_THREAD_FOR_INPUT_WITH_TIMEOUT
beos_cleanup_read_thread();
#endif

BLOCK_SIGNALS(&curset);
pid = fork(); 
if (pid == -1)
{
UNBLOCK_SIGNALS(&curset);

msg_puts(_("\nCannot fork\n"));
if ((options & (SHELL_READ|SHELL_WRITE))
#if defined(FEAT_GUI)
|| (gui.in_use && show_shell_mess)
#endif
)
{
#if defined(FEAT_GUI)
if (pty_master_fd >= 0) 
{
close(pty_master_fd);
close(pty_slave_fd);
}
else 
#endif
{
close(fd_toshell[0]);
close(fd_toshell[1]);
close(fd_fromshell[0]);
close(fd_fromshell[1]);
}
}
}
else if (pid == 0) 
{
reset_signals(); 
UNBLOCK_SIGNALS(&curset);

#if defined(FEAT_JOB_CHANNEL)
if (ch_log_active())

ch_logfile((char_u *)"", (char_u *)"");
#endif

if (!show_shell_mess || (options & SHELL_EXPAND))
{
int fd;









fd = open("/dev/null", O_RDWR | O_EXTRA, 0);
fclose(stdin);
fclose(stdout);
fclose(stderr);










if (fd >= 0)
{
vim_ignored = dup(fd); 
vim_ignored = dup(fd); 
vim_ignored = dup(fd); 


close(fd);
}
}
else if ((options & (SHELL_READ|SHELL_WRITE))
#if defined(FEAT_GUI)
|| gui.in_use
#endif
)
{

#if defined(HAVE_SETSID)



if (p_stmp)
{
(void)setsid();
#if defined(SIGHUP)




signal(SIGHUP, SIG_IGN);
#endif
}
#endif
#if defined(FEAT_GUI)
if (pty_slave_fd >= 0)
{

if (options & SHELL_COOKED)
setup_slavepty(pty_slave_fd);
#if defined(TIOCSCTTY)


ioctl(pty_slave_fd, TIOCSCTTY, (char *)NULL);
#endif
}
#endif
set_default_child_environment(FALSE);






#if defined(FEAT_GUI)
if (pty_master_fd >= 0)
{
close(pty_master_fd); 


close(0);
vim_ignored = dup(pty_slave_fd);
close(1);
vim_ignored = dup(pty_slave_fd);
if (gui.in_use)
{
close(2);
vim_ignored = dup(pty_slave_fd);
}

close(pty_slave_fd); 
}
else
#endif
{

close(fd_toshell[1]);
close(0);
vim_ignored = dup(fd_toshell[0]);
close(fd_toshell[0]);


close(fd_fromshell[0]);
close(1);
vim_ignored = dup(fd_fromshell[1]);
close(fd_fromshell[1]);

#if defined(FEAT_GUI)
if (gui.in_use)
{

close(2);
vim_ignored = dup(1);
}
#endif
}
}








execvp(argv[0], argv);
_exit(EXEC_FAILED); 
}
else 
{




catch_signals(SIG_IGN, SIG_ERR);
catch_int_signal();
UNBLOCK_SIGNALS(&curset);
#if defined(FEAT_JOB_CHANNEL)
++dont_check_job_ended;
#endif





if ((options & (SHELL_READ|SHELL_WRITE))
#if defined(FEAT_GUI)
|| (gui.in_use && show_shell_mess)
#endif
)
{
#define BUFLEN 100 
char_u buffer[BUFLEN + 1];
int buffer_off = 0; 
char_u ta_buf[BUFLEN + 1]; 
int ta_len = 0; 
int len;
int p_more_save;
int old_State;
int c;
int toshell_fd;
int fromshell_fd;
garray_T ga;
int noread_cnt;
#if defined(ELAPSED_FUNC)
elapsed_T start_tv;
#endif

#if defined(FEAT_GUI)
if (pty_master_fd >= 0)
{
fromshell_fd = pty_master_fd;
toshell_fd = dup(pty_master_fd);
}
else
#endif
{
close(fd_toshell[0]);
close(fd_fromshell[1]);
toshell_fd = fd_toshell[1];
fromshell_fd = fd_fromshell[0];
}

















p_more_save = p_more;
p_more = FALSE;
old_State = State;
State = EXTERNCMD; 

if ((options & SHELL_WRITE) && toshell_fd >= 0)
{


if ((wpid = fork()) == -1)
{
msg_puts(_("\nCannot fork\n"));
}
else if (wpid == 0) 
{
linenr_T lnum = curbuf->b_op_start.lnum;
int written = 0;
char_u *lp = ml_get(lnum);
size_t l;

close(fromshell_fd);
for (;;)
{
l = STRLEN(lp + written);
if (l == 0)
len = 0;
else if (lp[written] == NL)

len = write(toshell_fd, "", (size_t)1);
else
{
char_u *s = vim_strchr(lp + written, NL);

len = write(toshell_fd, (char *)lp + written,
s == NULL ? l
: (size_t)(s - (lp + written)));
}
if (len == (int)l)
{


if (lnum != curbuf->b_op_end.lnum
|| (!curbuf->b_p_bin
&& curbuf->b_p_fixeol)
|| (lnum != curbuf->b_no_eol_lnum
&& (lnum !=
curbuf->b_ml.ml_line_count
|| curbuf->b_p_eol)))
vim_ignored = write(toshell_fd, "\n",
(size_t)1);
++lnum;
if (lnum > curbuf->b_op_end.lnum)
{

close(toshell_fd);
toshell_fd = -1;
break;
}
lp = ml_get(lnum);
written = 0;
}
else if (len > 0)
written += len;
}
_exit(0);
}
else 
{
close(toshell_fd);
toshell_fd = -1;
}
}

if (options & SHELL_READ)
ga_init2(&ga, 1, BUFLEN);

noread_cnt = 0;
#if defined(ELAPSED_FUNC)
ELAPSED_INIT(start_tv);
#endif
for (;;)
{














len = 0;
if (!(options & SHELL_EXPAND)
&& ((options &
(SHELL_READ|SHELL_WRITE|SHELL_COOKED))
!= (SHELL_READ|SHELL_WRITE|SHELL_COOKED)
#if defined(FEAT_GUI)
|| gui.in_use
#endif
)
&& wait_pid == 0
&& (ta_len > 0 || noread_cnt > 4))
{
if (ta_len == 0)
{


noread_cnt = 0;
#if defined(ELAPSED_FUNC)
ELAPSED_INIT(start_tv);
#endif
len = ui_inchar(ta_buf, BUFLEN, 10L, 0);
}
if (ta_len > 0 || len > 0)
{





if (len == 1 && (pty_master_fd < 0 || cmd != NULL))
{




may_send_sigint(ta_buf[ta_len], pid, wpid);

if (pty_master_fd < 0 && toshell_fd >= 0
&& ta_buf[ta_len] == Ctrl_D)
{
close(toshell_fd);
toshell_fd = -1;
}
}

term_replace_bs_del_keycode(ta_buf, ta_len, len);





if (pty_master_fd < 0)
{
for (i = ta_len; i < ta_len + len; ++i)
{
if (ta_buf[i] == '\n' || ta_buf[i] == '\b')
msg_putchar(ta_buf[i]);
else if (has_mbyte)
{
int l = (*mb_ptr2len)(ta_buf + i);

msg_outtrans_len(ta_buf + i, l);
i += l - 1;
}
else
msg_outtrans_len(ta_buf + i, 1);
}
windgoto(msg_row, msg_col);
out_flush();
}

ta_len += len;








if (options & SHELL_WRITE)
ta_len = 0;
else if (toshell_fd >= 0)
{
len = write(toshell_fd, (char *)ta_buf, (size_t)1);
if (len > 0)
{
ta_len -= len;
mch_memmove(ta_buf, ta_buf + len, ta_len);
}
}
}
}

if (got_int)
{


#if defined(HAVE_SETSID)
kill(-pid, SIGINT);
#else
kill(0, SIGINT);
#endif
if (wpid > 0)
kill(wpid, SIGINT);
got_int = FALSE;
}










++noread_cnt;
while (RealWaitForChar(fromshell_fd, 10L, NULL, NULL))
{
len = read_eintr(fromshell_fd, buffer
+ buffer_off, (size_t)(BUFLEN - buffer_off)
);
if (len <= 0) 
goto finished;

noread_cnt = 0;
if (options & SHELL_READ)
{


for (i = 0; i < len; ++i)
{
if (buffer[i] == NL)
append_ga_line(&ga);
else if (buffer[i] == NUL)
ga_append(&ga, NL);
else
ga_append(&ga, buffer[i]);
}
}
else if (has_mbyte)
{
int l;
char_u *p;

len += buffer_off;
buffer[len] = NUL;




for (p = buffer; p < buffer + len; p += l)
{
l = MB_CPTR2LEN(p);
if (l == 0)
l = 1; 
else if (MB_BYTE2LEN(*p) != l)
break;
}
if (p == buffer) 
{

if (len >= 12)
++p;
else
{
buffer_off = len;
continue;
}
}
c = *p;
*p = NUL;
msg_puts((char *)buffer);
if (p < buffer + len)
{
*p = c;
buffer_off = (buffer + len) - p;
mch_memmove(buffer, p, buffer_off);
continue;
}
buffer_off = 0;
}
else
{
buffer[len] = NUL;
msg_puts((char *)buffer);
}

windgoto(msg_row, msg_col);
cursor_on();
out_flush();
if (got_int)
break;

#if defined(ELAPSED_FUNC)
if (wait_pid == 0)
{
long msec = ELAPSED_FUNC(start_tv);




if (msec > 2000)
{
noread_cnt = 5;
break;
}
}
#endif
}




if (wait_pid == pid)
{
if (noread_cnt < 5)
continue;
break;
}





#if defined(__NeXT__)
wait_pid = wait4(pid, &status, WNOHANG, (struct rusage *)0);
#else
wait_pid = waitpid(pid, &status, WNOHANG);
#endif
if ((wait_pid == (pid_t)-1 && errno == ECHILD)
|| (wait_pid == pid && WIFEXITED(status)))
{




wait_pid = pid;
}
else
wait_pid = 0;

#if defined(FEAT_XCLIPBOARD) && defined(FEAT_X11)

clip_update();
#endif
}
finished:
p_more = p_more_save;
if (options & SHELL_READ)
{
if (ga.ga_len > 0)
{
append_ga_line(&ga);

curbuf->b_no_eol_lnum = curwin->w_cursor.lnum;
}
else
curbuf->b_no_eol_lnum = 0;
ga_clear(&ga);
}




if (ta_len)
ui_inchar_undo(ta_buf, ta_len);
State = old_State;
if (toshell_fd >= 0)
close(toshell_fd);
close(fromshell_fd);
}
#if defined(FEAT_XCLIPBOARD) && defined(FEAT_X11)
else
{
long delay_msec = 1;





for (;;)
{
if (got_int)
{


#if defined(HAVE_SETSID)
kill(-pid, SIGINT);
#else
kill(0, SIGINT);
#endif
got_int = FALSE;
}
#if defined(__NeXT__)
wait_pid = wait4(pid, &status, WNOHANG, (struct rusage *)0);
#else
wait_pid = waitpid(pid, &status, WNOHANG);
#endif
if ((wait_pid == (pid_t)-1 && errno == ECHILD)
|| (wait_pid == pid && WIFEXITED(status)))
{
wait_pid = pid;
break;
}


clip_update();



mch_delay(delay_msec, TRUE);
if (++delay_msec > 10)
delay_msec = 10;
}
}
#endif








if (wait_pid != pid)
wait_pid = wait4pid(pid, &status);

#if defined(FEAT_GUI)



if (pty_master_fd >= 0)
close(pty_slave_fd);
#endif



if (wpid > 0)
{
kill(wpid, SIGKILL);
wait4pid(wpid, NULL);
}

#if defined(FEAT_JOB_CHANNEL)
--dont_check_job_ended;
#endif





if (tmode == TMODE_RAW)
settmode(TMODE_RAW);
did_settmode = TRUE;
set_signals();

if (WIFEXITED(status))
{

retval = WEXITSTATUS(status);
if (retval != 0 && !emsg_silent)
{
if (retval == EXEC_FAILED)
{
msg_puts(_("\nCannot execute shell "));
msg_outtrans(p_sh);
msg_putchar('\n');
}
else if (!(options & SHELL_SILENT))
{
msg_puts(_("\nshell returned "));
msg_outnum((long)retval);
msg_putchar('\n');
}
}
}
else
msg_puts(_("\nCommand terminated\n"));
}
}

error:
if (!did_settmode)
if (tmode == TMODE_RAW)
settmode(TMODE_RAW); 
#if defined(FEAT_TITLE)
resettitle();
#endif
vim_free(argv);
vim_free(tofree1);
vim_free(tofree2);

return retval;
}
#endif 

int
mch_call_shell(
char_u *cmd,
int options) 
{
#if defined(FEAT_GUI) && defined(FEAT_TERMINAL)
if (gui.in_use && vim_strchr(p_go, GO_TERMINAL) != NULL)
return mch_call_shell_terminal(cmd, options);
#endif
#if defined(USE_SYSTEM)
return mch_call_shell_system(cmd, options);
#else
return mch_call_shell_fork(cmd, options);
#endif
}

#if defined(FEAT_JOB_CHANNEL) || defined(PROTO)
void
mch_job_start(char **argv, job_T *job, jobopt_T *options, int is_terminal)
{
pid_t pid;
int fd_in[2] = {-1, -1}; 
int fd_out[2] = {-1, -1}; 
int fd_err[2] = {-1, -1}; 
int pty_master_fd = -1;
int pty_slave_fd = -1;
channel_T *channel = NULL;
int use_null_for_in = options->jo_io[PART_IN] == JIO_NULL;
int use_null_for_out = options->jo_io[PART_OUT] == JIO_NULL;
int use_null_for_err = options->jo_io[PART_ERR] == JIO_NULL;
int use_file_for_in = options->jo_io[PART_IN] == JIO_FILE;
int use_file_for_out = options->jo_io[PART_OUT] == JIO_FILE;
int use_file_for_err = options->jo_io[PART_ERR] == JIO_FILE;
int use_buffer_for_in = options->jo_io[PART_IN] == JIO_BUFFER;
int use_out_for_err = options->jo_io[PART_ERR] == JIO_OUT;
SIGSET_DECL(curset)

if (use_out_for_err && use_null_for_out)
use_null_for_err = TRUE;


job->jv_status = JOB_FAILED;

if (options->jo_pty
&& (!(use_file_for_in || use_null_for_in)
|| !(use_file_for_out || use_null_for_out)
|| !(use_out_for_err || use_file_for_err || use_null_for_err)))
open_pty(&pty_master_fd, &pty_slave_fd,
&job->jv_tty_out, &job->jv_tty_in);



if (use_file_for_in)
{
char_u *fname = options->jo_io_name[PART_IN];

fd_in[0] = mch_open((char *)fname, O_RDONLY, 0);
if (fd_in[0] < 0)
{
semsg(_(e_notopen), fname);
goto failed;
}
}
else


if (!use_null_for_in && (pty_master_fd < 0 || use_buffer_for_in)
&& pipe(fd_in) < 0)
goto failed;

if (use_file_for_out)
{
char_u *fname = options->jo_io_name[PART_OUT];

fd_out[1] = mch_open((char *)fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fd_out[1] < 0)
{
semsg(_(e_notopen), fname);
goto failed;
}
}
else if (!use_null_for_out && pty_master_fd < 0 && pipe(fd_out) < 0)
goto failed;

if (use_file_for_err)
{
char_u *fname = options->jo_io_name[PART_ERR];

fd_err[1] = mch_open((char *)fname, O_WRONLY | O_CREAT | O_TRUNC, 0600);
if (fd_err[1] < 0)
{
semsg(_(e_notopen), fname);
goto failed;
}
}
else if (!use_out_for_err && !use_null_for_err
&& pty_master_fd < 0 && pipe(fd_err) < 0)
goto failed;

if (!use_null_for_in || !use_null_for_out || !use_null_for_err)
{
if (options->jo_set & JO_CHANNEL)
{
channel = options->jo_channel;
if (channel != NULL)
++channel->ch_refcount;
}
else
channel = add_channel();
if (channel == NULL)
goto failed;
if (job->jv_tty_out != NULL)
ch_log(channel, "using pty %s on fd %d",
job->jv_tty_out, pty_master_fd);
}

BLOCK_SIGNALS(&curset);
pid = fork(); 
if (pid == -1)
{

UNBLOCK_SIGNALS(&curset);
goto failed;
}
if (pid == 0)
{
int null_fd = -1;
int stderr_works = TRUE;


reset_signals(); 
UNBLOCK_SIGNALS(&curset);

#if defined(FEAT_JOB_CHANNEL)
if (ch_log_active())

ch_logfile((char_u *)"", (char_u *)"");
#endif

#if defined(HAVE_SETSID)



(void)setsid();
#endif

#if defined(FEAT_TERMINAL)
if (options->jo_term_rows > 0)
{
char *term = (char *)T_NAME;

#if defined(FEAT_GUI)
if (term_is_gui(T_NAME))

term = getenv("TERM");
#endif


if (term == NULL || *term == NUL || STRNCMP(term, "xterm", 5) != 0)
term = "xterm";
set_child_environment(
(long)options->jo_term_rows,
(long)options->jo_term_cols,
term,
is_terminal);
}
else
#endif
set_default_child_environment(is_terminal);

if (options->jo_env != NULL)
{
dict_T *dict = options->jo_env;
hashitem_T *hi;
int todo = (int)dict->dv_hashtab.ht_used;

for (hi = dict->dv_hashtab.ht_array; todo > 0; ++hi)
if (!HASHITEM_EMPTY(hi))
{
typval_T *item = &dict_lookup(hi)->di_tv;

vim_setenv((char_u*)hi->hi_key, tv_get_string(item));
--todo;
}
}

if (use_null_for_in || use_null_for_out || use_null_for_err)
{
null_fd = open("/dev/null", O_RDWR | O_EXTRA, 0);
if (null_fd < 0)
{
perror("opening /dev/null failed");
_exit(OPEN_NULL_FAILED);
}
}

if (pty_slave_fd >= 0)
{

setup_slavepty(pty_slave_fd);
#if defined(TIOCSCTTY)


ioctl(pty_slave_fd, TIOCSCTTY, (char *)NULL);
#endif
}


close(0);
if (use_null_for_in && null_fd >= 0)
vim_ignored = dup(null_fd);
else if (fd_in[0] < 0)
vim_ignored = dup(pty_slave_fd);
else
vim_ignored = dup(fd_in[0]);


close(2);
if (use_null_for_err && null_fd >= 0)
{
vim_ignored = dup(null_fd);
stderr_works = FALSE;
}
else if (use_out_for_err)
vim_ignored = dup(fd_out[1]);
else if (fd_err[1] < 0)
vim_ignored = dup(pty_slave_fd);
else
vim_ignored = dup(fd_err[1]);


close(1);
if (use_null_for_out && null_fd >= 0)
vim_ignored = dup(null_fd);
else if (fd_out[1] < 0)
vim_ignored = dup(pty_slave_fd);
else
vim_ignored = dup(fd_out[1]);

if (fd_in[0] >= 0)
close(fd_in[0]);
if (fd_in[1] >= 0)
close(fd_in[1]);
if (fd_out[0] >= 0)
close(fd_out[0]);
if (fd_out[1] >= 0)
close(fd_out[1]);
if (fd_err[0] >= 0)
close(fd_err[0]);
if (fd_err[1] >= 0)
close(fd_err[1]);
if (pty_master_fd >= 0)
{
close(pty_master_fd); 
close(pty_slave_fd); 
}

if (null_fd >= 0)
close(null_fd);

if (options->jo_cwd != NULL && mch_chdir((char *)options->jo_cwd) != 0)
_exit(EXEC_FAILED);


execvp(argv[0], argv);

if (stderr_works)
perror("executing job failed");
#if defined(EXITFREE)


#endif
_exit(EXEC_FAILED); 
}


UNBLOCK_SIGNALS(&curset);

job->jv_pid = pid;
job->jv_status = JOB_STARTED;
job->jv_channel = channel; 

if (pty_master_fd >= 0)
close(pty_slave_fd); 

if (fd_in[0] >= 0)
close(fd_in[0]);
if (fd_out[1] >= 0)
close(fd_out[1]);
if (fd_err[1] >= 0)
close(fd_err[1]);
if (channel != NULL)
{
int in_fd = INVALID_FD;
int out_fd = INVALID_FD;
int err_fd = INVALID_FD;

if (!(use_file_for_in || use_null_for_in))
in_fd = fd_in[1] >= 0 ? fd_in[1] : pty_master_fd;

if (!(use_file_for_out || use_null_for_out))
out_fd = fd_out[0] >= 0 ? fd_out[0] : pty_master_fd;



if (!(use_out_for_err || use_file_for_err || use_null_for_err))
{
if (fd_err[0] >= 0)
err_fd = fd_err[0];
else if (out_fd != pty_master_fd)
err_fd = pty_master_fd;
}

channel_set_pipes(channel, in_fd, out_fd, err_fd);
channel_set_job(channel, job, options);
}
else
{
if (fd_in[1] >= 0)
close(fd_in[1]);
if (fd_out[0] >= 0)
close(fd_out[0]);
if (fd_err[0] >= 0)
close(fd_err[0]);
if (pty_master_fd >= 0)
close(pty_master_fd);
}


return;

failed:
channel_unref(channel);
if (fd_in[0] >= 0)
close(fd_in[0]);
if (fd_in[1] >= 0)
close(fd_in[1]);
if (fd_out[0] >= 0)
close(fd_out[0]);
if (fd_out[1] >= 0)
close(fd_out[1]);
if (fd_err[0] >= 0)
close(fd_err[0]);
if (fd_err[1] >= 0)
close(fd_err[1]);
if (pty_master_fd >= 0)
close(pty_master_fd);
if (pty_slave_fd >= 0)
close(pty_slave_fd);
}

static char_u *
get_signal_name(int sig)
{
int i;
char_u numbuf[NUMBUFLEN];

if (sig == SIGKILL)
return vim_strsave((char_u *)"kill");

for (i = 0; signal_info[i].sig != -1; i++)
if (sig == signal_info[i].sig)
return strlow_save((char_u *)signal_info[i].name);

vim_snprintf((char *)numbuf, NUMBUFLEN, "%d", sig);
return vim_strsave(numbuf);
}

char *
mch_job_status(job_T *job)
{
#if defined(HAVE_UNION_WAIT)
union wait status;
#else
int status = -1;
#endif
pid_t wait_pid = 0;

#if defined(__NeXT__)
wait_pid = wait4(job->jv_pid, &status, WNOHANG, (struct rusage *)0);
#else
wait_pid = waitpid(job->jv_pid, &status, WNOHANG);
#endif
if (wait_pid == -1)
{

if (job->jv_status < JOB_ENDED)
ch_log(job->jv_channel, "Job no longer exists: %s",
strerror(errno));
goto return_dead;
}
if (wait_pid == 0)
return "run";
if (WIFEXITED(status))
{

job->jv_exitval = WEXITSTATUS(status);
if (job->jv_status < JOB_ENDED)
ch_log(job->jv_channel, "Job exited with %d", job->jv_exitval);
goto return_dead;
}
if (WIFSIGNALED(status))
{
job->jv_exitval = -1;
job->jv_termsig = get_signal_name(WTERMSIG(status));
if (job->jv_status < JOB_ENDED && job->jv_termsig != NULL)
ch_log(job->jv_channel, "Job terminated by signal \"%s\"",
job->jv_termsig);
goto return_dead;
}
return "run";

return_dead:
if (job->jv_status < JOB_ENDED)
job->jv_status = JOB_ENDED;
return "dead";
}

job_T *
mch_detect_ended_job(job_T *job_list)
{
#if defined(HAVE_UNION_WAIT)
union wait status;
#else
int status = -1;
#endif
pid_t wait_pid = 0;
job_T *job;

#if !defined(USE_SYSTEM)



if (dont_check_job_ended > 0)
return NULL;
#endif

#if defined(__NeXT__)
wait_pid = wait4(-1, &status, WNOHANG, (struct rusage *)0);
#else
wait_pid = waitpid(-1, &status, WNOHANG);
#endif
if (wait_pid <= 0)

return NULL;
for (job = job_list; job != NULL; job = job->jv_next)
{
if (job->jv_pid == wait_pid)
{
if (WIFEXITED(status))

job->jv_exitval = WEXITSTATUS(status);
else if (WIFSIGNALED(status))
{
job->jv_exitval = -1;
job->jv_termsig = get_signal_name(WTERMSIG(status));
}
if (job->jv_status < JOB_ENDED)
{
ch_log(job->jv_channel, "Job ended");
job->jv_status = JOB_ENDED;
}
return job;
}
}
return NULL;
}





int
mch_signal_job(job_T *job, char_u *how)
{
int sig = -1;

if (*how == NUL || STRCMP(how, "term") == 0)
sig = SIGTERM;
else if (STRCMP(how, "hup") == 0)
sig = SIGHUP;
else if (STRCMP(how, "quit") == 0)
sig = SIGQUIT;
else if (STRCMP(how, "int") == 0)
sig = SIGINT;
else if (STRCMP(how, "kill") == 0)
sig = SIGKILL;
#if defined(SIGWINCH)
else if (STRCMP(how, "winch") == 0)
sig = SIGWINCH;
#endif
else if (isdigit(*how))
sig = atoi((char *)how);
else
return FAIL;


if (job->jv_pid != 0)
{

kill(-job->jv_pid, sig);
kill(job->jv_pid, sig);
}

return OK;
}




void
mch_clear_job(job_T *job)
{

#if defined(__NeXT__)
(void)wait4(job->jv_pid, NULL, WNOHANG, (struct rusage *)0);
#else
(void)waitpid(job->jv_pid, NULL, WNOHANG);
#endif
}
#endif

#if defined(FEAT_TERMINAL) || defined(PROTO)
int
mch_create_pty_channel(job_T *job, jobopt_T *options)
{
int pty_master_fd = -1;
int pty_slave_fd = -1;
channel_T *channel;

open_pty(&pty_master_fd, &pty_slave_fd, &job->jv_tty_out, &job->jv_tty_in);
close(pty_slave_fd);

channel = add_channel();
if (channel == NULL)
{
close(pty_master_fd);
return FAIL;
}
if (job->jv_tty_out != NULL)
ch_log(channel, "using pty %s on fd %d",
job->jv_tty_out, pty_master_fd);
job->jv_channel = channel; 
channel->ch_keep_open = TRUE;



channel_set_pipes(channel, pty_master_fd, pty_master_fd, INVALID_FD);
channel_set_job(channel, job, options);
return OK;
}
#endif





void
mch_breakcheck(int force)
{
if ((curr_tmode == TMODE_RAW || force)
&& RealWaitForChar(read_cmd_fd, 0L, NULL, NULL))
fill_input_buf(FALSE);
}













static int
WaitForChar(long msec, int *interrupted, int ignore_input)
{
#if defined(FEAT_TIMERS)
return ui_wait_for_chars_or_timer(
msec, WaitForCharOrMouse, interrupted, ignore_input) == OK;
#else
return WaitForCharOrMouse(msec, interrupted, ignore_input);
#endif
}










static int
WaitForCharOrMouse(long msec, int *interrupted, int ignore_input)
{
#if defined(FEAT_MOUSE_GPM)
int gpm_process_wanted;
#endif
#if defined(FEAT_XCLIPBOARD)
int rest;
#endif
int avail;

if (!ignore_input && input_available()) 
return 1;

#if defined(FEAT_MOUSE_DEC)

if (WantQueryMouse)
{
WantQueryMouse = FALSE;
if (!no_query_mouse_for_testing)
mch_write((char_u *)IF_EB("\033[1'|", ESC_STR "[1'|"), 5);
}
#endif





#if defined(FEAT_MOUSE_GPM) || defined(FEAT_XCLIPBOARD)
#if defined(FEAT_XCLIPBOARD)
rest = 0;
if (do_xterm_trace())
rest = msec;
#endif
do
{
#if defined(FEAT_XCLIPBOARD)
if (rest != 0)
{
msec = XT_TRACE_DELAY;
if (rest >= 0 && rest < XT_TRACE_DELAY)
msec = rest;
if (rest >= 0)
rest -= msec;
}
#endif
#if defined(FEAT_SOUND_CANBERRA)

if (has_sound_callback_in_queue())
invoke_sound_callback();
#endif
#if defined(FEAT_MOUSE_GPM)
gpm_process_wanted = 0;
avail = RealWaitForChar(read_cmd_fd, msec,
&gpm_process_wanted, interrupted);
if (!avail && !gpm_process_wanted)
#else
avail = RealWaitForChar(read_cmd_fd, msec, NULL, interrupted);
if (!avail)
#endif
{
if (!ignore_input && input_available())
return 1;
#if defined(FEAT_XCLIPBOARD)
if (rest == 0 || !do_xterm_trace())
#endif
break;
}
}
while (FALSE
#if defined(FEAT_MOUSE_GPM)
|| (gpm_process_wanted && mch_gpm_process() == 0)
#endif
#if defined(FEAT_XCLIPBOARD)
|| (!avail && rest != 0)
#endif
)
;

#else
avail = RealWaitForChar(read_cmd_fd, msec, NULL, interrupted);
#endif
return avail;
}

#if !defined(VMS)










#if defined(__BEOS__)
int
#else
static int
#endif
RealWaitForChar(int fd, long msec, int *check_for_gpm UNUSED, int *interrupted)
{
int ret;
int result;
#if defined(FEAT_XCLIPBOARD) || defined(USE_XSMP) || defined(FEAT_MZSCHEME)
static int busy = FALSE;


#define MAY_LOOP

#if defined(ELAPSED_FUNC)


long start_msec = msec;
elapsed_T start_tv;

if (msec > 0)
ELAPSED_INIT(start_tv);
#endif



if (busy)
return 0;
#endif

#if defined(MAY_LOOP)
for (;;)
#endif
{
#if defined(MAY_LOOP)
int finished = TRUE; 
#if defined(FEAT_MZSCHEME)
int mzquantum_used = FALSE;
#endif
#endif
#if !defined(HAVE_SELECT)

struct pollfd fds[6 + 3 * MAX_OPEN_CHANNELS];
int nfd;
#if defined(FEAT_XCLIPBOARD)
int xterm_idx = -1;
#endif
#if defined(FEAT_MOUSE_GPM)
int gpm_idx = -1;
#endif
#if defined(USE_XSMP)
int xsmp_idx = -1;
#endif
int towait = (int)msec;

#if defined(FEAT_MZSCHEME)
mzvim_check_threads();
if (mzthreads_allowed() && p_mzq > 0 && (msec < 0 || msec > p_mzq))
{
towait = (int)p_mzq; 
mzquantum_used = TRUE;
}
#endif
fds[0].fd = fd;
fds[0].events = POLLIN;
nfd = 1;

#if defined(FEAT_XCLIPBOARD)
may_restore_clipboard();
if (xterm_Shell != (Widget)0)
{
xterm_idx = nfd;
fds[nfd].fd = ConnectionNumber(xterm_dpy);
fds[nfd].events = POLLIN;
nfd++;
}
#endif
#if defined(FEAT_MOUSE_GPM)
if (check_for_gpm != NULL && gpm_flag && gpm_fd >= 0)
{
gpm_idx = nfd;
fds[nfd].fd = gpm_fd;
fds[nfd].events = POLLIN;
nfd++;
}
#endif
#if defined(USE_XSMP)
if (xsmp_icefd != -1)
{
xsmp_idx = nfd;
fds[nfd].fd = xsmp_icefd;
fds[nfd].events = POLLIN;
nfd++;
}
#endif
#if defined(FEAT_JOB_CHANNEL)
nfd = channel_poll_setup(nfd, &fds, &towait);
#endif
if (interrupted != NULL)
*interrupted = FALSE;

ret = poll(fds, nfd, towait);

result = ret > 0 && (fds[0].revents & POLLIN);
if (result == 0 && interrupted != NULL && ret > 0)
*interrupted = TRUE;

#if defined(FEAT_MZSCHEME)
if (ret == 0 && mzquantum_used)

finished = FALSE;
#endif

#if defined(FEAT_XCLIPBOARD)
if (xterm_Shell != (Widget)0 && (fds[xterm_idx].revents & POLLIN))
{
xterm_update(); 
if (--ret == 0 && !input_available())

finished = FALSE;
}
#endif
#if defined(FEAT_MOUSE_GPM)
if (gpm_idx >= 0 && (fds[gpm_idx].revents & POLLIN))
*check_for_gpm = 1;
#endif
#if defined(USE_XSMP)
if (xsmp_idx >= 0 && (fds[xsmp_idx].revents & (POLLIN | POLLHUP)))
{
if (fds[xsmp_idx].revents & POLLIN)
{
busy = TRUE;
xsmp_handle_requests();
busy = FALSE;
}
else if (fds[xsmp_idx].revents & POLLHUP)
{
if (p_verbose > 0)
verb_msg(_("XSMP lost ICE connection"));
xsmp_close();
}
if (--ret == 0)
finished = FALSE; 
}
#endif
#if defined(FEAT_JOB_CHANNEL)

if (ret >= 0)
channel_poll_check(ret, &fds);
#endif

#else 

struct timeval tv;
struct timeval *tvp;


static fd_set rfds, wfds, efds;
int maxfd;
long towait = msec;

#if defined(FEAT_MZSCHEME)
mzvim_check_threads();
if (mzthreads_allowed() && p_mzq > 0 && (msec < 0 || msec > p_mzq))
{
towait = p_mzq; 
mzquantum_used = TRUE;
}
#endif

if (towait >= 0)
{
tv.tv_sec = towait / 1000;
tv.tv_usec = (towait % 1000) * (1000000/1000);
tvp = &tv;
}
else
tvp = NULL;




select_eintr:
FD_ZERO(&rfds);
FD_ZERO(&wfds);
FD_ZERO(&efds);
FD_SET(fd, &rfds);
#if !defined(__QNX__) && !defined(__CYGWIN32__)

FD_SET(fd, &efds);
#endif
maxfd = fd;

#if defined(FEAT_XCLIPBOARD)
may_restore_clipboard();
if (xterm_Shell != (Widget)0)
{
FD_SET(ConnectionNumber(xterm_dpy), &rfds);
if (maxfd < ConnectionNumber(xterm_dpy))
maxfd = ConnectionNumber(xterm_dpy);



xterm_update();
}
#endif
#if defined(FEAT_MOUSE_GPM)
if (check_for_gpm != NULL && gpm_flag && gpm_fd >= 0)
{
FD_SET(gpm_fd, &rfds);
FD_SET(gpm_fd, &efds);
if (maxfd < gpm_fd)
maxfd = gpm_fd;
}
#endif
#if defined(USE_XSMP)
if (xsmp_icefd != -1)
{
FD_SET(xsmp_icefd, &rfds);
FD_SET(xsmp_icefd, &efds);
if (maxfd < xsmp_icefd)
maxfd = xsmp_icefd;
}
#endif
#if defined(FEAT_JOB_CHANNEL)
maxfd = channel_select_setup(maxfd, &rfds, &wfds, &tv, &tvp);
#endif
if (interrupted != NULL)
*interrupted = FALSE;

ret = select(maxfd + 1, SELECT_TYPE_ARG234 &rfds,
SELECT_TYPE_ARG234 &wfds, SELECT_TYPE_ARG234 &efds, tvp);
result = ret > 0 && FD_ISSET(fd, &rfds);
if (result)
--ret;
else if (interrupted != NULL && ret > 0)
*interrupted = TRUE;

#if defined(EINTR)
if (ret == -1 && errno == EINTR)
{


if (do_resize)
handle_resize();





goto select_eintr;
}
#endif
#if defined(__TANDEM)
if (ret == -1 && errno == ENOTSUP)
{
FD_ZERO(&rfds);
FD_ZERO(&efds);
ret = 0;
}
#endif
#if defined(FEAT_MZSCHEME)
if (ret == 0 && mzquantum_used)

finished = FALSE;
#endif

#if defined(FEAT_XCLIPBOARD)
if (ret > 0 && xterm_Shell != (Widget)0
&& FD_ISSET(ConnectionNumber(xterm_dpy), &rfds))
{
xterm_update(); 


if (--ret == 0 && !input_available())
{

finished = FALSE;
}
}
#endif
#if defined(FEAT_MOUSE_GPM)
if (ret > 0 && gpm_flag && check_for_gpm != NULL && gpm_fd >= 0)
{
if (FD_ISSET(gpm_fd, &efds))
gpm_close();
else if (FD_ISSET(gpm_fd, &rfds))
*check_for_gpm = 1;
}
#endif
#if defined(USE_XSMP)
if (ret > 0 && xsmp_icefd != -1)
{
if (FD_ISSET(xsmp_icefd, &efds))
{
if (p_verbose > 0)
verb_msg(_("XSMP lost ICE connection"));
xsmp_close();
if (--ret == 0)
finished = FALSE; 
}
else if (FD_ISSET(xsmp_icefd, &rfds))
{
busy = TRUE;
xsmp_handle_requests();
busy = FALSE;
if (--ret == 0)
finished = FALSE; 
}
}
#endif
#if defined(FEAT_JOB_CHANNEL)

if (ret >= 0)
ret = channel_select_check(ret, &rfds, &wfds);
#endif

#endif 

#if defined(MAY_LOOP)
if (finished || msec == 0)
break;

#if defined(FEAT_CLIENTSERVER)
if (server_waiting())
break;
#endif


if (msec > 0)
{
#if defined(ELAPSED_FUNC)

msec = start_msec - ELAPSED_FUNC(start_tv);
#else

msec = msec / 2;
#endif
if (msec <= 0)
break; 
}
#endif
}

return result;
}







int
mch_expandpath(
garray_T *gap,
char_u *path,
int flags) 
{
return unix_expandpath(gap, path, 0, flags, FALSE);
}














#if !defined(SEEK_SET)
#define SEEK_SET 0
#endif
#if !defined(SEEK_END)
#define SEEK_END 2
#endif

#define SHELL_SPECIAL (char_u *)"\t \"&'$;<>()\\|"

int
mch_expand_wildcards(
int num_pat,
char_u **pat,
int *num_file,
char_u ***file,
int flags) 
{
int i;
size_t len;
long llen;
char_u *p;
int dir;




int j;
char_u *tempname;
char_u *command;
FILE *fd;
char_u *buffer;
#define STYLE_ECHO 0 
#define STYLE_GLOB 1 
#define STYLE_VIMGLOB 2 
#define STYLE_PRINT 3 
#define STYLE_BT 4 

int shell_style = STYLE_ECHO;
int check_spaces;
static int did_find_nul = FALSE;
int ampersand = FALSE;

static char *sh_vimglob_func = "vimglob() { while [ $#-ge 1 ]; do echo \"$1\"; shift; done }; vimglob >";

*num_file = 0; 
*file = NULL;





if (!have_wildcard(num_pat, pat))
return save_patterns(num_pat, pat, num_file, file);

#if defined(HAVE_SANDBOX)

if (sandbox != 0 && check_secure())
return FAIL;
#endif




if (secure || restricted)
for (i = 0; i < num_pat; ++i)
if (vim_strchr(pat[i], '`') != NULL
&& (check_restricted() || check_secure()))
return FAIL;




if ((tempname = vim_tempname('o', FALSE)) == NULL)
{
emsg(_(e_notmp));
return FAIL;
}















if (num_pat == 1 && *pat[0] == '`'
&& (len = STRLEN(pat[0])) > 2
&& *(pat[0] + len - 1) == '`')
shell_style = STYLE_BT;
else if ((len = STRLEN(p_sh)) >= 3)
{
if (STRCMP(p_sh + len - 3, "csh") == 0)
shell_style = STYLE_GLOB;
else if (STRCMP(p_sh + len - 3, "zsh") == 0)
shell_style = STYLE_PRINT;
}
if (shell_style == STYLE_ECHO && strstr((char *)gettail(p_sh),
"sh") != NULL)
shell_style = STYLE_VIMGLOB;




len = STRLEN(tempname) + 29;
if (shell_style == STYLE_VIMGLOB)
len += STRLEN(sh_vimglob_func);

for (i = 0; i < num_pat; ++i)
{


#if defined(USE_SYSTEM)
len += STRLEN(pat[i]) + 3; 
#else
++len; 
for (j = 0; pat[i][j] != NUL; ++j)
{
if (vim_strchr(SHELL_SPECIAL, pat[i][j]) != NULL)
++len; 
++len;
}
#endif
}
command = alloc(len);
if (command == NULL)
{

vim_free(tempname);
return FAIL;
}









if (shell_style == STYLE_BT)
{

STRCPY(command, "(");
STRCAT(command, pat[0] + 1); 
p = command + STRLEN(command) - 1;
*p-- = ')'; 
while (p > command && VIM_ISWHITE(*p))
--p;
if (*p == '&') 
{
ampersand = TRUE;
*p = ' ';
}
STRCAT(command, ">");
}
else
{
if (flags & EW_NOTFOUND)
STRCPY(command, "set nonomatch; ");
else
STRCPY(command, "unset nonomatch; ");
if (shell_style == STYLE_GLOB)
STRCAT(command, "glob >");
else if (shell_style == STYLE_PRINT)
STRCAT(command, "print -N >");
else if (shell_style == STYLE_VIMGLOB)
STRCAT(command, sh_vimglob_func);
else
STRCAT(command, "echo >");
}

STRCAT(command, tempname);

if (shell_style != STYLE_BT)
for (i = 0; i < num_pat; ++i)
{



#if defined(USE_SYSTEM)
STRCAT(command, " \"");
STRCAT(command, pat[i]);
STRCAT(command, "\"");
#else
int intick = FALSE;

p = command + STRLEN(command);
*p++ = ' ';
for (j = 0; pat[i][j] != NUL; ++j)
{
if (pat[i][j] == '`')
intick = !intick;
else if (pat[i][j] == '\\' && pat[i][j + 1] != NUL)
{



if (intick
|| vim_strchr(SHELL_SPECIAL, pat[i][j + 1]) != NULL
|| pat[i][j + 1] == '`')
*p++ = '\\';
++j;
}
else if (!intick
&& ((flags & EW_KEEPDOLLAR) == 0 || pat[i][j] != '$')
&& vim_strchr(SHELL_SPECIAL, pat[i][j]) != NULL)



*p++ = '\\';


*p++ = pat[i][j];
}
*p = NUL;
#endif
}
if (flags & EW_SILENT)
show_shell_mess = FALSE;
if (ampersand)
STRCAT(command, "&"); 






if (shell_style == STYLE_PRINT)
extra_shell_arg = (char_u *)"-G"; 






else if (shell_style == STYLE_GLOB && !have_dollars(num_pat, pat))
extra_shell_arg = (char_u *)"-f"; 




i = call_shell(command, SHELL_EXPAND | SHELL_SILENT);



if (ampersand)
mch_delay(10L, TRUE);

extra_shell_arg = NULL; 
show_shell_mess = TRUE;
vim_free(command);

if (i != 0) 
{
mch_remove(tempname);
vim_free(tempname);





#if !defined(USE_SYSTEM)
if (!(flags & EW_SILENT))
#endif
{
redraw_later_clear(); 
msg_putchar('\n'); 
cmdline_row = Rows - 1; 
#if defined(USE_SYSTEM)
if (!(flags & EW_SILENT))
#endif
{
msg(_(e_wildexpand));
msg_start(); 
}
}


if (shell_style == STYLE_BT)
return FAIL;
goto notfound;
}




fd = fopen((char *)tempname, READBIN);
if (fd == NULL)
{

if (!(flags & EW_SILENT))
{
msg(_(e_wildexpand));
msg_start(); 
}
vim_free(tempname);
goto notfound;
}
fseek(fd, 0L, SEEK_END);
llen = ftell(fd); 
fseek(fd, 0L, SEEK_SET);
if (llen < 0)

buffer = NULL;
else
buffer = alloc(llen + 1);
if (buffer == NULL)
{

mch_remove(tempname);
vim_free(tempname);
fclose(fd);
return FAIL;
}
len = llen;
i = fread((char *)buffer, 1, len, fd);
fclose(fd);
mch_remove(tempname);
if (i != (int)len)
{

semsg(_(e_notread), tempname);
vim_free(tempname);
vim_free(buffer);
return FAIL;
}
vim_free(tempname);

#if defined(__CYGWIN__)

p = buffer;
for (i = 0; i < (int)len; ++i)
if (!(buffer[i] == CAR && buffer[i + 1] == NL))
*p++ = buffer[i];
len = p - buffer;
#endif



if (shell_style == STYLE_ECHO)
{
buffer[len] = '\n'; 
p = buffer;
for (i = 0; *p != '\n'; ++i) 
{
while (*p != ' ' && *p != '\n')
++p;
p = skipwhite(p); 
}
}

else if (shell_style == STYLE_BT || shell_style == STYLE_VIMGLOB)
{
buffer[len] = NUL; 
p = buffer;
for (i = 0; *p != NUL; ++i) 
{
while (*p != '\n' && *p != NUL)
++p;
if (*p != NUL)
++p;
p = skipwhite(p); 
}
}

else
{








check_spaces = FALSE;
if (shell_style == STYLE_PRINT && !did_find_nul)
{

buffer[len] = NUL;
if (len && (int)STRLEN(buffer) < (int)len)
did_find_nul = TRUE;
else
check_spaces = TRUE;
}





if (len && buffer[len - 1] == NUL)
--len;
else
buffer[len] = NUL;
i = 0;
for (p = buffer; p < buffer + len; ++p)
if (*p == NUL || (*p == ' ' && check_spaces)) 
{
++i;
*p = NUL;
}
if (len)
++i; 
}
if (i == 0)
{





vim_free(buffer);
goto notfound;
}
*num_file = i;
*file = ALLOC_MULT(char_u *, i);
if (*file == NULL)
{

vim_free(buffer);
return FAIL;
}




p = buffer;
for (i = 0; i < *num_file; ++i)
{
(*file)[i] = p;

if (shell_style == STYLE_ECHO || shell_style == STYLE_BT
|| shell_style == STYLE_VIMGLOB)
{
while (!(shell_style == STYLE_ECHO && *p == ' ')
&& *p != '\n' && *p != NUL)
++p;
if (p == buffer + len) 
*p = NUL;
else
{
*p++ = NUL;
p = skipwhite(p); 
}
}
else 
{
while (*p && p < buffer + len) 
++p;
++p; 
}
}




for (j = 0, i = 0; i < *num_file; ++i)
{

if (!(flags & EW_NOTFOUND) && mch_getperm((*file)[i]) < 0)
continue;


dir = (mch_isdir((*file)[i]));
if ((dir && !(flags & EW_DIR)) || (!dir && !(flags & EW_FILE)))
continue;


if (!dir && (flags & EW_EXEC)
&& !mch_can_exe((*file)[i], NULL, !(flags & EW_SHELLCMD)))
continue;

p = alloc(STRLEN((*file)[i]) + 1 + dir);
if (p)
{
STRCPY(p, (*file)[i]);
if (dir)
add_pathsep(p); 
(*file)[j++] = p;
}
}
vim_free(buffer);
*num_file = j;

if (*num_file == 0) 
{
VIM_CLEAR(*file);
goto notfound;
}

return OK;

notfound:
if (flags & EW_NOTFOUND)
return save_patterns(num_pat, pat, num_file, file);
return FAIL;
}

#endif 

static int
save_patterns(
int num_pat,
char_u **pat,
int *num_file,
char_u ***file)
{
int i;
char_u *s;

*file = ALLOC_MULT(char_u *, num_pat);
if (*file == NULL)
return FAIL;
for (i = 0; i < num_pat; i++)
{
s = vim_strsave(pat[i]);
if (s != NULL)


backslash_halve(s);
(*file)[i] = s;
}
*num_file = num_pat;
return OK;
}





int
mch_has_exp_wildcard(char_u *p)
{
for ( ; *p; MB_PTR_ADV(p))
{
if (*p == '\\' && p[1] != NUL)
++p;
else
if (vim_strchr((char_u *)
#if defined(VMS)
"*?%"
#else
"*?[{'"
#endif
, *p) != NULL)
return TRUE;
}
return FALSE;
}





int
mch_has_wildcard(char_u *p)
{
for ( ; *p; MB_PTR_ADV(p))
{
if (*p == '\\' && p[1] != NUL)
++p;
else
if (vim_strchr((char_u *)
#if defined(VMS)
"*?%$"
#else
"*?[{`'$"
#endif
, *p) != NULL
|| (*p == '~' && p[1] != NUL))
return TRUE;
}
return FALSE;
}

static int
have_wildcard(int num, char_u **file)
{
int i;

for (i = 0; i < num; i++)
if (mch_has_wildcard(file[i]))
return 1;
return 0;
}

static int
have_dollars(int num, char_u **file)
{
int i;

for (i = 0; i < num; i++)
if (vim_strchr(file[i], '$') != NULL)
return TRUE;
return FALSE;
}

#if !defined(HAVE_RENAME) || defined(PROTO)





int
mch_rename(const char *src, const char *dest)
{
struct stat st;

if (stat(dest, &st) >= 0) 
return -1;
if (link(src, dest) != 0) 
return -1;
if (mch_remove(src) == 0) 
return 0;
return -1;
}
#endif 

#if defined(FEAT_MOUSE_GPM) || defined(PROTO)




static int
gpm_open(void)
{
static Gpm_Connect gpm_connect; 

if (!gpm_flag)
{
gpm_connect.eventMask = (GPM_UP | GPM_DRAG | GPM_DOWN);
gpm_connect.defaultMask = ~GPM_HARD;

gpm_connect.minMod = 0; 
gpm_connect.maxMod = 0xffff;
if (Gpm_Open(&gpm_connect, 0) > 0)
{




#if defined(SIGTSTP)
signal(SIGTSTP, restricted ? SIG_IGN : SIG_DFL);
#endif
return 1; 
}
if (gpm_fd == -2)
Gpm_Close(); 
return 0;
}
return 1; 
}




int
gpm_enabled(void)
{
return gpm_flag && gpm_fd >= 0;
}




static void
gpm_close(void)
{
if (gpm_enabled())
Gpm_Close();
}






static int
mch_gpm_process(void)
{
int button;
static Gpm_Event gpm_event;
char_u string[6];
int_u vim_modifiers;
int row,col;
unsigned char buttons_mask;
unsigned char gpm_modifiers;
static unsigned char old_buttons = 0;

Gpm_GetEvent(&gpm_event);

#if defined(FEAT_GUI)

if (hold_gui_events)
return 0;
#endif

row = gpm_event.y - 1;
col = gpm_event.x - 1;

string[0] = ESC; 
string[1] = 'M';
string[2] = 'G';
switch (GPM_BARE_EVENTS(gpm_event.type))
{
case GPM_DRAG:
string[3] = MOUSE_DRAG;
break;
case GPM_DOWN:
buttons_mask = gpm_event.buttons & ~old_buttons;
old_buttons = gpm_event.buttons;
switch (buttons_mask)
{
case GPM_B_LEFT:
button = MOUSE_LEFT;
break;
case GPM_B_MIDDLE:
button = MOUSE_MIDDLE;
break;
case GPM_B_RIGHT:
button = MOUSE_RIGHT;
break;
default:
return 0;


}
string[3] = (char_u)(button | 0x20);
SET_NUM_MOUSE_CLICKS(string[3], gpm_event.clicks + 1);
break;
case GPM_UP:
string[3] = MOUSE_RELEASE;
old_buttons &= ~gpm_event.buttons;
break;
default:
return 0;
}

gpm_modifiers = gpm_event.modifiers;
vim_modifiers = 0x0;



if (gpm_modifiers & ((1 << KG_SHIFT) | (1 << KG_SHIFTR) | (1 << KG_SHIFTL)))
vim_modifiers |= MOUSE_SHIFT;

if (gpm_modifiers & ((1 << KG_CTRL) | (1 << KG_CTRLR) | (1 << KG_CTRLL)))
vim_modifiers |= MOUSE_CTRL;
if (gpm_modifiers & ((1 << KG_ALT) | (1 << KG_ALTGR)))
vim_modifiers |= MOUSE_ALT;
string[3] |= vim_modifiers;
string[4] = (char_u)(col + ' ' + 1);
string[5] = (char_u)(row + ' ' + 1);
add_to_input_buf(string, 6);
return 6;
}
#endif 

#if defined(FEAT_SYSMOUSE)






static int
sysmouse_open(void)
{
struct mouse_info mouse;

mouse.operation = MOUSE_MODE;
mouse.u.mode.mode = 0;
mouse.u.mode.signal = SIGUSR2;
if (ioctl(1, CONS_MOUSECTL, &mouse) != -1)
{
signal(SIGUSR2, (RETSIGTYPE (*)())sig_sysmouse);
mouse.operation = MOUSE_SHOW;
ioctl(1, CONS_MOUSECTL, &mouse);
return OK;
}
return FAIL;
}





static void
sysmouse_close(void)
{
struct mouse_info mouse;

signal(SIGUSR2, restricted ? SIG_IGN : SIG_DFL);
mouse.operation = MOUSE_MODE;
mouse.u.mode.mode = 0;
mouse.u.mode.signal = 0;
ioctl(1, CONS_MOUSECTL, &mouse);
}




static RETSIGTYPE
sig_sysmouse SIGDEFARG(sigarg)
{
struct mouse_info mouse;
struct video_info video;
char_u string[6];
int row, col;
int button;
int buttons;
static int oldbuttons = 0;

#if defined(FEAT_GUI)

if (hold_gui_events)
return;
#endif

mouse.operation = MOUSE_GETINFO;
if (ioctl(1, FBIO_GETMODE, &video.vi_mode) != -1
&& ioctl(1, FBIO_MODEINFO, &video) != -1
&& ioctl(1, CONS_MOUSECTL, &mouse) != -1
&& video.vi_cheight > 0 && video.vi_cwidth > 0)
{
row = mouse.u.data.y / video.vi_cheight;
col = mouse.u.data.x / video.vi_cwidth;
buttons = mouse.u.data.buttons;
string[0] = ESC; 
string[1] = 'M';
string[2] = 'S';
if (oldbuttons == buttons && buttons != 0)
{
button = MOUSE_DRAG;
}
else
{
switch (buttons)
{
case 0:
button = MOUSE_RELEASE;
break;
case 1:
button = MOUSE_LEFT;
break;
case 2:
button = MOUSE_MIDDLE;
break;
case 4:
button = MOUSE_RIGHT;
break;
default:
return;
}
oldbuttons = buttons;
}
string[3] = (char_u)(button);
string[4] = (char_u)(col + ' ' + 1);
string[5] = (char_u)(row + ' ' + 1);
add_to_input_buf(string, 6);
}
return;
}
#endif 

#if defined(FEAT_LIBCALL) || defined(PROTO)
typedef char_u * (*STRPROCSTR)(char_u *);
typedef char_u * (*INTPROCSTR)(int);
typedef int (*STRPROCINT)(char_u *);
typedef int (*INTPROCINT)(int);





int
mch_libcall(
char_u *libname,
char_u *funcname,
char_u *argstring, 
int argint,
char_u **string_result, 
int *number_result)
{
#if defined(USE_DLOPEN)
void *hinstLib;
char *dlerr = NULL;
#else
shl_t hinstLib;
#endif
STRPROCSTR ProcAdd;
INTPROCSTR ProcAddI;
char_u *retval_str = NULL;
int retval_int = 0;
int success = FALSE;




#if defined(USE_DLOPEN)

(void)dlerror();

hinstLib = dlopen((char *)libname, RTLD_LAZY
#if defined(RTLD_LOCAL)
| RTLD_LOCAL
#endif
);
if (hinstLib == NULL)
{

dlerr = (char *)dlerror();
if (dlerr != NULL)
semsg(_("dlerror = \"%s\""), dlerr);
}
#else
hinstLib = shl_load((const char*)libname, BIND_IMMEDIATE|BIND_VERBOSE, 0L);
#endif


if (hinstLib != NULL)
{
#if defined(USING_SETJMP)




mch_startjmp();
if (SETJMP(lc_jump_env) != 0)
{
success = FALSE;
#if defined(USE_DLOPEN)
dlerr = NULL;
#endif
mch_didjmp();
}
else
#endif
{
retval_str = NULL;
retval_int = 0;

if (argstring != NULL)
{
#if defined(USE_DLOPEN)
*(void **)(&ProcAdd) = dlsym(hinstLib, (const char *)funcname);
dlerr = (char *)dlerror();
#else
if (shl_findsym(&hinstLib, (const char *)funcname,
TYPE_PROCEDURE, (void *)&ProcAdd) < 0)
ProcAdd = NULL;
#endif
if ((success = (ProcAdd != NULL
#if defined(USE_DLOPEN)
&& dlerr == NULL
#endif
)))
{
if (string_result == NULL)
retval_int = ((STRPROCINT)ProcAdd)(argstring);
else
retval_str = (ProcAdd)(argstring);
}
}
else
{
#if defined(USE_DLOPEN)
*(void **)(&ProcAddI) = dlsym(hinstLib, (const char *)funcname);
dlerr = (char *)dlerror();
#else
if (shl_findsym(&hinstLib, (const char *)funcname,
TYPE_PROCEDURE, (void *)&ProcAddI) < 0)
ProcAddI = NULL;
#endif
if ((success = (ProcAddI != NULL
#if defined(USE_DLOPEN)
&& dlerr == NULL
#endif
)))
{
if (string_result == NULL)
retval_int = ((INTPROCINT)ProcAddI)(argint);
else
retval_str = (ProcAddI)(argint);
}
}



if (string_result == NULL)
*number_result = retval_int;
else if (retval_str != NULL
&& retval_str != (char_u *)1
&& retval_str != (char_u *)-1)
*string_result = vim_strsave(retval_str);
}

#if defined(USING_SETJMP)
mch_endjmp();
#if defined(SIGHASARG)
if (lc_signal != 0)
{
int i;


for (i = 0; signal_info[i].sig != -1; i++)
if (lc_signal == signal_info[i].sig)
break;
semsg("E368: got SIG%s in libcall()", signal_info[i].name);
}
#endif
#endif

#if defined(USE_DLOPEN)

if (dlerr != NULL)
semsg(_("dlerror = \"%s\""), dlerr);


(void)dlclose(hinstLib);
#else
(void)shl_unload(hinstLib);
#endif
}

if (!success)
{
semsg(_(e_libcall), funcname);
return FAIL;
}

return OK;
}
#endif

#if (defined(FEAT_X11) && defined(FEAT_XCLIPBOARD)) || defined(PROTO)
static int xterm_trace = -1; 
static int xterm_button;




void
setup_term_clip(void)
{
int z = 0;
char *strp = "";
Widget AppShell;

if (!x_connect_to_server())
return;

open_app_context();
if (app_context != NULL && xterm_Shell == (Widget)0)
{
int (*oldhandler)();
#if defined(USING_SETJMP)
int (*oldIOhandler)();
#endif
#if defined(ELAPSED_FUNC)
elapsed_T start_tv;

if (p_verbose > 0)
ELAPSED_INIT(start_tv);
#endif


oldhandler = XSetErrorHandler(x_error_check);

#if defined(USING_SETJMP)

oldIOhandler = XSetIOErrorHandler(x_IOerror_check);
mch_startjmp();
if (SETJMP(lc_jump_env) != 0)
{
mch_didjmp();
xterm_dpy = NULL;
}
else
#endif
{
xterm_dpy = XtOpenDisplay(app_context, xterm_display,
"vim_xterm", "Vim_xterm", NULL, 0, &z, &strp);
if (xterm_dpy != NULL)
xterm_dpy_retry_count = 0;
#if defined(USING_SETJMP)
mch_endjmp();
#endif
}

#if defined(USING_SETJMP)

(void)XSetIOErrorHandler(oldIOhandler);
#endif

(void)XSetErrorHandler(oldhandler);

if (xterm_dpy == NULL)
{
if (p_verbose > 0)
verb_msg(_("Opening the X display failed"));
return;
}


(void)XSetIOErrorHandler(x_IOerror_handler);

#if defined(ELAPSED_FUNC)
if (p_verbose > 0)
{
verbose_enter();
xopen_message(ELAPSED_FUNC(start_tv));
verbose_leave();
}
#endif


AppShell = XtVaAppCreateShell("vim_xterm", "Vim_xterm",
applicationShellWidgetClass, xterm_dpy,
NULL);
if (AppShell == (Widget)0)
return;
xterm_Shell = XtVaCreatePopupShell("VIM",
topLevelShellWidgetClass, AppShell,
XtNmappedWhenManaged, 0,
XtNwidth, 1,
XtNheight, 1,
NULL);
if (xterm_Shell == (Widget)0)
return;

x11_setup_atoms(xterm_dpy);
x11_setup_selection(xterm_Shell);
if (x11_display == NULL)
x11_display = xterm_dpy;

XtRealizeWidget(xterm_Shell);
XSync(xterm_dpy, False);
xterm_update();
}
if (xterm_Shell != (Widget)0)
{
clip_init(TRUE);
if (x11_window == 0 && (strp = getenv("WINDOWID")) != NULL)
x11_window = (Window)atol(strp);

if (test_x11_window(xterm_dpy) == FAIL)
x11_window = 0;
if (x11_window != 0)
xterm_trace = 0;
}
}

void
start_xterm_trace(int button)
{
if (x11_window == 0 || xterm_trace < 0 || xterm_Shell == (Widget)0)
return;
xterm_trace = 1;
xterm_button = button;
do_xterm_trace();
}


void
stop_xterm_trace(void)
{
if (xterm_trace < 0)
return;
xterm_trace = 0;
}





static int
do_xterm_trace(void)
{
Window root, child;
int root_x, root_y;
int win_x, win_y;
int row, col;
int_u mask_return;
char_u buf[50];
char_u *strp;
long got_hints;
static char_u *mouse_code;
static char_u mouse_name[2] = {KS_MOUSE, KE_FILLER};
static int prev_row = 0, prev_col = 0;
static XSizeHints xterm_hints;

if (xterm_trace <= 0)
return FALSE;

if (xterm_trace == 1)
{


if (!XGetWMNormalHints(xterm_dpy, x11_window, &xterm_hints, &got_hints)
|| !(got_hints & PResizeInc)
|| xterm_hints.width_inc <= 1
|| xterm_hints.height_inc <= 1)
{
xterm_trace = -1; 
return FALSE;
}


mouse_code = find_termcode(mouse_name);
prev_row = mouse_row;
prev_col = mouse_col;
xterm_trace = 2;



XQueryPointer(xterm_dpy, x11_window, &root, &child, &root_x, &root_y,
&win_x, &win_y, &mask_return);
xterm_hints.y = win_y - (xterm_hints.height_inc * mouse_row)
- (xterm_hints.height_inc / 2);
if (xterm_hints.y <= xterm_hints.height_inc / 2)
xterm_hints.y = 2;
xterm_hints.x = win_x - (xterm_hints.width_inc * mouse_col)
- (xterm_hints.width_inc / 2);
if (xterm_hints.x <= xterm_hints.width_inc / 2)
xterm_hints.x = 2;
return TRUE;
}
if (mouse_code == NULL || STRLEN(mouse_code) > 45)
{
xterm_trace = 0;
return FALSE;
}

XQueryPointer(xterm_dpy, x11_window, &root, &child, &root_x, &root_y,
&win_x, &win_y, &mask_return);

row = check_row((win_y - xterm_hints.y) / xterm_hints.height_inc);
col = check_col((win_x - xterm_hints.x) / xterm_hints.width_inc);
if (row == prev_row && col == prev_col)
return TRUE;

STRCPY(buf, mouse_code);
strp = buf + STRLEN(buf);
*strp++ = (xterm_button | MOUSE_DRAG) & ~0x20;
*strp++ = (char_u)(col + ' ' + 1);
*strp++ = (char_u)(row + ' ' + 1);
*strp = 0;
add_to_input_buf(buf, STRLEN(buf));

prev_row = row;
prev_col = col;
return TRUE;
}

#if defined(FEAT_GUI) || defined(FEAT_XCLIPBOARD) || defined(PROTO)



void
clear_xterm_clip(void)
{
if (xterm_Shell != (Widget)0)
{
XtDestroyWidget(xterm_Shell);
xterm_Shell = (Widget)0;
}
if (xterm_dpy != NULL)
{
#if 0

XtCloseDisplay(xterm_dpy);
#endif
if (x11_display == xterm_dpy)
x11_display = NULL;
xterm_dpy = NULL;
}
#if 0
if (app_context != (XtAppContext)NULL)
{

XtDestroyApplicationContext(app_context);
app_context = (XtAppContext)NULL;
}
#endif
}
#endif




static void
clip_update(void)
{
#if defined(FEAT_GUI)
if (gui.in_use)
gui_mch_update();
else
#endif
if (xterm_Shell != (Widget)0)
xterm_update();
}







static void
xterm_update(void)
{
XEvent event;

for (;;)
{
XtInputMask mask = XtAppPending(app_context);

if (mask == 0 || vim_is_input_buf_full())
break;

if (mask & XtIMXEvent)
{

XtAppNextEvent(app_context, &event);
#if defined(FEAT_CLIENTSERVER)
{
XPropertyEvent *e = (XPropertyEvent *)&event;

if (e->type == PropertyNotify && e->window == commWindow
&& e->atom == commProperty && e->state == PropertyNewValue)
serverEventProc(xterm_dpy, &event, 0);
}
#endif
XtDispatchEvent(&event);
}
else
{

XtAppProcessEvent(app_context, mask);
}
}
}

int
clip_xterm_own_selection(Clipboard_T *cbd)
{
if (xterm_Shell != (Widget)0)
return clip_x11_own_selection(xterm_Shell, cbd);
return FAIL;
}

void
clip_xterm_lose_selection(Clipboard_T *cbd)
{
if (xterm_Shell != (Widget)0)
clip_x11_lose_selection(xterm_Shell, cbd);
}

void
clip_xterm_request_selection(Clipboard_T *cbd)
{
if (xterm_Shell != (Widget)0)
clip_x11_request_selection(xterm_Shell, xterm_dpy, cbd);
}

void
clip_xterm_set_selection(Clipboard_T *cbd)
{
clip_x11_set_selection(cbd);
}
#endif


#if defined(USE_XSMP) || defined(PROTO)




#if defined(FEAT_GUI) && defined(USE_XSMP_INTERACT)




static void
xsmp_handle_interaction(SmcConn smc_conn, SmPointer client_data UNUSED)
{
cmdmod_T save_cmdmod;
int cancel_shutdown = False;

save_cmdmod = cmdmod;
cmdmod.confirm = TRUE;
if (check_changed_any(FALSE, FALSE))

cancel_shutdown = True;
cmdmod = save_cmdmod;
setcursor(); 
out_flush();


SmcInteractDone(smc_conn, cancel_shutdown);





if (!cancel_shutdown)
{
xsmp.save_yourself = False;
SmcSaveYourselfDone(smc_conn, True);
}
}
#endif




static void
xsmp_handle_save_yourself(
SmcConn smc_conn,
SmPointer client_data UNUSED,
int save_type UNUSED,
Bool shutdown,
int interact_style UNUSED,
Bool fast UNUSED)
{

if (xsmp.save_yourself)
SmcSaveYourselfDone(smc_conn, True);
xsmp.save_yourself = True;
xsmp.shutdown = shutdown;


out_flush();
ml_sync_all(FALSE, FALSE); 

if (p_verbose > 0)
verb_msg(_("XSMP handling save-yourself request"));

#if defined(FEAT_GUI) && defined(USE_XSMP_INTERACT)

if (shutdown && !fast && gui.in_use)

SmcInteractRequest(smc_conn, SmDialogError,
xsmp_handle_interaction, client_data);
else
#endif
{

SmcSaveYourselfDone(smc_conn, True);
xsmp.save_yourself = False;
}
}





static void
xsmp_die(SmcConn smc_conn UNUSED, SmPointer client_data UNUSED)
{
xsmp_close();


getout_preserve_modified(0);
}





static void
xsmp_save_complete(
SmcConn smc_conn UNUSED,
SmPointer client_data UNUSED)
{
xsmp.save_yourself = False;
}






static void
xsmp_shutdown_cancelled(
SmcConn smc_conn,
SmPointer client_data UNUSED)
{
if (xsmp.save_yourself)
SmcSaveYourselfDone(smc_conn, True);
xsmp.save_yourself = False;
xsmp.shutdown = False;
}





static void
xsmp_ice_connection(
IceConn iceConn,
IcePointer clientData UNUSED,
Bool opening,
IcePointer *watchData UNUSED)
{

if (opening)
{
xsmp_icefd = IceConnectionNumber(iceConn);
IceRemoveConnectionWatch(xsmp_ice_connection, NULL);
}
}



int
xsmp_handle_requests(void)
{
Bool rep;

if (IceProcessMessages(xsmp.iceconn, NULL, &rep)
== IceProcessMessagesIOError)
{

if (p_verbose > 0)
verb_msg(_("XSMP lost ICE connection"));
xsmp_close();
return FAIL;
}
else
return OK;
}

static int dummy;


void
xsmp_init(void)
{
char errorstring[80];
SmcCallbacks smcallbacks;
#if 0
SmPropValue smname;
SmProp smnameprop;
SmProp *smprops[1];
#endif

if (p_verbose > 0)
verb_msg(_("XSMP opening connection"));

xsmp.save_yourself = xsmp.shutdown = False;


smcallbacks.save_yourself.callback = xsmp_handle_save_yourself;
smcallbacks.save_yourself.client_data = NULL;
smcallbacks.die.callback = xsmp_die;
smcallbacks.die.client_data = NULL;
smcallbacks.save_complete.callback = xsmp_save_complete;
smcallbacks.save_complete.client_data = NULL;
smcallbacks.shutdown_cancelled.callback = xsmp_shutdown_cancelled;
smcallbacks.shutdown_cancelled.client_data = NULL;



if (IceAddConnectionWatch(xsmp_ice_connection, &dummy) == 0)
{
if (p_verbose > 0)
verb_msg(_("XSMP ICE connection watch failed"));
return;
}


xsmp.smcconn = SmcOpenConnection(
NULL,
NULL,
SmProtoMajor,
SmProtoMinor,
SmcSaveYourselfProcMask | SmcDieProcMask
| SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask,
&smcallbacks,
NULL,
&xsmp.clientid,
sizeof(errorstring) - 1,
errorstring);
if (xsmp.smcconn == NULL)
{
char errorreport[132];

if (p_verbose > 0)
{
vim_snprintf(errorreport, sizeof(errorreport),
_("XSMP SmcOpenConnection failed: %s"), errorstring);
verb_msg(errorreport);
}
return;
}
xsmp.iceconn = SmcGetIceConnection(xsmp.smcconn);

#if 0

smname.value = "vim";
smname.length = 3;
smnameprop.name = "SmProgram";
smnameprop.type = "SmARRAY8";
smnameprop.num_vals = 1;
smnameprop.vals = &smname;

smprops[0] = &smnameprop;
SmcSetProperties(xsmp.smcconn, 1, smprops);
#endif
}



void
xsmp_close(void)
{
if (xsmp_icefd != -1)
{
SmcCloseConnection(xsmp.smcconn, 0, NULL);
if (xsmp.clientid != NULL)
free(xsmp.clientid);
xsmp.clientid = NULL;
xsmp_icefd = -1;
}
}
#endif 


#if defined(EBCDIC)

char CtrlTable[] =
{

0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0x1E,
0x1F,

0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0x1F,

0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0x01,
0x02,
0x03,
0x37,
0x2D,
0x2E,
0x2F,
0x16,
0x05,

0, 0, 0, 0, 0, 0, 0,
0x15,
0x0B,
0x0C,
0x0D,
0x0E,
0x0F,
0x10,
0x11,
0x12,

0, 0, 0, 0, 0, 0, 0, 0,
0x13,
0x3C,
0x3D,
0x32,
0x26,
0x18,
0x19,
0x3F,

0, 0, 0,
0x27,

0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0x1D,
0, 0, 0,
0x01,
0x02,
0x03,
0x37,
0x2D,
0x2E,
0x2F,
0x16,
0x05,
0, 0, 0, 0, 0, 0, 0,
0x15,
0x0B,
0x0C,
0x0D,
0x0E,
0x0F,
0x10,
0x11,
0x12,
0, 0, 0, 0, 0, 0,
0x1C,
0,
0x13,
0x3C,
0x3D,
0x32,
0x26,
0x18,
0x19,
0x3F,
0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

char MetaCharTable[]=
{
0, 0, 0, 0,'\\', 0,'F', 0,'W','M','N', 0, 0, 0, 0, 0,
0, 0, 0, 0,']', 0, 0,'G', 0, 0,'R','O', 0, 0, 0, 0,
'@','A','B','C','D','E', 0, 0,'H','I','J','K','L', 0, 0, 0,
'P','Q', 0,'S','T','U','V', 0,'X','Y','Z','[', 0, 0,'^', 0
};



char CtrlCharTable[]=
{
124,193,194,195, 0,201, 0, 0, 0, 0, 0,210,211,212,213,214,
215,216,217,226, 0,209,200, 0,231,232, 0, 0,224,189, 95,109,
0, 0, 0, 0, 0, 0,230,173, 0, 0, 0, 0, 0,197,198,199,
0, 0,229, 0, 0, 0, 0,196, 0, 0, 0, 0,227,228, 0,233,
};


#endif
