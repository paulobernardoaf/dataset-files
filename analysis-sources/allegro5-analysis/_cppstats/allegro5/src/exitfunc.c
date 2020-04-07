#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_exitfunc.h"
struct al_exit_func {
void (*funcptr)(void);
const char *desc;
struct al_exit_func *next;
};
static struct al_exit_func *exit_func_list = NULL;
void _al_add_exit_func(void (*func)(void), const char *desc)
{
struct al_exit_func *n;
for (n = exit_func_list; n; n = n->next)
if (n->funcptr == func)
return;
n = al_malloc(sizeof(struct al_exit_func));
if (!n)
return;
n->next = exit_func_list;
n->funcptr = func;
n->desc = desc;
exit_func_list = n;
}
void _al_remove_exit_func(void (*func)(void))
{
struct al_exit_func *iter = exit_func_list, *prev = NULL;
while (iter) {
if (iter->funcptr == func) {
if (prev)
prev->next = iter->next;
else
exit_func_list = iter->next;
al_free(iter);
return;
}
prev = iter;
iter = iter->next;
}
}
void _al_run_exit_funcs(void)
{
while (exit_func_list) {
void (*func)(void) = exit_func_list->funcptr;
_al_remove_exit_func(func);
(*func)();
}
}
