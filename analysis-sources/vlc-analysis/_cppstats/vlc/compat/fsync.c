#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#if defined(_WIN32)
#include <io.h>
#endif
int fsync (int fd)
{
#if defined(_WIN32)
return _commit (fd);
#else
#warning fsync() not implemented!
return 0;
#endif
}
