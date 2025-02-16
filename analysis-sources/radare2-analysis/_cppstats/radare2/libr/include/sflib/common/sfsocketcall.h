int socketcall(int call, unsigned long *args);
#define SYS_socket 1 
#define SYS_bind 2 
#define SYS_connect 3 
#define SYS_listen 4 
#define SYS_accept 5 
#define SYS_getsockname 6 
#define SYS_getpeername 7 
#define SYS_socketpair 8 
#define SYS_send 9 
#define SYS_recv 10 
#define SYS_sendto 11 
#define SYS_recvfrom 12 
#define SYS_shutdown 13 
#define SYS_setsockopt 14 
#define SYS_getsockopt 15 
#define SYS_sendmsg 16 
#define SYS_recvmsg 17 
#define __sys_socketcall0(type, name) type name(void) { return socketcall(SYS_##name, 0); }
#define __sys_socketcall1(type, name, type0, arg0) type name(type0 arg0) { unsigned long arr[1]; arr[0] = (long)arg0; return socketcall(SYS_##name, arr); }
#define __sys_socketcall2(type, name, type0,arg0, type1,arg1) type name(type0 arg0, type1 arg1) { unsigned long arr[2]; arr[0] = (long)arg0; arr[1] = (long)arg1; return socketcall(SYS_##name, arr); }
#define __sys_socketcall3(type, name, type0,arg0, type1,arg1, type2,arg2) type name(type0 arg0, type1 arg1, type2 arg2) { unsigned long arr[3]; arr[0] = (long)arg0; arr[1] = (long)arg1; arr[2] = (long)arg2; return socketcall(SYS_##name, arr); }
#define __sys_socketcall4(type, name, type0,arg0, type1,arg1, type2,arg2, type3,arg3) type name(type0 arg0, type1 arg1, type2 arg2, type3 arg3) { unsigned long arr[4]; arr[0] = (long)arg0; arr[1] = (long)arg1; arr[2] = (long)arg2; arr[3] = (long)arg3; return socketcall(SYS_##name, arr); }
#define __sys_socketcall5(type, name, type0,arg0, type1,arg1, type2,arg2, type3,arg3, type4,arg4) type name(type0 arg0, type1 arg1, type2 arg2, type3 arg3, type4 arg4) { unsigned long arr[5]; arr[0] = (long)arg0; arr[1] = (long)arg1; arr[2] = (long)arg2; arr[3] = (long)arg3; arr[4] = (long)arg4; return socketcall(SYS_##name, arr); }
#define __sys_socketcall6(type, name, type0,arg0, type1,arg1, type2,arg2, type3,arg3, type4,arg4, type5,arg5) type name(type0 arg0, type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) { unsigned long arr[6]; arr[0] = (long)arg0; arr[1] = (long)arg1; arr[2] = (long)arg2; arr[3] = (long)arg3; arr[4] = (long)arg4; arr[5] = (long)arg5; return socketcall(SYS_##name, arr); }
inline static __sys_socketcall3(int,socket, int,domain, int,type, int,protocol)
inline static __sys_socketcall3(int,bind, int,sockfd, struct sockaddr *,my_addr, socklen_t,addrlen)
inline static __sys_socketcall3(int,connect, int,sockfd, const struct sockaddr *,serv_addr, socklen_t,addrlen)
inline static __sys_socketcall2(int,listen, int,s, int,backlog)
inline static __sys_socketcall3(int,accept, int,s, struct sockaddr *,addr, socklen_t*,addrlen)
inline static __sys_socketcall4(int,send, int,s, const void *,msg, size_t,len, int,flags)
inline static __sys_socketcall4(ssize_t,recv, int,s, void *,buf, size_t,len, int,flags)
inline static __sys_socketcall6(ssize_t,recvfrom, int,s, void *,buf, size_t,len, int,flags, struct sockaddr *,from, socklen_t *,fromlen)
inline static __sys_socketcall3(ssize_t,recvmsg, int,s, struct msghdr *,msg, int,flags)
inline static __sys_socketcall6(ssize_t,sendto, int,s, const void *,buf, size_t,len, int,flags, const struct sockaddr *,to, socklen_t,tolen)
inline static __sys_socketcall3(ssize_t,sendmsg, int,s, const struct msghdr *,msg, int,flags)
