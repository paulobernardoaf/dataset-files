






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>

#include "../libvlc.h"
#include "config/configuration.h"

#include <unistd.h>
#include <pwd.h>
#include <assert.h>
#include <limits.h>






VLC_WEAK char *config_GetLibDir(void)
{
return strdup(LIBDIR);
}

char *config_GetSysPath(vlc_sysdir_t type, const char *filename)
{
static const char env_vars[][16] = {
[VLC_PKG_DATA_DIR] = "VLC_DATA_PATH",
[VLC_PKG_LIB_DIR] = "VLC_LIB_PATH",
[VLC_PKG_LIBEXEC_DIR] = "VLC_LIB_PATH",
};

if (type < ARRAY_SIZE(env_vars)) {
const char *name = env_vars[type];
if (*name != '\0') {
const char *value = getenv(name);
if (value != NULL) {
const char *fmt = (filename != NULL) ? "%s/%s" : "%s";
char *filepath;

if (unlikely(asprintf(&filepath, fmt, value, filename) == -1))
filepath = NULL;
return filepath;
}
}
}

char *libdir = config_GetLibDir();
if (libdir == NULL)
return NULL; 

static const char *const dirs[] = {
[VLC_PKG_DATA_DIR] = PKGDATADIR,
[VLC_PKG_LIB_DIR] = PKGLIBDIR,
[VLC_PKG_LIBEXEC_DIR] = PKGLIBEXECDIR,
[VLC_SYSDATA_DIR] = SYSDATADIR,
[VLC_LIB_DIR] = LIBDIR,
[VLC_LIBEXEC_DIR] = LIBEXECDIR,
[VLC_LOCALE_DIR] = LOCALEDIR,
};
assert(type < ARRAY_SIZE(dirs));

const char *dir_static = dirs[type];
assert(*dir_static != '\0');

size_t prefix_len = 0;
while (prefix_len < strlen(LIBDIR)
&& LIBDIR[prefix_len] == dir_static[prefix_len])
prefix_len++;


char *filepath = NULL;
size_t suffix_len = strlen(LIBDIR) - prefix_len;
size_t libdir_len = strlen(libdir);

if (suffix_len > libdir_len
|| memcmp(&LIBDIR[prefix_len], libdir + (libdir_len - suffix_len),
suffix_len)) {
suffix_len = libdir_len;
prefix_len = 0;
}


const char *fmt = (filename != NULL) ? "%.*s%s/%s" : "%.*s%s";

if (unlikely(asprintf(&filepath, fmt, (int)(libdir_len - suffix_len),
libdir, dir_static + prefix_len, filename) == -1))
filepath = NULL;

free(libdir);
return filepath;
}

static char *config_GetHomeDir (void)
{

const char *home = getenv ("HOME");
if (home != NULL)
return strdup (home);
#if defined(HAVE_GETPWUID_R)

long max = sysconf (_SC_GETPW_R_SIZE_MAX);
if (max != -1)
{
char buf[max];
struct passwd pwbuf, *pw;

if (getpwuid_r (getuid (), &pwbuf, buf, sizeof (buf), &pw) == 0
&& pw != NULL)
return strdup (pw->pw_dir);
}
#endif
return NULL;
}

static char *config_GetAppDir (const char *xdg_name, const char *xdg_default)
{
char *psz_dir;
char var[sizeof ("XDG__HOME") + strlen (xdg_name)];


snprintf (var, sizeof (var), "XDG_%s_HOME", xdg_name);

const char *home = getenv (var);
if (home != NULL)
{
if (asprintf (&psz_dir, "%s/vlc", home) == -1)
psz_dir = NULL;
return psz_dir;
}

char *psz_home = config_GetHomeDir ();
if( psz_home == NULL
|| asprintf( &psz_dir, "%s/%s/vlc", psz_home, xdg_default ) == -1 )
psz_dir = NULL;
free (psz_home);
return psz_dir;
}

static char *config_GetTypeDir (const char *xdg_name)
{
const size_t namelen = strlen (xdg_name);
const char *home = getenv ("HOME");
const char *dir = getenv ("XDG_CONFIG_HOME");
const char *file = "user-dirs.dirs";

if (home == NULL)
return NULL;
if (dir == NULL)
{
dir = home;
file = ".config/user-dirs.dirs";
}

char *path;
if (asprintf (&path, "%s/%s", dir, file) == -1)
return NULL;

FILE *stream = fopen (path, "rte");
free (path);
path = NULL;
if (stream != NULL)
{
char *linebuf = NULL;
size_t linelen = 0;

while (getline (&linebuf, &linelen, stream) != -1)
{
char *ptr = linebuf;
ptr += strspn (ptr, " \t"); 
if (strncmp (ptr, "XDG_", 4))
continue;
ptr += 4; 
if (strncmp (ptr, xdg_name, namelen))
continue;
ptr += namelen; 
if (strncmp (ptr, "_DIR", 4))
continue;
ptr += 4; 
ptr += strspn (ptr, " \t"); 
if (*ptr != '=')
continue;
ptr++; 
ptr += strspn (ptr, " \t"); 
if (*ptr != '"')
continue;
ptr++; 
linelen -= ptr - linebuf;

char *out;
if (strncmp (ptr, "$HOME", 5))
{
path = malloc (linelen);
if (path == NULL)
continue;
out = path;
}
else
{ 
const size_t homelen = strlen (home);
ptr += 5;
path = malloc (homelen + linelen - 5);
if (path == NULL)
continue;
memcpy (path, home, homelen);
out = path + homelen;
}

while (*ptr != '"')
{
if (*ptr == '\\')
ptr++;
if (*ptr == '\0')
{
free (path);
path = NULL;
break;
}
*(out++) = *(ptr++);
}
if (path != NULL)
*out = '\0';
break;
}
free (linebuf);
fclose (stream);
}


if (path == NULL)
{
if (strcmp (xdg_name, "DESKTOP") == 0)
{
if (asprintf (&path, "%s/Desktop", home) == -1)
return NULL;
}
else
path = strdup (home);
}

return path;
}


char *config_GetUserDir (vlc_userdir_t type)
{
switch (type)
{
case VLC_HOME_DIR:
break;
case VLC_CONFIG_DIR:
return config_GetAppDir ("CONFIG", ".config");
case VLC_USERDATA_DIR:
return config_GetAppDir ("DATA", ".local/share");
case VLC_CACHE_DIR:
return config_GetAppDir ("CACHE", ".cache");

case VLC_DESKTOP_DIR:
return config_GetTypeDir ("DESKTOP");
case VLC_DOWNLOAD_DIR:
return config_GetTypeDir ("DOWNLOAD");
case VLC_TEMPLATES_DIR:
return config_GetTypeDir ("TEMPLATES");
case VLC_PUBLICSHARE_DIR:
return config_GetTypeDir ("PUBLICSHARE");
case VLC_DOCUMENTS_DIR:
return config_GetTypeDir ("DOCUMENTS");
case VLC_MUSIC_DIR:
return config_GetTypeDir ("MUSIC");
case VLC_PICTURES_DIR:
return config_GetTypeDir ("PICTURES");
case VLC_VIDEOS_DIR:
return config_GetTypeDir ("VIDEOS");
}
return config_GetHomeDir ();
}
