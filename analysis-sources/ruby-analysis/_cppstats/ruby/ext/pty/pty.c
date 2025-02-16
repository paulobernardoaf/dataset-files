#include "ruby/config.h"
#if defined(RUBY_EXTCONF_H)
#include RUBY_EXTCONF_H
#endif
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif
#if defined(HAVE_SYS_IOCTL_H)
#include <sys/ioctl.h>
#endif
#if defined(HAVE_LIBUTIL_H)
#include <libutil.h>
#endif
#if defined(HAVE_UTIL_H)
#include <util.h>
#endif
#if defined(HAVE_PTY_H)
#include <pty.h>
#endif
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if defined(HAVE_SYS_WAIT_H)
#include <sys/wait.h>
#else
#define WIFSTOPPED(status) (((status) & 0xff) == 0x7f)
#endif
#if defined(HAVE_SYS_STROPTS_H)
#include <sys/stropts.h>
#endif
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include "internal.h"
#include "internal/process.h"
#include "internal/signal.h"
#include "ruby/io.h"
#include "ruby/util.h"
#define DEVICELEN 16
#if !defined(HAVE_SETEUID)
#if defined(HAVE_SETREUID)
#define seteuid(e) setreuid(-1, (e))
#else 
#if defined(HAVE_SETRESUID)
#define seteuid(e) setresuid(-1, (e), -1)
#else 
#endif 
#endif 
#endif 
static VALUE eChildExited;
static VALUE
echild_status(VALUE self)
{
return rb_ivar_get(self, rb_intern("status"));
}
struct pty_info {
int fd;
rb_pid_t child_pid;
};
static void getDevice(int*, int*, char [DEVICELEN], int);
struct child_info {
int master, slave;
char *slavename;
VALUE execarg_obj;
struct rb_execarg *eargp;
};
static int
chfunc(void *data, char *errbuf, size_t errbuf_len)
{
struct child_info *carg = data;
int master = carg->master;
int slave = carg->slave;
#define ERROR_EXIT(str) do { strlcpy(errbuf, (str), errbuf_len); return -1; } while (0)
#if defined(HAVE_SETSID)
(void) setsid();
#else 
#if defined(HAVE_SETPGRP)
#if defined(SETGRP_VOID)
if (setpgrp() == -1)
ERROR_EXIT("setpgrp()");
#else 
if (setpgrp(0, getpid()) == -1)
ERROR_EXIT("setpgrp()");
{
int i = rb_cloexec_open("/dev/tty", O_RDONLY, 0);
if (i < 0) ERROR_EXIT("/dev/tty");
rb_update_max_fd(i);
if (ioctl(i, TIOCNOTTY, (char *)0))
ERROR_EXIT("ioctl(TIOCNOTTY)");
close(i);
}
#endif 
#endif 
#endif 
#if defined(TIOCSCTTY)
close(master);
(void) ioctl(slave, TIOCSCTTY, (char *)0);
#else
close(slave);
slave = rb_cloexec_open(carg->slavename, O_RDWR, 0);
if (slave < 0) {
ERROR_EXIT("open: pty slave");
}
rb_update_max_fd(slave);
close(master);
#endif
dup2(slave,0);
dup2(slave,1);
dup2(slave,2);
if (slave < 0 || slave > 2) (void)!close(slave);
#if defined(HAVE_SETEUID) || defined(HAVE_SETREUID) || defined(HAVE_SETRESUID)
if (seteuid(getuid())) ERROR_EXIT("seteuid()");
#endif
return rb_exec_async_signal_safe(carg->eargp, errbuf, sizeof(errbuf_len));
#undef ERROR_EXIT
}
static void
establishShell(int argc, VALUE *argv, struct pty_info *info,
char SlaveName[DEVICELEN])
{
int master, slave, status = 0;
rb_pid_t pid;
char *p, *getenv();
VALUE v;
struct child_info carg;
char errbuf[32];
if (argc == 0) {
const char *shellname = "/bin/sh";
if ((p = getenv("SHELL")) != NULL) {
shellname = p;
}
else {
#if defined HAVE_PWD_H
const char *username = getenv("USER");
struct passwd *pwent = getpwnam(username ? username : getlogin());
if (pwent && pwent->pw_shell)
shellname = pwent->pw_shell;
#endif
}
v = rb_str_new2(shellname);
argc = 1;
argv = &v;
}
carg.execarg_obj = rb_execarg_new(argc, argv, 1, 0);
carg.eargp = rb_execarg_get(carg.execarg_obj);
rb_execarg_parent_start(carg.execarg_obj);
getDevice(&master, &slave, SlaveName, 0);
carg.master = master;
carg.slave = slave;
carg.slavename = SlaveName;
errbuf[0] = '\0';
pid = rb_fork_async_signal_safe(&status, chfunc, &carg, Qnil, errbuf, sizeof(errbuf));
if (pid < 0) {
int e = errno;
close(master);
close(slave);
rb_execarg_parent_end(carg.execarg_obj);
errno = e;
if (status) rb_jump_tag(status);
rb_sys_fail(errbuf[0] ? errbuf : "fork failed");
}
close(slave);
rb_execarg_parent_end(carg.execarg_obj);
info->child_pid = pid;
info->fd = master;
RB_GC_GUARD(carg.execarg_obj);
}
#if defined(HAVE_POSIX_OPENPT) || defined(HAVE_OPENPTY) || defined(HAVE_PTSNAME)
static int
no_mesg(char *slavedevice, int nomesg)
{
if (nomesg)
return chmod(slavedevice, 0600);
else
return 0;
}
#endif
#if defined(I_PUSH) && !defined(__linux__) && !defined(_AIX)
static inline int
ioctl_I_PUSH(int fd, const char *const name)
{
int ret = 0;
#if defined(I_FIND)
ret = ioctl(fd, I_FIND, name);
#endif
if (ret == 0) {
ret = ioctl(fd, I_PUSH, name);
}
return ret;
}
#endif
static int
get_device_once(int *master, int *slave, char SlaveName[DEVICELEN], int nomesg, int fail)
{
#if defined(HAVE_POSIX_OPENPT)
int masterfd = -1, slavefd = -1;
char *slavedevice;
#if defined(__sun) || defined(__OpenBSD__) || (defined(__FreeBSD__) && __FreeBSD_version < 902000)
if ((masterfd = posix_openpt(O_RDWR|O_NOCTTY)) == -1) goto error;
if (rb_grantpt(masterfd) == -1) goto error;
rb_fd_fix_cloexec(masterfd);
#else
{
int flags = O_RDWR|O_NOCTTY;
#if defined(O_CLOEXEC)
flags |= O_CLOEXEC;
#endif
if ((masterfd = posix_openpt(flags)) == -1) goto error;
}
rb_fd_fix_cloexec(masterfd);
if (rb_grantpt(masterfd) == -1) goto error;
#endif
if (unlockpt(masterfd) == -1) goto error;
if ((slavedevice = ptsname(masterfd)) == NULL) goto error;
if (no_mesg(slavedevice, nomesg) == -1) goto error;
if ((slavefd = rb_cloexec_open(slavedevice, O_RDWR|O_NOCTTY, 0)) == -1) goto error;
rb_update_max_fd(slavefd);
#if defined(I_PUSH) && !defined(__linux__) && !defined(_AIX)
if (ioctl_I_PUSH(slavefd, "ptem") == -1) goto error;
if (ioctl_I_PUSH(slavefd, "ldterm") == -1) goto error;
if (ioctl_I_PUSH(slavefd, "ttcompat") == -1) goto error;
#endif
*master = masterfd;
*slave = slavefd;
strlcpy(SlaveName, slavedevice, DEVICELEN);
return 0;
error:
if (slavefd != -1) close(slavefd);
if (masterfd != -1) close(masterfd);
if (fail) {
rb_raise(rb_eRuntimeError, "can't get Master/Slave device");
}
return -1;
#elif defined HAVE_OPENPTY
if (openpty(master, slave, SlaveName,
(struct termios *)0, (struct winsize *)0) == -1) {
if (!fail) return -1;
rb_raise(rb_eRuntimeError, "openpty() failed");
}
rb_fd_fix_cloexec(*master);
rb_fd_fix_cloexec(*slave);
if (no_mesg(SlaveName, nomesg) == -1) {
if (!fail) return -1;
rb_raise(rb_eRuntimeError, "can't chmod slave pty");
}
return 0;
#elif defined HAVE__GETPTY
char *name;
mode_t mode = nomesg ? 0600 : 0622;
if (!(name = _getpty(master, O_RDWR, mode, 0))) {
if (!fail) return -1;
rb_raise(rb_eRuntimeError, "_getpty() failed");
}
rb_fd_fix_cloexec(*master);
*slave = rb_cloexec_open(name, O_RDWR, 0);
rb_update_max_fd(*slave);
strlcpy(SlaveName, name, DEVICELEN);
return 0;
#elif defined(HAVE_PTSNAME)
int masterfd = -1, slavefd = -1;
char *slavedevice;
void (*s)();
extern char *ptsname(int);
extern int unlockpt(int);
#if defined(__sun)
if((masterfd = open("/dev/ptmx", O_RDWR, 0)) == -1) goto error;
if(rb_grantpt(masterfd) == -1) goto error;
rb_fd_fix_cloexec(masterfd);
#else
if((masterfd = rb_cloexec_open("/dev/ptmx", O_RDWR, 0)) == -1) goto error;
rb_update_max_fd(masterfd);
if(rb_grantpt(masterfd) == -1) goto error;
#endif
if(unlockpt(masterfd) == -1) goto error;
if((slavedevice = ptsname(masterfd)) == NULL) goto error;
if (no_mesg(slavedevice, nomesg) == -1) goto error;
if((slavefd = rb_cloexec_open(slavedevice, O_RDWR, 0)) == -1) goto error;
rb_update_max_fd(slavefd);
#if defined(I_PUSH) && !defined(__linux__) && !defined(_AIX)
if(ioctl_I_PUSH(slavefd, "ptem") == -1) goto error;
if(ioctl_I_PUSH(slavefd, "ldterm") == -1) goto error;
ioctl_I_PUSH(slavefd, "ttcompat");
#endif
*master = masterfd;
*slave = slavefd;
strlcpy(SlaveName, slavedevice, DEVICELEN);
return 0;
error:
if (slavefd != -1) close(slavefd);
if (masterfd != -1) close(masterfd);
if (fail) rb_raise(rb_eRuntimeError, "can't get Master/Slave device");
return -1;
#else
int masterfd = -1, slavefd = -1;
int i;
char MasterName[DEVICELEN];
#define HEX1(c) c"0",c"1",c"2",c"3",c"4",c"5",c"6",c"7", c"8",c"9",c"a",c"b",c"c",c"d",c"e",c"f"
#if defined(__hpux)
static const char MasterDevice[] = "/dev/ptym/pty%s";
static const char SlaveDevice[] = "/dev/pty/tty%s";
static const char deviceNo[][3] = {
HEX1("p"), HEX1("q"), HEX1("r"), HEX1("s"),
HEX1("t"), HEX1("u"), HEX1("v"), HEX1("w"),
};
#elif defined(_IBMESA) 
static const char MasterDevice[] = "/dev/ptyp%s";
static const char SlaveDevice[] = "/dev/ttyp%s";
static const char deviceNo[][3] = {
HEX1("0"), HEX1("1"), HEX1("2"), HEX1("3"),
HEX1("4"), HEX1("5"), HEX1("6"), HEX1("7"),
HEX1("8"), HEX1("9"), HEX1("a"), HEX1("b"),
HEX1("c"), HEX1("d"), HEX1("e"), HEX1("f"),
};
#else 
static const char MasterDevice[] = "/dev/pty%s";
static const char SlaveDevice[] = "/dev/tty%s";
static const char deviceNo[][3] = {
HEX1("p"), HEX1("q"), HEX1("r"), HEX1("s"),
};
#endif
#undef HEX1
for (i = 0; i < numberof(deviceNo); i++) {
const char *const devno = deviceNo[i];
snprintf(MasterName, sizeof MasterName, MasterDevice, devno);
if ((masterfd = rb_cloexec_open(MasterName,O_RDWR,0)) >= 0) {
rb_update_max_fd(masterfd);
*master = masterfd;
snprintf(SlaveName, DEVICELEN, SlaveDevice, devno);
if ((slavefd = rb_cloexec_open(SlaveName,O_RDWR,0)) >= 0) {
rb_update_max_fd(slavefd);
*slave = slavefd;
if (chown(SlaveName, getuid(), getgid()) != 0) goto error;
if (chmod(SlaveName, nomesg ? 0600 : 0622) != 0) goto error;
return 0;
}
close(masterfd);
}
}
error:
if (slavefd != -1) close(slavefd);
if (masterfd != -1) close(masterfd);
if (fail) rb_raise(rb_eRuntimeError, "can't get %s", SlaveName);
return -1;
#endif
}
static void
getDevice(int *master, int *slave, char SlaveName[DEVICELEN], int nomesg)
{
if (get_device_once(master, slave, SlaveName, nomesg, 0)) {
rb_gc();
get_device_once(master, slave, SlaveName, nomesg, 1);
}
}
static VALUE
pty_close_pty(VALUE assoc)
{
VALUE io;
int i;
for (i = 0; i < 2; i++) {
io = rb_ary_entry(assoc, i);
if (RB_TYPE_P(io, T_FILE) && 0 <= RFILE(io)->fptr->fd)
rb_io_close(io);
}
return Qnil;
}
static VALUE
pty_open(VALUE klass)
{
int master_fd, slave_fd;
char slavename[DEVICELEN];
VALUE master_io, slave_file;
rb_io_t *master_fptr, *slave_fptr;
VALUE assoc;
getDevice(&master_fd, &slave_fd, slavename, 1);
master_io = rb_obj_alloc(rb_cIO);
MakeOpenFile(master_io, master_fptr);
master_fptr->mode = FMODE_READWRITE | FMODE_SYNC | FMODE_DUPLEX;
master_fptr->fd = master_fd;
master_fptr->pathv = rb_obj_freeze(rb_sprintf("masterpty:%s", slavename));
slave_file = rb_obj_alloc(rb_cFile);
MakeOpenFile(slave_file, slave_fptr);
slave_fptr->mode = FMODE_READWRITE | FMODE_SYNC | FMODE_DUPLEX | FMODE_TTY;
slave_fptr->fd = slave_fd;
slave_fptr->pathv = rb_obj_freeze(rb_str_new_cstr(slavename));
assoc = rb_assoc_new(master_io, slave_file);
if (rb_block_given_p()) {
return rb_ensure(rb_yield, assoc, pty_close_pty, assoc);
}
return assoc;
}
static VALUE
pty_detach_process(VALUE v)
{
struct pty_info *info = (void *)v;
#if defined(WNOHANG)
int st;
if (rb_waitpid(info->child_pid, &st, WNOHANG) <= 0)
return Qnil;
#endif
rb_detach_process(info->child_pid);
return Qnil;
}
static VALUE
pty_getpty(int argc, VALUE *argv, VALUE self)
{
VALUE res;
struct pty_info info;
rb_io_t *wfptr,*rfptr;
VALUE rport = rb_obj_alloc(rb_cFile);
VALUE wport = rb_obj_alloc(rb_cFile);
char SlaveName[DEVICELEN];
MakeOpenFile(rport, rfptr);
MakeOpenFile(wport, wfptr);
establishShell(argc, argv, &info, SlaveName);
rfptr->mode = rb_io_modestr_fmode("r");
rfptr->fd = info.fd;
rfptr->pathv = rb_obj_freeze(rb_str_new_cstr(SlaveName));
wfptr->mode = rb_io_modestr_fmode("w") | FMODE_SYNC;
wfptr->fd = rb_cloexec_dup(info.fd);
if (wfptr->fd == -1)
rb_sys_fail("dup()");
rb_update_max_fd(wfptr->fd);
wfptr->pathv = rfptr->pathv;
res = rb_ary_new2(3);
rb_ary_store(res,0,(VALUE)rport);
rb_ary_store(res,1,(VALUE)wport);
rb_ary_store(res,2,PIDT2NUM(info.child_pid));
if (rb_block_given_p()) {
rb_ensure(rb_yield, res, pty_detach_process, (VALUE)&info);
return Qnil;
}
return res;
}
NORETURN(static void raise_from_check(rb_pid_t pid, int status));
static void
raise_from_check(rb_pid_t pid, int status)
{
const char *state;
VALUE msg;
VALUE exc;
#if defined(WIFSTOPPED)
#elif defined(IF_STOPPED)
#define WIFSTOPPED(status) IF_STOPPED(status)
#else
---->> Either IF_STOPPED or WIFSTOPPED is needed <<----
#endif 
if (WIFSTOPPED(status)) { 
state = "stopped";
}
else if (kill(pid, 0) == 0) {
state = "changed";
}
else {
state = "exited";
}
msg = rb_sprintf("pty - %s: %ld", state, (long)pid);
exc = rb_exc_new_str(eChildExited, msg);
rb_iv_set(exc, "status", rb_last_status_get());
rb_exc_raise(exc);
}
static VALUE
pty_check(int argc, VALUE *argv, VALUE self)
{
VALUE pid, exc;
rb_pid_t cpid;
int status;
const int flag =
#if defined(WNOHANG)
WNOHANG|
#endif
#if defined(WUNTRACED)
WUNTRACED|
#endif
0;
rb_scan_args(argc, argv, "11", &pid, &exc);
cpid = rb_waitpid(NUM2PIDT(pid), &status, flag);
if (cpid == -1 || cpid == 0) return Qnil;
if (!RTEST(exc)) return rb_last_status_get();
raise_from_check(cpid, status);
UNREACHABLE_RETURN(Qnil);
}
static VALUE cPTY;
void
Init_pty(void)
{
cPTY = rb_define_module("PTY");
rb_define_module_function(cPTY,"getpty",pty_getpty,-1);
rb_define_module_function(cPTY,"spawn",pty_getpty,-1);
rb_define_singleton_method(cPTY,"check",pty_check,-1);
rb_define_singleton_method(cPTY,"open",pty_open,0);
eChildExited = rb_define_class_under(cPTY,"ChildExited",rb_eRuntimeError);
rb_define_method(eChildExited,"status",echild_status,0);
}
