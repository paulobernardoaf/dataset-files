#if !defined(R_SANDBOX_H)
#define R_SANDBOX_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__FreeBSD__)
#include <sys/param.h>

#if __FreeBSD_version >= 1000000
#define HAVE_CAPSICUM 1
#else
#define HAVE_CAPSICUM 0
#endif
#else
#define HAVE_CAPSICUM 0
#endif






#if __WINDOWS__
R_API HANDLE r_sandbox_opendir(const char *path, WIN32_FIND_DATAW *entry);
#else
R_API DIR* r_sandbox_opendir(const char *path);
#endif
R_API int r_sandbox_truncate(int fd, ut64 length);
R_API int r_sandbox_lseek(int fd, ut64 addr, int mode);
R_API int r_sandbox_close(int fd);
R_API int r_sandbox_read(int fd, ut8 *buf, int len);
R_API int r_sandbox_write(int fd, const ut8 *buf, int len);
R_API bool r_sandbox_enable(bool e);
R_API bool r_sandbox_disable(bool e);
R_API int r_sandbox_system(const char *x, int fork);
R_API bool r_sandbox_creat(const char *path, int mode);
R_API int r_sandbox_open(const char *path, int mode, int perm);
R_API FILE *r_sandbox_fopen(const char *path, const char *mode);
R_API int r_sandbox_chdir(const char *path);
R_API bool r_sandbox_check_path(const char *path);
R_API int r_sandbox_kill(int pid, int sig);

#if defined(__cplusplus)
}
#endif

#endif 
