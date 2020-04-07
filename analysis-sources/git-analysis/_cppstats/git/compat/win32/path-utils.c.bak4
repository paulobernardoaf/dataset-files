#include "../../git-compat-util.h"

int win32_has_dos_drive_prefix(const char *path)
{
int i;





if (!(0x80 & (unsigned char)*path))
return *path && path[1] == ':' ? 2 : 0;









for (i = 1; i < 4 && (0x80 & (unsigned char)path[i]); i++)
; 
return path[i] == ':' ? i + 1 : 0;
}

int win32_skip_dos_drive_prefix(char **path)
{
int ret = has_dos_drive_prefix(*path);
*path += ret;
return ret;
}

int win32_offset_1st_component(const char *path)
{
char *pos = (char *)path;


if (!skip_dos_drive_prefix(&pos) &&
is_dir_sep(pos[0]) && is_dir_sep(pos[1])) {

pos = strpbrk(pos + 2, "\\/");
if (!pos)
return 0; 

do {
pos++;
} while (*pos && !is_dir_sep(*pos));
}

return pos + is_dir_sep(*pos) - path;
}
