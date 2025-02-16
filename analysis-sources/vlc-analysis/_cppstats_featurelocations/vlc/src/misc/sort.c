




















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <vlc_common.h>
#include <vlc_sort.h>

static thread_local struct
{
int (*compar)(const void *, const void *, void *);
void *arg;
} state;

static int compar_wrapper(const void *a, const void *b)
{
return state.compar(a, b, state.arg);
}




VLC_WEAK void vlc_qsort(void *base, size_t nmemb, size_t size,
int (*compar)(const void *, const void *, void *),
void *arg)
{
int (*saved_compar)(const void *, const void *, void *) = state.compar;
void *saved_arg = state.arg;

state.compar = compar;
state.arg = arg;

qsort(base, nmemb, size, compar_wrapper);


state.compar = saved_compar;
state.arg = saved_arg;
}
