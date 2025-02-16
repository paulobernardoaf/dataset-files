





















#if !defined(SFLIB_H)
#define SFLIB_H


#include "sfsysnr.h"
#include "sfsyscall.h"
#include "../common/sftypes.h"


static inline _sfsyscall3( ssize_t, read, int, fd, void *, buf, size_t, count )
static inline _sfsyscall3( ssize_t, write, int, fd, const void *, buf, size_t, count )
static inline _sfsyscall3( int, open, const char *, pathname, int, flags, mode_t, mode )
static inline _sfsyscall1( int, close, int, fd )
static inline _sfsyscall2( int, stat, const char *, file_name, struct stat *, buf )
static inline _sfsyscall2( int, fstat, int, filedes, struct stat *, buf )
static inline _sfsyscall2( int, lstat, const char *, file_name, struct stat *, buf )
static inline _sfsyscall3( int, poll, struct pollfd *, ufds, unsigned int, nfds, int, timeout )
static inline _sfsyscall3( off_t, lseek, int, fildes, off_t, offset, int, whence )
static inline _sfsyscall6(void *,mmap, void *,start, size_t,length, int,prot , int,flags, int,fd, off_t,offset)
static inline _sfsyscall3( int, mprotect, const void *, addr, size_t, len, int, prot )
static inline _sfsyscall2( int, munmap, void *, start, size_t, length )
static inline _sfsyscall1( int, brk, void *, end_data_segment )



static inline _sfsyscall4(int, ioctl, int,d, int,request, char *,argp, int,len)


static inline _sfsyscall3( int, readv, int, fd, const struct iovec *, vector, int, count )
static inline _sfsyscall3( int, writev, int, fd, const struct iovec *, vector, int, count )
static inline _sfsyscall2( int, access, const char *, pathname, int, mode )
static inline _sfsyscall1( int, pipe, unsigned long *, filedes)
static inline _sfsyscall5( int, select, int, n, fd_set *, readfds, fd_set *, writefds, fd_set *, exceptfds, struct timeval *, timeout)
static inline _sfsyscall0( int, sched_yield )

static inline _sfsyscall3( int, msync, const void *, start, size_t, length, int, flags )


static inline _sfsyscall3(int, shmget, long, key, int, size, int, flags )
static inline _sfsyscall3(void *, shmat, int, shmid, const void *, shmaddr, int, shmflg )

static inline _sfsyscall1( int, dup, int, oldfd )
static inline _sfsyscall2( int, dup2, int, oldfd, int, newfd )
static inline _sfsyscall0( int, pause )
static inline _sfsyscall2( int, nanosleep, const struct timespec *, req, struct timespec *, rem )

static inline _sfsyscall1( unsigned int, alarm, unsigned int, seconds )

static inline _sfsyscall0( pid_t, getpid )

static inline _sfsyscall3(int, socket, int,domain, int,type, int,protocol)
static inline _sfsyscall3(int,connect, int,sockfd, const struct sockaddr *,serv_addr, socklen_t,addrlen)
static inline _sfsyscall3(int, accept, int,s, struct sockaddr *,addr, socklen_t *,addrlen);
static inline _sfsyscall6(ssize_t, sendto, int, s, const void *, msg, size_t, len, int, flags, const struct sockaddr *, to, socklen_t, tolen)
static inline _sfsyscall6(ssize_t, recvfrom, int, s, void * , buf, size_t, len, int, flags, struct sockaddr * , from, socklen_t * , fromlen)



static inline _sfsyscall3(int, bind, int,sockfd, struct sockaddr *,my_addr, socklen_t,addrlen)
static inline _sfsyscall2(int,listen,int,s, int,backlog)



static inline _sfsyscall5(int, setsockopt, int, s, int, level, int, optname, void *, optval, socklen_t, optlen)


static inline _sfsyscall0( pid_t, fork )
static inline _sfsyscall0( pid_t, vfork )
static inline _sfsyscall3(int, execve, char *, s, char **, argv, char **,envp)
static inline _sfsyscall1(int, exit, int, status)
static inline _sfsyscall4(pid_t, wait4, pid_t, pid, int *, status, int, options, struct rusage *, rusage)
static inline _sfsyscall2( int, kill, pid_t, pid, int, sig )
static inline _sfsyscall1( int, uname, struct utsname *, buf )
static inline _sfsyscall3(int, semget, long, key, int, nsems, int, semflg)
static inline _sfsyscall3(int, semop, int, semid, struct sembuf *, sops, unsigned, nsops)






static inline _sfsyscall3( int, fcntl, int, fd, int, cmd, long, arg )

static inline _sfsyscall1( int, fsync, int, fd )
static inline _sfsyscall1( int, fdatasync, int, fd )
static inline _sfsyscall2( int, truncate, const char *, path, off_t, length )
static inline _sfsyscall2( int, ftruncate, int, fd, off_t, length )
static inline _sfsyscall3( int, getdents, unsigned int, fd, struct dirent *, dirp, unsigned int, count )

static inline _sfsyscall1( int, chdir, const char *, path )

static inline _sfsyscall2( int, rename, const char *, oldpath, const char *, newpath )
static inline _sfsyscall2( int, mkdir, const char *, pathname, mode_t, mode )
static inline _sfsyscall1( int, rmdir, const char *, pathname )
static inline _sfsyscall2( int, creat, const char *, pathname, mode_t, mode )
static inline _sfsyscall2( int, link, const char *, oldpath, const char *, newpath )
static inline _sfsyscall1( int, unlink, const char *, pathname )
static inline _sfsyscall2( int, symlink, const char *, oldpath, const char *, newpath )
static inline _sfsyscall3( int, readlink, const char *, path, char *, buf, size_t, bufsiz )
static inline _sfsyscall2( int, chmod, const char *, path, mode_t, mode )
static inline _sfsyscall2( int, fchmod, int, fildes, mode_t, mode )
static inline _sfsyscall3( int, chown, const char *, path, uid_t, owner, gid_t, group )
static inline _sfsyscall3( int, fchown, int, fd, uid_t, owner, gid_t, group )
static inline _sfsyscall3( int, lchown, const char *, path, uid_t, owner, gid_t, group )
static inline _sfsyscall1( mode_t, umask, mode_t, mask )
static inline _sfsyscall2( int, gettimeofday, struct timeval *, tv, struct timezone *, tz )


static inline _sfsyscall1( int, sysinfo, struct sysinfo *, info )
static inline _sfsyscall1( clock_t, times, struct tms *, buf )
static inline _sfsyscall4(long, ptrace, int, request, pid_t, pid, void *,addr, void *,data)
static inline _sfsyscall0( uid_t, getuid )
static inline _sfsyscall3( int, syslog, int, type, char *, bufp, int, len )
static inline _sfsyscall0( gid_t, getgid )


static inline _sfsyscall0( uid_t, geteuid )
static inline _sfsyscall0( gid_t, getegid )
static inline _sfsyscall2( int, setpgid, pid_t, pid, pid_t, pgid )
static inline _sfsyscall0( pid_t, getppid )
static inline _sfsyscall0( pid_t, getpgrp )
static inline _sfsyscall0( pid_t, setsid )
static inline _sfsyscall2( int, setreuid, uid_t, ruid, uid_t, euid )
static inline _sfsyscall2( int, setregid, gid_t, rgid, gid_t, egid )

static inline _sfsyscall2( int, setgroups, size_t, size, const gid_t *, list )
static inline _sfsyscall3( int, setresuid, uid_t, ruid, uid_t, euid, uid_t, suid )
static inline _sfsyscall3( int, getresuid, uid_t *, ruid, uid_t *, euid, uid_t *, suid )


static inline _sfsyscall1( pid_t, getpgid, pid_t, pid )
static inline _sfsyscall1( int, setfsuid, uid_t, fsuid )
static inline _sfsyscall1( int, setfsgid, uid_t, fsgid )
static inline _sfsyscall1( pid_t, getsid, pid_t, pid )
static inline _sfsyscall2( int, capget, cap_user_header_t, header, cap_user_data_t, data )






static inline _sfsyscall2( int, utime, const char *, filename, struct utimbuf *, buf )
static inline _sfsyscall3( int, mknod, const char *, pathname, mode_t, mode, dev_t, dev )
static inline _sfsyscall1( int, uselib, const char *, library )
static inline _sfsyscall1( int, personality, unsigned long, persona )
static inline _sfsyscall2( int, ustat, dev_t, dev, struct ustat *, ubuf )
static inline _sfsyscall2( int, statfs, const char *, path, struct statfs *, buf )
static inline _sfsyscall2( int, fstatfs, int, fd, struct statfs *, buf )
static inline _sfsyscall3( int, sysfs, int, option, unsigned int, fs_index, char *, buf )
static inline _sfsyscall2( int, getpriority, int, which, int, who )
static inline _sfsyscall3( int, setpriority, int, which, int, who, int, prio )
static inline _sfsyscall2( int, sched_setparam, pid_t, pid, const struct sched_param *, p )
static inline _sfsyscall2( int, sched_getparam, pid_t, pid, struct sched_param *, p )

static inline _sfsyscall1( int, sched_getscheduler, pid_t, pid )
static inline _sfsyscall1( int, sched_get_priority_max, int, policy )
static inline _sfsyscall1( int, sched_get_priority_min, int, policy )
static inline _sfsyscall2( int, sched_rr_get_interval, pid_t, pid, struct timespec *, tp )
static inline _sfsyscall2( int, mlock, const void *, addr, size_t, len )
static inline _sfsyscall2( int, munlock, const void *, addr, size_t, len )
static inline _sfsyscall1( int, mlockall, int, flags )
static inline _sfsyscall0( int, munlockall )
static inline _sfsyscall0( int, vhangup )






static inline _sfsyscall1( int, chroot, const char *, path )
static inline _sfsyscall0( int, sync )
static inline _sfsyscall1( int, acct, const char *, filename )



static inline _sfsyscall2( int, swapon, const char *, path, int, swapflags )
static inline _sfsyscall1( int, swapoff, const char *, path )

static inline _sfsyscall2( int, sethostname, const char *, name, size_t, len )
static inline _sfsyscall2( int, setdomainname, const char *, name, size_t, len )
static inline _sfsyscall1( int, iopl, int, level )
static inline _sfsyscall3( int, ioperm, unsigned long, from, unsigned long, num, int, turn_on )
static inline _sfsyscall2( caddr_t, create_module, const char *, name, size_t, size )
static inline _sfsyscall2( int, init_module, const char *, name, struct module *, image )
static inline _sfsyscall1( int, delete_module, const char *, name )
static inline _sfsyscall1( int, get_kernel_syms, struct kernel_sym *, table )























static inline _sfsyscall1( time_t, time, time_t *, t )





































































































#endif 
