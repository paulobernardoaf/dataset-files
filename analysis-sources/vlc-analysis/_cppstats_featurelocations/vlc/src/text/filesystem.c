

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_fs.h>
#include <vlc_sort.h>

#include <assert.h>

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>







FILE *vlc_fopen (const char *filename, const char *mode)
{
int rwflags = 0, oflags = 0;

for (const char *ptr = mode; *ptr; ptr++)
{
switch (*ptr)
{
case 'r':
rwflags = O_RDONLY;
break;

case 'a':
rwflags = O_WRONLY;
oflags |= O_CREAT | O_APPEND;
break;

case 'w':
rwflags = O_WRONLY;
oflags |= O_CREAT | O_TRUNC;
break;

case 'x':
oflags |= O_EXCL;
break;

case '+':
rwflags = O_RDWR;
break;

#if defined(O_BINARY)
case 'b':
oflags = (oflags & ~O_TEXT) | O_BINARY;
break;

case 't':
oflags = (oflags & ~O_BINARY) | O_TEXT;
break;
#endif
}
}

int fd = vlc_open (filename, rwflags | oflags, 0666);
if (fd == -1)
return NULL;

FILE *stream = fdopen (fd, mode);
if (stream == NULL)
vlc_close (fd);

return stream;
}


static int dummy_select( const char *str )
{
(void)str;
return 1;
}

static int compar_void(const void *a, const void *b, void *data)
{
const char *sa = a, *sb = b;
int (*cmp)(const char **, const char **) = data;

return cmp(&sa, &sb);
}





int vlc_loaddir( DIR *dir, char ***namelist,
int (*select)( const char * ),
int (*compar)( const char **, const char ** ) )
{
assert (dir);

if (select == NULL)
select = dummy_select;

char **tab = NULL;
unsigned num = 0;

rewinddir (dir);

for (unsigned size = 0;;)
{
errno = 0;
const char *entry = vlc_readdir (dir);
if (entry == NULL)
{
if (errno)
goto error;
break;
}

if (!select (entry))
continue;

if (num >= size)
{
size = size ? (2 * size) : 16;
char **newtab = realloc (tab, sizeof (*tab) * (size));

if (unlikely(newtab == NULL))
goto error;
tab = newtab;
}

tab[num] = strdup(entry);
if (likely(tab[num] != NULL))
num++;
}

if (compar != NULL && num > 0)
vlc_qsort(tab, num, sizeof (*tab), compar_void, compar);
*namelist = tab;
return num;

error:
for (unsigned i = 0; i < num; i++)
free (tab[i]);
free (tab);
return -1;
}












int vlc_scandir( const char *dirname, char ***namelist,
int (*select)( const char * ),
int (*compar)( const char **, const char ** ) )
{
DIR *dir = vlc_opendir (dirname);
int val = -1;

if (dir != NULL)
{
val = vlc_loaddir (dir, namelist, select, compar);
closedir (dir);
}
return val;
}

#include <vlc_rand.h>

VLC_WEAK int vlc_mkstemp(char *template)
{
static const char bytes[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqstruvwxyz_-";
static const size_t nbytes = ARRAY_SIZE(bytes) - 1;
char *pattern;

static_assert(((ARRAY_SIZE(bytes) - 1) & (ARRAY_SIZE(bytes) - 2)) == 0,
"statistical bias");


assert(template != NULL);

const size_t len = strlen(template);
if (len < 6
|| strcmp(pattern = template + len - 6, "XXXXXX")) {
errno = EINVAL;
return -1;
}


for( int i = 0; i < 256; i++ )
{

uint8_t pi_rand[6];

vlc_rand_bytes( pi_rand, sizeof(pi_rand) );
for( int j = 0; j < 6; j++ )
pattern[j] = bytes[pi_rand[j] % nbytes];


int fd = vlc_open( template, O_CREAT | O_EXCL | O_RDWR, 0600 );
if( fd >= 0 )
return fd;
if( errno != EEXIST )
return -1;
}

errno = EEXIST;
return -1;
}
