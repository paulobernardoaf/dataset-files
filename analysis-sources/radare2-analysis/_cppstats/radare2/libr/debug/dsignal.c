#include <r_debug.h>
#define DB dbg->sgnls
static struct {
const char *k;
const char *v;
} signals[] = {
{ "SIGHUP", "1" },
{ "SIGINT", "2" },
{ "SIGQUIT", "3" },
{ "SIGILL", "4" },
{ "SIGTRAP", "5" },
{ "SIGABRT", "6" },
{ "SIGBUS", "7" },
{ "SIGFPE", "8" },
{ "SIGKILL", "9" },
{ "SIGUSR1", "10" },
{ "SIGSEGV", "11" },
{ "SIGUSR2", "12" },
{ "SIGPIPE", "13" },
{ "SIGALRM", "14" },
{ "SIGTERM", "15" },
{ "SIGSTKFLT", "16" },
{ "SIGCHLD", "17" },
{ "SIGCONT", "18" },
{ "SIGSTOP", "19" },
{ "SIGTSTP", "20" },
{ "SIGTTIN", "21" },
{ "SIGTTOU", "22" },
{ "SIGURG", "23" },
{ "SIGXCPU", "24" },
{ "SIGXFSZ", "25" },
{ "SIGVTALRM", "26" },
{ "SIGPROF", "27" },
{ "SIGWINCH", "28" },
{ "SIGIO", "29" },
{ "SIGPOLL", "SIGIO" },
{ "SIGLOST", "29" },
{ "SIGPWR", "30" },
{ "SIGSYS", "31" },
{ "SIGRTMIN", "32" },
{ "SIGRTMAX", "NSIG" },
{ NULL }
};
R_API void r_debug_signal_init(RDebug *dbg) {
int i;
DB = sdb_new (NULL, "signals", 0);
for (i=0; signals[i].k; i++) {
sdb_set (DB, signals[i].k, signals[i].v, 0);
sdb_set (DB, signals[i].v, signals[i].k, 0);
}
}
static int siglistcb (void *p, const char *k, const char *v) {
static char key[32] = "cfg.";
RDebug *dbg = (RDebug *)p;
int opt, mode = dbg->_mode;
if (atoi (k) > 0) {
strncpy (key + 4, k, 20);
opt = sdb_num_get (DB, key, 0);
if (opt) {
r_cons_printf ("%s %s", k, v);
if (opt & R_DBG_SIGNAL_CONT) {
r_cons_strcat (" cont");
}
if (opt & R_DBG_SIGNAL_SKIP) {
r_cons_strcat (" skip");
}
r_cons_newline ();
} else {
if (mode == 0) {
r_cons_printf ("%s %s\n", k, v);
}
}
}
return 1;
}
static int siglistjsoncb (void *p, const char *k, const char *v) {
static char key[32] = "cfg.";
RDebug *dbg = (RDebug *)p;
int opt;
if (atoi (k)>0) {
strncpy (key + 4, k, 20);
opt = (int)sdb_num_get (DB, key, 0);
if (dbg->_mode == 2) {
dbg->_mode = 0;
} else {
r_cons_strcat (",");
}
r_cons_printf ("{\"signum\":\"%s\",\"name\":\"%s\",\"option\":", k, v);
if (opt & R_DBG_SIGNAL_CONT) {
r_cons_strcat ("\"cont\"");
} else if (opt & R_DBG_SIGNAL_SKIP) {
r_cons_strcat ("\"skip\"");
} else {
r_cons_strcat ("null");
}
r_cons_strcat ("}");
}
return true;
}
R_API void r_debug_signal_list(RDebug *dbg, int mode) {
dbg->_mode = mode;
switch (mode) {
case 0:
case 1:
sdb_foreach (DB, siglistcb, dbg);
break;
case 2:
r_cons_strcat ("[");
sdb_foreach (DB, siglistjsoncb, dbg);
r_cons_strcat ("]");
r_cons_newline();
break;
}
dbg->_mode = 0;
}
R_API int r_debug_signal_send(RDebug *dbg, int num) {
return r_sandbox_kill (dbg->pid, num);
}
R_API void r_debug_signal_setup(RDebug *dbg, int num, int opt) {
sdb_queryf (DB, "cfg.%d=%d", num, opt);
}
R_API int r_debug_signal_what(RDebug *dbg, int num) {
char k[32];
snprintf (k, sizeof (k), "cfg.%d", num);
return sdb_num_get (DB, k, 0);
}
R_API int r_debug_signal_set(RDebug *dbg, int num, ut64 addr) {
return 0;
}
R_API RList *r_debug_kill_list(RDebug *dbg) {
if (dbg->h->kill_list) {
return dbg->h->kill_list (dbg);
}
return NULL;
}
R_API int r_debug_kill_setup(RDebug *dbg, int sig, int action) {
eprintf ("TODO: set signal handlers of child\n");
#if 0
if (dbg->h->kill_setup)
return dbg->h->kill_setup (dbg, sig, action);
#endif
return false;
}
