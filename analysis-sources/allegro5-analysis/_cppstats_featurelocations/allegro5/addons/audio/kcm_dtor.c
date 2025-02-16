

















#include "allegro5/allegro.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/internal/aintern_dtor.h"
#include "allegro5/internal/aintern_thread.h"
#include "allegro5/internal/aintern_audio.h"


static _AL_DTOR_LIST *kcm_dtors = NULL;





void _al_kcm_init_destructors(void)
{
if (!kcm_dtors) {
kcm_dtors = _al_init_destructors();
}
}





void _al_kcm_shutdown_destructors(void)
{
if (kcm_dtors) {
_al_run_destructors(kcm_dtors);
_al_shutdown_destructors(kcm_dtors);
kcm_dtors = NULL;
}
}





_AL_LIST_ITEM *_al_kcm_register_destructor(char const *name, void *object,
void (*func)(void*))
{
return _al_register_destructor(kcm_dtors, name, object, func);
}





void _al_kcm_unregister_destructor(_AL_LIST_ITEM *dtor_item)
{
_al_unregister_destructor(kcm_dtors, dtor_item);
}





void _al_kcm_foreach_destructor(
void (*callback)(void *object, void (*func)(void *), void *udata),
void *userdata)
{
_al_foreach_destructor(kcm_dtors, callback, userdata);
}



