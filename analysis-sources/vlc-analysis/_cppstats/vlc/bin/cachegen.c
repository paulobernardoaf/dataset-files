#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc/vlc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#endif
static void version (void)
{
puts ("LibVLC plugins cache generation version "VERSION);
}
static void usage (const char *path)
{
printf (
"Usage: %s <path>\n"
"Generate the LibVLC plugins cache for the specified plugins directory.\n",
path);
}
int main (int argc, char *argv[])
{
#if defined(HAVE_GETOPT_H)
static const struct option opts[] =
{
{ "help", no_argument, NULL, 'h' },
{ "version", no_argument, NULL, 'V' },
{ NULL, no_argument, NULL, '\0'}
};
int c;
while ((c = getopt_long (argc, argv, "hV", opts, NULL)) != -1)
switch (c)
{
case 'h':
usage (argv[0]);
return 0;
case 'V':
version ();
return 0;
default:
usage (argv[0]);
return 1;
}
#else
int optind = 1;
#endif
for (int i = optind; i < argc; i++)
{
const char *path = argv[i];
if (setenv ("VLC_PLUGIN_PATH", path, 1))
abort ();
const char *vlc_argv[4];
int vlc_argc = 0;
vlc_argv[vlc_argc++] = "--quiet";
vlc_argv[vlc_argc++] = "--reset-plugins-cache";
vlc_argv[vlc_argc++] = "--"; 
vlc_argv[vlc_argc] = NULL;
libvlc_instance_t *vlc = libvlc_new (vlc_argc, vlc_argv);
if (vlc == NULL)
return 1;
libvlc_release(vlc);
}
return 0;
}
