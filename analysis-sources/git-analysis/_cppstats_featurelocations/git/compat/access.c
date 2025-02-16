#define COMPAT_CODE_ACCESS
#include "../git-compat-util.h"





int git_access(const char *path, int mode)
{
struct stat st;


if (geteuid())
return access(path, mode);

if (stat(path, &st) < 0)
return -1;


if (!(mode & X_OK))
return 0;




if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
return 0;

errno = EACCES;
return -1;
}
