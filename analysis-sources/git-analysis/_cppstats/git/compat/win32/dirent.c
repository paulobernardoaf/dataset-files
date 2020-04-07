#include "../../git-compat-util.h"
struct DIR {
struct dirent dd_dir; 
HANDLE dd_handle; 
int dd_stat; 
};
static inline void finddata2dirent(struct dirent *ent, WIN32_FIND_DATAW *fdata)
{
xwcstoutf(ent->d_name, fdata->cFileName, sizeof(ent->d_name));
if (fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
ent->d_type = DT_DIR;
else
ent->d_type = DT_REG;
}
DIR *opendir(const char *name)
{
wchar_t pattern[MAX_PATH + 2]; 
WIN32_FIND_DATAW fdata;
HANDLE h;
int len;
DIR *dir;
if ((len = xutftowcs_path(pattern, name)) < 0)
return NULL;
if (len && !is_dir_sep(pattern[len - 1]))
pattern[len++] = '/';
pattern[len++] = '*';
pattern[len] = 0;
h = FindFirstFileW(pattern, &fdata);
if (h == INVALID_HANDLE_VALUE) {
DWORD err = GetLastError();
errno = (err == ERROR_DIRECTORY) ? ENOTDIR : err_win_to_posix(err);
return NULL;
}
dir = xmalloc(sizeof(DIR));
dir->dd_handle = h;
dir->dd_stat = 0;
finddata2dirent(&dir->dd_dir, &fdata);
return dir;
}
struct dirent *readdir(DIR *dir)
{
if (!dir) {
errno = EBADF; 
return NULL;
}
if (dir->dd_stat) {
WIN32_FIND_DATAW fdata;
if (FindNextFileW(dir->dd_handle, &fdata)) {
finddata2dirent(&dir->dd_dir, &fdata);
} else {
DWORD lasterr = GetLastError();
if (lasterr != ERROR_NO_MORE_FILES)
errno = err_win_to_posix(lasterr);
return NULL;
}
}
++dir->dd_stat;
return &dir->dd_dir;
}
int closedir(DIR *dir)
{
if (!dir) {
errno = EBADF;
return -1;
}
FindClose(dir->dd_handle);
free(dir);
return 0;
}
