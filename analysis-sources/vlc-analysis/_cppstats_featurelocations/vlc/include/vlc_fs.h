



















#if !defined(VLC_FS_H)
#define VLC_FS_H 1

#include <sys/types.h>
#include <dirent.h>

struct stat;
struct iovec;

#if defined(_WIN32)
#include <sys/stat.h>
#if !defined(stat)
#define stat _stati64
#endif
#if !defined(fstat)
#define fstat _fstati64
#endif
#if !defined(_MSC_VER)
#undef lseek
#define lseek _lseeki64
#endif
#endif

#if defined(__ANDROID__)
#define lseek lseek64
#endif







































VLC_API int vlc_open(const char *filename, int flags, ...) VLC_USED;











VLC_API int vlc_openat(int fd, const char *filename, int flags, ...) VLC_USED;

VLC_API int vlc_mkstemp( char * );






VLC_API int vlc_dup(int) VLC_USED;






VLC_API int vlc_pipe(int [2]) VLC_USED;












VLC_API int vlc_memfd(void) VLC_USED;







VLC_API ssize_t vlc_write(int, const void *, size_t);





VLC_API ssize_t vlc_writev(int, const struct iovec *, int);





















VLC_API int vlc_close(int fd);











VLC_API int vlc_stat(const char *filename, struct stat *) VLC_USED;







VLC_API int vlc_lstat(const char *filename, struct stat *) VLC_USED;








VLC_API int vlc_unlink(const char *filename);









VLC_API int vlc_rename(const char *oldpath, const char *newpath);

VLC_API FILE * vlc_fopen( const char *filename, const char *mode ) VLC_USED;













VLC_API DIR *vlc_opendir(const char *dirname) VLC_USED;












VLC_API const char *vlc_readdir(DIR *dir) VLC_USED;

VLC_API int vlc_loaddir( DIR *dir, char ***namelist, int (*select)( const char * ), int (*compar)( const char **, const char ** ) );
VLC_API int vlc_scandir( const char *dirname, char ***namelist, int (*select)( const char * ), int (*compar)( const char **, const char ** ) );









VLC_API int vlc_mkdir(const char *dirname, mode_t mode);







VLC_API char *vlc_getcwd(void) VLC_USED;



#if defined( _WIN32 )
typedef struct vlc_DIR
{
_WDIR *wdir; 
char *entry;
union
{
DWORD drives;
bool insert_dot_dot;
} u;
} vlc_DIR;

static inline int vlc_closedir( DIR *dir )
{
vlc_DIR *vdir = (vlc_DIR *)dir;
_WDIR *wdir = vdir->wdir;

free( vdir->entry );
free( vdir );
return (wdir != NULL) ? _wclosedir( wdir ) : 0;
}
#undef closedir
#define closedir vlc_closedir

static inline void vlc_rewinddir( DIR *dir )
{
_WDIR *wdir = *(_WDIR **)dir;

_wrewinddir( wdir );
}
#undef rewinddir
#define rewinddir vlc_rewinddir
#endif

#if defined(__ANDROID__)
#define lseek lseek64
#endif

#endif
