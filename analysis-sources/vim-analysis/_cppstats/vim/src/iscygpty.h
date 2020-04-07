#if defined(_WIN32)
int is_cygpty(int fd);
int is_cygpty_used(void);
#else
#define is_cygpty(fd) 0
#define is_cygpty_used() 0
#endif
