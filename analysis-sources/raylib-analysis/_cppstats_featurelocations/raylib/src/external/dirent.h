




















#if !defined(DIRENT_H)
#define DIRENT_H


#if !defined(DIRENT_MALLOC)
#define DIRENT_MALLOC(sz) malloc(sz)
#endif
#if !defined(DIRENT_FREE)
#define DIRENT_FREE(p) free(p)
#endif






typedef struct DIR DIR;

struct dirent {
char *d_name;
};

#if defined(__cplusplus)
extern "C" {
#endif




DIR *opendir(const char *name);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
void rewinddir(DIR *dir);

#if defined(__cplusplus)
}
#endif

#endif 






















#include <io.h> 
#include <stdlib.h>
#include <string.h>
#include <errno.h>




typedef ptrdiff_t handle_type; 

struct DIR {
handle_type handle; 
struct _finddata_t info;
struct dirent result; 
char *name; 
};

DIR *opendir(const char *name)
{
DIR *dir = 0;

if (name && name[0])
{
size_t base_length = strlen(name);


const char *all = strchr("/\\", name[base_length - 1]) ? "*" : "/*";

if ((dir = (DIR *)DIRENT_MALLOC(sizeof *dir)) != 0 &&
(dir->name = (char *)DIRENT_MALLOC(base_length + strlen(all) + 1)) != 0)
{
strcat(strcpy(dir->name, name), all);

if ((dir->handle = (handle_type) _findfirst(dir->name, &dir->info)) != -1)
{
dir->result.d_name = 0;
}
else 
{
DIRENT_FREE(dir->name);
DIRENT_FREE(dir);
dir = 0;
}
}
else 
{
DIRENT_FREE(dir);
dir = 0;
errno = ENOMEM;
}
}
else errno = EINVAL;

return dir;
}

int closedir(DIR *dir)
{
int result = -1;

if (dir)
{
if (dir->handle != -1) result = _findclose(dir->handle);

DIRENT_FREE(dir->name);
DIRENT_FREE(dir);
}


if (result == -1) errno = EBADF;

return result;
}

struct dirent *readdir(DIR *dir)
{
struct dirent *result = 0;

if (dir && dir->handle != -1)
{
if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1)
{
result = &dir->result;
result->d_name = dir->info.name;
}
}
else errno = EBADF;

return result;
}

void rewinddir(DIR *dir)
{
if (dir && dir->handle != -1)
{
_findclose(dir->handle);
dir->handle = (handle_type) _findfirst(dir->name, &dir->info);
dir->result.d_name = 0;
}
else errno = EBADF;
}
