



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <spawn.h>
#include <unistd.h>

extern char **environ;

#include <vlc_common.h>
#include <vlc_fs.h>
#include <vlc_network.h>






char *vlc_getProxyUrl(const char *url)
{

pid_t pid;
posix_spawn_file_actions_t actions;
posix_spawnattr_t attr;
char *argv[3] = { (char *)"proxy", (char *)url, NULL };
int fd[2];

if (vlc_pipe(fd))
return NULL;

if (posix_spawn_file_actions_init(&actions))
return NULL;
if (posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null",
O_RDONLY, 0644) ||
posix_spawn_file_actions_adddup2(&actions, fd[1], STDOUT_FILENO))
{
posix_spawn_file_actions_destroy(&actions);
return NULL;
}

posix_spawnattr_init(&attr);
{
sigset_t set;

sigemptyset(&set);
posix_spawnattr_setsigmask(&attr, &set);
sigaddset (&set, SIGPIPE);
posix_spawnattr_setsigdefault(&attr, &set);
posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGDEF
| POSIX_SPAWN_SETSIGMASK);
}

if (posix_spawnp(&pid, "proxy", &actions, &attr, argv, environ))
pid = -1;

posix_spawnattr_destroy(&attr);
posix_spawn_file_actions_destroy(&actions);
vlc_close(fd[1]);

if (pid != -1)
{
char buf[1024];
size_t len = 0;

do
{
ssize_t val = read(fd[0], buf + len, sizeof (buf) - len);
if (val <= 0)
break;
len += val;
}
while (len < sizeof (buf));

vlc_close(fd[0]);
while (waitpid(pid, &(int){ 0 }, 0) == -1);

if (len >= 9 && !strncasecmp(buf, "direct://", 9))
return NULL;

char *end = memchr(buf, '\n', len);
if (end != NULL)
{
*end = '\0';
return strdup(buf);
}

}
else
vlc_close(fd[0]);


char *var = getenv("http_proxy");
if (var != NULL)
var = strdup(var);
return var;
}
