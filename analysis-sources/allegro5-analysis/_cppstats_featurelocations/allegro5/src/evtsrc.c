




















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_dtor.h"
#include "allegro5/internal/aintern_events.h"
#include "allegro5/internal/aintern_system.h"


ALLEGRO_STATIC_ASSERT(evtsrc,
sizeof(ALLEGRO_EVENT_SOURCE_REAL) <= sizeof(ALLEGRO_EVENT_SOURCE));






void _al_event_source_init(ALLEGRO_EVENT_SOURCE *es)
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;

memset(es, 0, sizeof(*es));
_AL_MARK_MUTEX_UNINITED(this->mutex);
_al_mutex_init(&this->mutex);
_al_vector_init(&this->queues, sizeof(ALLEGRO_EVENT_QUEUE *));
this->data = 0;
}








void _al_event_source_free(ALLEGRO_EVENT_SOURCE *es)
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;


while (!_al_vector_is_empty(&this->queues)) {
ALLEGRO_EVENT_QUEUE **slot = _al_vector_ref_back(&this->queues);
al_unregister_event_source(*slot, es);
}

_al_vector_free(&this->queues);

_al_mutex_destroy(&this->mutex);
}






void _al_event_source_lock(ALLEGRO_EVENT_SOURCE *es)
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;

_al_mutex_lock(&this->mutex);
}






void _al_event_source_unlock(ALLEGRO_EVENT_SOURCE *es)
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;

_al_mutex_unlock(&this->mutex);
}









void _al_event_source_on_registration_to_queue(ALLEGRO_EVENT_SOURCE *es,
ALLEGRO_EVENT_QUEUE *queue)
{
_al_event_source_lock(es);
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;


ALLEGRO_EVENT_QUEUE **slot = _al_vector_alloc_back(&this->queues);
*slot = queue;
}
_al_event_source_unlock(es);
}







void _al_event_source_on_unregistration_from_queue(ALLEGRO_EVENT_SOURCE *es,
ALLEGRO_EVENT_QUEUE *queue)
{
_al_event_source_lock(es);
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;

_al_vector_find_and_delete(&this->queues, &queue);
}
_al_event_source_unlock(es);
}















bool _al_event_source_needs_to_generate_event(ALLEGRO_EVENT_SOURCE *es)
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;




return !_al_vector_is_empty(&this->queues);
}













void _al_event_source_emit_event(ALLEGRO_EVENT_SOURCE *es, ALLEGRO_EVENT *event)
{
ALLEGRO_EVENT_SOURCE_REAL *this = (ALLEGRO_EVENT_SOURCE_REAL *)es;

event->any.source = es;




{
size_t num_queues = _al_vector_size(&this->queues);
unsigned int i;
ALLEGRO_EVENT_QUEUE **slot;

for (i = 0; i < num_queues; i++) {
slot = _al_vector_ref(&this->queues, i);
_al_event_queue_push_event(*slot, event);
}
}
}





void al_init_user_event_source(ALLEGRO_EVENT_SOURCE *src)
{
ASSERT(src);

_al_event_source_init(src);
}





void al_destroy_user_event_source(ALLEGRO_EVENT_SOURCE *src)
{
if (src) {
_al_event_source_free(src);
}
}





bool al_emit_user_event(ALLEGRO_EVENT_SOURCE *src,
ALLEGRO_EVENT *event, void (*dtor)(ALLEGRO_USER_EVENT *))
{
size_t num_queues;
bool rc;

ASSERT(src);
ASSERT(event);

if (dtor) {
ALLEGRO_USER_EVENT_DESCRIPTOR *descr = al_malloc(sizeof(*descr));
descr->refcount = 0;
descr->dtor = dtor;
event->user.__internal__descr = descr;
}
else {
event->user.__internal__descr = NULL;
}

_al_event_source_lock(src);
{
ALLEGRO_EVENT_SOURCE_REAL *rsrc = (ALLEGRO_EVENT_SOURCE_REAL *)src;

num_queues = _al_vector_size(&rsrc->queues);
if (num_queues > 0) {
event->any.timestamp = al_get_time();
_al_event_source_emit_event(src, event);
rc = true;
}
else {
rc = false;
}
}
_al_event_source_unlock(src);

if (dtor && !rc) {
dtor(&event->user);
al_free(event->user.__internal__descr);
}

return rc;
}





void al_set_event_source_data(ALLEGRO_EVENT_SOURCE *source, intptr_t data)
{
ALLEGRO_EVENT_SOURCE_REAL *const rsource = (ALLEGRO_EVENT_SOURCE_REAL *)source;
rsource->data = data;
}





intptr_t al_get_event_source_data(const ALLEGRO_EVENT_SOURCE *source)
{
const ALLEGRO_EVENT_SOURCE_REAL *const rsource = (ALLEGRO_EVENT_SOURCE_REAL *)source;
return rsource->data;
}










