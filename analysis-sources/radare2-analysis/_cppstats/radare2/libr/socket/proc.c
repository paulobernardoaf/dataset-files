#include <r_socket.h>
#include <r_util.h>
#include <signal.h>
#if __UNIX__
#include <sys/wait.h>
#endif
#define BUFFER_SIZE 4096
R_API struct r_socket_proc_t *r_socket_proc_open(char* const argv[]) {
#if __UNIX__ && LIBC_HAVE_FORK
RSocketProc *sp = R_NEW (RSocketProc);
#if defined(O_CLOEXEC)
const int flags = O_CLOEXEC; 
#else
const int flags = 0; 
#endif
if (!sp) {
return NULL;
}
if (pipe (sp->fd0)==-1) {
perror ("pipe");
goto error;
}
if (fcntl (sp->fd0[0], flags) < 0) {
goto error;
}
if (fcntl (sp->fd0[1], flags) < 0) {
goto error;
}
if (pipe (sp->fd1)==-1) {
perror ("pipe");
goto error;
}
if (fcntl (sp->fd1[0], flags) < 0) {
goto error;
}
if (fcntl (sp->fd1[1], flags) < 0) {
goto error;
}
sp->pid = r_sys_fork ();
switch (sp->pid) {
case 0:
close (0);
dup2 (sp->fd0[0], 0);
close (1);
dup2 (sp->fd1[1], 1);
execv (argv[0], argv);
exit (1);
break;
case -1:
perror ("fork");
r_socket_proc_close (sp);
goto error;
}
return sp;
error:
free (sp);
return NULL;
#else
return NULL;
#endif
}
R_API int r_socket_proc_close(struct r_socket_proc_t *sp) {
#if __UNIX__
kill (sp->pid, SIGKILL);
waitpid (sp->pid, NULL, 0); 
close (sp->fd0[0]);
close (sp->fd0[1]);
close (sp->fd1[1]);
#endif
return 0;
}
R_API int r_socket_proc_read (RSocketProc *sp, unsigned char *buf, int len) {
RSocket s;
s.is_ssl = false;
s.fd = sp->fd1[0];
return r_socket_read (&s, buf, len);
}
R_API int r_socket_proc_gets (RSocketProc *sp, char *buf, int size) {
RSocket s;
s.is_ssl = false;
s.fd = sp->fd1[0];
return r_socket_gets (&s, buf, size);
}
R_API int r_socket_proc_write (RSocketProc *sp, void *buf, int len) {
RSocket s;
s.is_ssl = false;
s.fd = sp->fd0[1];
return r_socket_write (&s, buf, len);
}
R_API void r_socket_proc_printf (RSocketProc *sp, const char *fmt, ...) {
RSocket s;
char buf[BUFFER_SIZE];
va_list ap;
s.is_ssl = false;
s.fd = sp->fd0[1];
if (s.fd != R_INVALID_SOCKET) {
va_start (ap, fmt);
vsnprintf (buf, BUFFER_SIZE, fmt, ap);
r_socket_write (&s, buf, strlen(buf));
va_end (ap);
}
}
R_API int r_socket_proc_ready (RSocketProc *sp, int secs, int usecs) {
RSocket s;
s.is_ssl = false;
s.fd = sp->fd1[0];
return r_socket_ready (&s, secs, usecs);
}
