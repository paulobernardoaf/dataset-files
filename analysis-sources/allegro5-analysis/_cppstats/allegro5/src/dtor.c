#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_dtor.h"
#include "allegro5/internal/aintern_thread.h"
#include "allegro5/internal/aintern_tls.h"
#include "allegro5/internal/aintern_list.h"
ALLEGRO_DEBUG_CHANNEL("dtor")
struct _AL_DTOR_LIST {
_AL_MUTEX mutex;
_AL_LIST* dtors;
};
typedef struct DTOR {
char const *name;
void *object;
void (*func)(void*);
} DTOR;
_AL_DTOR_LIST *_al_init_destructors(void)
{
_AL_DTOR_LIST *dtors = al_malloc(sizeof(*dtors));
_AL_MARK_MUTEX_UNINITED(dtors->mutex);
_al_mutex_init(&dtors->mutex);
dtors->dtors = _al_list_create();
return dtors;
}
void _al_push_destructor_owner(void)
{
int *dtor_owner_count = _al_tls_get_dtor_owner_count();
(*dtor_owner_count)++;
}
void _al_pop_destructor_owner(void)
{
int *dtor_owner_count = _al_tls_get_dtor_owner_count();
(*dtor_owner_count)--;
ASSERT(*dtor_owner_count >= 0);
}
void _al_run_destructors(_AL_DTOR_LIST *dtors)
{
if (!dtors) {
return;
}
_al_mutex_lock(&dtors->mutex);
{
_AL_LIST_ITEM *iter = _al_list_back(dtors->dtors);
while (iter) {
DTOR *dtor = _al_list_item_data(iter);
void *object = dtor->object;
void (*func)(void *) = dtor->func;
ALLEGRO_DEBUG("calling dtor for %s %p, func %p\n",
dtor->name, object, func);
_al_mutex_unlock(&dtors->mutex);
{
(*func)(object);
}
_al_mutex_lock(&dtors->mutex);
iter = _al_list_back(dtors->dtors);
}
}
_al_mutex_unlock(&dtors->mutex);
}
void _al_shutdown_destructors(_AL_DTOR_LIST *dtors)
{
if (!dtors) {
return;
}
ASSERT(_al_list_is_empty(dtors->dtors));
_al_list_destroy(dtors->dtors);
_al_mutex_destroy(&dtors->mutex);
al_free(dtors);
}
_AL_LIST_ITEM *_al_register_destructor(_AL_DTOR_LIST *dtors, char const *name,
void *object, void (*func)(void*))
{
int *dtor_owner_count;
_AL_LIST_ITEM *ret = NULL;
ASSERT(object);
ASSERT(func);
dtor_owner_count = _al_tls_get_dtor_owner_count();
if (*dtor_owner_count > 0)
return NULL;
_al_mutex_lock(&dtors->mutex);
{
#if defined(DEBUGMODE)
{
_AL_LIST_ITEM *iter = _al_list_front(dtors->dtors);
while (iter) {
DTOR *dtor = _al_list_item_data(iter);
ASSERT(dtor->object != object);
iter = _al_list_next(dtors->dtors, iter);
}
}
#endif 
{
DTOR *new_dtor = al_malloc(sizeof(DTOR));
if (new_dtor) {
new_dtor->object = object;
new_dtor->func = func;
new_dtor->name = name;
ALLEGRO_DEBUG("added dtor for %s %p, func %p\n", name,
object, func);
ret = _al_list_push_back(dtors->dtors, new_dtor);
}
else {
ALLEGRO_WARN("failed to add dtor for %s %p\n", name,
object);
}
}
}
_al_mutex_unlock(&dtors->mutex);
return ret;
}
void _al_unregister_destructor(_AL_DTOR_LIST *dtors, _AL_LIST_ITEM *dtor_item)
{
if (!dtor_item) {
return;
}
_al_mutex_lock(&dtors->mutex);
{
DTOR *dtor = _al_list_item_data(dtor_item);
ALLEGRO_DEBUG("removed dtor for %s %p\n", dtor->name, dtor->object);
al_free(dtor);
_al_list_erase(dtors->dtors, dtor_item);
}
_al_mutex_unlock(&dtors->mutex);
}
void _al_foreach_destructor(_AL_DTOR_LIST *dtors,
void (*callback)(void *object, void (*func)(void *), void *udata),
void *userdata)
{
_al_mutex_lock(&dtors->mutex);
{
_AL_LIST_ITEM *iter = _al_list_front(dtors->dtors);
while (iter) {
DTOR *dtor = _al_list_item_data(iter);
callback(dtor->object, dtor->func, userdata);
iter = _al_list_next(dtors->dtors, iter);
}
}
_al_mutex_unlock(&dtors->mutex);
}
