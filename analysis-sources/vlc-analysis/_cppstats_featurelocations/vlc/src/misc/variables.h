





















#if !defined(LIBVLC_VARIABLES_H)
#define LIBVLC_VARIABLES_H 1

#include <vlc_list.h>

struct vlc_res;




typedef struct vlc_object_internals vlc_object_internals_t;

struct vlc_object_internals
{
vlc_object_t *parent; 
const char *typename; 


void *var_root;
vlc_mutex_t var_lock;
vlc_cond_t var_wait;


struct vlc_res *resources;
};

#define vlc_internals(o) ((o)->priv)
#define vlc_externals(priv) (abort(), (void *)(priv))

extern void var_DestroyAll( vlc_object_t * );










char **var_GetAllNames(vlc_object_t *);

#endif
