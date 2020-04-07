





















#if !defined(SFSYSNR_H)
#define SFSYSNR_H











#define __NR_syscall 0


#define __NR_exit 1


#define __NR_fork 2


#define __NR_read 3


#define __NR_write 4


#define __NR_open 5


#define __NR_close 6


#define __NR_wait4 7




#define __NR_link 9


#define __NR_unlink 10



#define __NR_chdir 12


#define __NR_fchdir 13


#define __NR_mknod 14


#define __NR_chmod 15


#define __NR_chown 16


#define __NR_break 17






#define __NR_getpid 20


#define __NR_mount 21


#define __NR_unmount 22


#define __NR_setuid 23


#define __NR_getuid 24


#define __NR_geteuid 25


#define __NR_ptrace 26


#define __NR_recvmsg 27


#define __NR_sendmsg 28


#define __NR_recvfrom 29


#define __NR_accept 30


#define __NR_getpeername 31


#define __NR_getsockname 32


#define __NR_access 33


#define __NR_chflags 34


#define __NR_fchflags 35


#define __NR_sync 36


#define __NR_kill 37




#define __NR_getppid 39




#define __NR_dup 41


#define __NR_opipe 42


#define __NR_getegid 43


#define __NR_profil 44


#define __NR_ktrace 45


#define __NR_sigaction 46


#define __NR_getgid 47


#define __NR_sigprocmask 48


#define __NR_getlogin 49


#define __NR_setlogin 50


#define __NR_acct 51


#define __NR_sigpending 52


#define __NR_sigaltstack 53


#define __NR_ioctl 54


#define __NR_reboot 55


#define __NR_revoke 56


#define __NR_symlink 57


#define __NR_readlink 58


#define __NR_execve 59


#define __NR_umask 60


#define __NR_chroot 61








#define __NR_omsync 65


#define __NR_vfork 66




#define __NR_sbrk 69


#define __NR_sstk 70




#define __NR_vadvise 72


#define __NR_munmap 73


#define __NR_mprotect 74


#define __NR_madvise 75




#define __NR_mincore 78


#define __NR_getgroups 79


#define __NR_setgroups 80


#define __NR_getpgrp 81


#define __NR_setpgid 82


#define __NR_setitimer 83




#define __NR_swapon 85


#define __NR_getitimer 86








#define __NR_dup2 90


#define __NR_fcntl 92


#define __NR_select 93


#define __NR_fsync 95


#define __NR_setpriority 96


#define __NR_socket 97


#define __NR_connect 98




#define __NR_getpriority 100






#define __NR_sigreturn 103


#define __NR_bind 104


#define __NR_setsockopt 105


#define __NR_listen 106









#define __NR_sigsuspend 111









#define __NR_gettimeofday 116


#define __NR_getrusage 117


#define __NR_getsockopt 118



#define __NR_readv 120


#define __NR_writev 121


#define __NR_settimeofday 122


#define __NR_fchown 123


#define __NR_fchmod 124








#define __NR_rename 128






#define __NR_flock 131


#define __NR_mkfifo 132


#define __NR_sendto 133


#define __NR_shutdown 134


#define __NR_socketpair 135


#define __NR_mkdir 136


#define __NR_rmdir 137


#define __NR_utimes 138



#define __NR_adjtime 140














#define __NR_setsid 147


#define __NR_quotactl 148






#define __NR_nfssvc 155








#define __NR_getfh 161








#define __NR_sysarch 165








#define __NR_pread 173


#define __NR_pwrite 174


#define __NR_setgid 181


#define __NR_setegid 182


#define __NR_seteuid 183


#define __NR_lfs_bmapv 184


#define __NR_lfs_markv 185


#define __NR_lfs_segclean 186


#define __NR_lfs_segwait 187


#define __NR_stat 188


#define __NR_fstat 189


#define __NR_lstat 190


#define __NR_pathconf 191


#define __NR_fpathconf 192


#define __NR_swapctl 193


#define __NR_getrlimit 194


#define __NR_setrlimit 195


#define __NR_getdirentries 196


#define __NR_mmap 197


#define __NR___syscall 198


#define __NR_lseek 199


#define __NR_truncate 200


#define __NR_ftruncate 201


#define __NR___sysctl 202


#define __NR_mlock 203


#define __NR_munlock 204


#define __NR_undelete 205


#define __NR_futimes 206


#define __NR_getpgid 207


#define __NR_xfspioctl 208




#define __NR_semget 221


#define __NR_semop 222





#define __NR_msgget 225


#define __NR_msgsnd 226


#define __NR_msgrcv 227


#define __NR_shmat 228




#define __NR_shmdt 230


#define __NR_shmget 231


#define __NR_clock_gettime 232


#define __NR_clock_settime 233


#define __NR_clock_getres 234


#define __NR_nanosleep 240


#define __NR_minherit 250


#define __NR_rfork 251


#define __NR_poll 252


#define __NR_issetugid 253


#define __NR_lchown 254


#define __NR_getsid 255


#define __NR_msync 256


#define __NR___semctl 257


#define __NR_shmctl 258


#define __NR_msgctl 259


#define __NR_getfsstat 260


#define __NR_statfs 261


#define __NR_fstatfs 262


#define __NR_pipe 263


#define __NR_fhopen 264


#define __NR_fhstat 265


#define __NR_fhstatfs 266


#define __NR_preadv 267


#define __NR_pwritev 268


#define __NR_kqueue 269


#define __NR_kevent 270


#define __NR_mlockall 271


#define __NR_munlockall 272


#define __NR_getpeereid 273


#define __NR_extattrctl 274


#define __NR_extattr_set_file 275


#define __NR_extattr_get_file 276


#define __NR_extattr_delete_file 277


#define __NR_extattr_set_fd 278


#define __NR_extattr_get_fd 279


#define __NR_extattr_delete_fd 280

#define __NR_MAXSYSCALL 281


#endif 
