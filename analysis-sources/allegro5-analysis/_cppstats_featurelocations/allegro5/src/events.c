























#include <string.h>

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_dtor.h"
#include "allegro5/internal/aintern_exitfunc.h"
#include "allegro5/internal/aintern_events.h"
#include "allegro5/internal/aintern_system.h"



struct ALLEGRO_EVENT_QUEUE
{
_AL_VECTOR sources; 
_AL_VECTOR events; 
unsigned int events_head; 
unsigned int events_tail; 
bool paused;
_AL_MUTEX mutex;
_AL_COND cond;
_AL_LIST_ITEM *dtor_item;
};




static _AL_MUTEX user_event_refcount_mutex = _AL_MUTEX_UNINITED;




static void shutdown_events(void);
static bool do_wait_for_event(ALLEGRO_EVENT_QUEUE *queue,
ALLEGRO_EVENT *ret_event, ALLEGRO_TIMEOUT *timeout);
static void copy_event(ALLEGRO_EVENT *dest, const ALLEGRO_EVENT *src);
static void ref_if_user_event(ALLEGRO_EVENT *event);
static void unref_if_user_event(ALLEGRO_EVENT *event);
static void discard_events_of_source(ALLEGRO_EVENT_QUEUE *queue,
const ALLEGRO_EVENT_SOURCE *source);






void _al_init_events(void)
{
_al_mutex_init(&user_event_refcount_mutex);
_al_add_exit_func(shutdown_events, "shutdown_events");
}






static void shutdown_events(void)
{
_al_mutex_destroy(&user_event_refcount_mutex);
}





ALLEGRO_EVENT_QUEUE *al_create_event_queue(void)
{
ALLEGRO_EVENT_QUEUE *queue = al_malloc(sizeof *queue);

ASSERT(queue);

if (queue) {
_al_vector_init(&queue->sources, sizeof(ALLEGRO_EVENT_SOURCE *));

_al_vector_init(&queue->events, sizeof(ALLEGRO_EVENT));
_al_vector_alloc_back(&queue->events);
queue->events_head = 0;
queue->events_tail = 0;
queue->paused = false;

_AL_MARK_MUTEX_UNINITED(queue->mutex);
_al_mutex_init(&queue->mutex);
_al_cond_init(&queue->cond);

queue->dtor_item = _al_register_destructor(_al_dtor_list, "queue", queue,
(void (*)(void *)) al_destroy_event_queue);
}

return queue;
}





void al_destroy_event_queue(ALLEGRO_EVENT_QUEUE *queue)
{
ASSERT(queue);

_al_unregister_destructor(_al_dtor_list, queue->dtor_item);


while (_al_vector_is_nonempty(&queue->sources)) {
ALLEGRO_EVENT_SOURCE **slot = _al_vector_ref_back(&queue->sources);
al_unregister_event_source(queue, *slot);
}

ASSERT(_al_vector_is_empty(&queue->sources));
_al_vector_free(&queue->sources);

ASSERT(queue->events_head == queue->events_tail);
_al_vector_free(&queue->events);

_al_cond_destroy(&queue->cond);
_al_mutex_destroy(&queue->mutex);

al_free(queue);
}




bool al_is_event_source_registered(ALLEGRO_EVENT_QUEUE *queue, 
ALLEGRO_EVENT_SOURCE *source)
{
ASSERT(queue);
ASSERT(source);

if(_al_vector_contains(&queue->sources, &source))
return true;
else
return false;
}



void al_register_event_source(ALLEGRO_EVENT_QUEUE *queue,
ALLEGRO_EVENT_SOURCE *source)
{
ALLEGRO_EVENT_SOURCE **slot;
ASSERT(queue);
ASSERT(source);

if (!_al_vector_contains(&queue->sources, &source)) {
_al_event_source_on_registration_to_queue(source, queue);
_al_mutex_lock(&queue->mutex);
slot = _al_vector_alloc_back(&queue->sources);
*slot = source;
_al_mutex_unlock(&queue->mutex);
}
}





void al_unregister_event_source(ALLEGRO_EVENT_QUEUE *queue,
ALLEGRO_EVENT_SOURCE *source)
{
bool found;
ASSERT(queue);
ASSERT(source);


_al_mutex_lock(&queue->mutex);
found = _al_vector_find_and_delete(&queue->sources, &source);
_al_mutex_unlock(&queue->mutex);

if (found) {

_al_event_source_on_unregistration_from_queue(source, queue);


_al_mutex_lock(&queue->mutex);
discard_events_of_source(queue, source);
_al_mutex_unlock(&queue->mutex);
}
}





void al_pause_event_queue(ALLEGRO_EVENT_QUEUE *queue, bool pause)
{
ASSERT(queue);

_al_mutex_lock(&queue->mutex);
queue->paused = pause;
_al_mutex_unlock(&queue->mutex);
}





bool al_is_event_queue_paused(const ALLEGRO_EVENT_QUEUE *queue)
{
ASSERT(queue);

return queue->paused;
}



static void heartbeat(void)
{
ALLEGRO_SYSTEM *system = al_get_system_driver();
if (system->vt->heartbeat)
system->vt->heartbeat();
}



static bool is_event_queue_empty(ALLEGRO_EVENT_QUEUE *queue)
{
return (queue->events_head == queue->events_tail);
}





bool al_is_event_queue_empty(ALLEGRO_EVENT_QUEUE *queue)
{
ASSERT(queue);

heartbeat();

return is_event_queue_empty(queue);
}






static unsigned int circ_array_next(const _AL_VECTOR *vector, unsigned int i)
{
return (i + 1) % _al_vector_size(vector);
}










static ALLEGRO_EVENT *get_next_event_if_any(ALLEGRO_EVENT_QUEUE *queue,
bool delete)
{
ALLEGRO_EVENT *event;

if (is_event_queue_empty(queue)) {
return NULL;
}

event = _al_vector_ref(&queue->events, queue->events_tail);
if (delete) {
queue->events_tail = circ_array_next(&queue->events, queue->events_tail);
}
return event;
}





bool al_get_next_event(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT *ret_event)
{
ALLEGRO_EVENT *next_event;
ASSERT(queue);
ASSERT(ret_event);

heartbeat();

_al_mutex_lock(&queue->mutex);

next_event = get_next_event_if_any(queue, true);
if (next_event) {
copy_event(ret_event, next_event);

}

_al_mutex_unlock(&queue->mutex);

return (next_event ? true : false);
}





bool al_peek_next_event(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT *ret_event)
{
ALLEGRO_EVENT *next_event;
ASSERT(queue);
ASSERT(ret_event);

heartbeat();

_al_mutex_lock(&queue->mutex);

next_event = get_next_event_if_any(queue, false);
if (next_event) {
copy_event(ret_event, next_event);
ref_if_user_event(ret_event);
}

_al_mutex_unlock(&queue->mutex);

return (next_event ? true : false);
}





bool al_drop_next_event(ALLEGRO_EVENT_QUEUE *queue)
{
ALLEGRO_EVENT *next_event;
ASSERT(queue);

heartbeat();

_al_mutex_lock(&queue->mutex);

next_event = get_next_event_if_any(queue, true);
if (next_event) {
unref_if_user_event(next_event);
}

_al_mutex_unlock(&queue->mutex);

return (next_event ? true : false);
}





void al_flush_event_queue(ALLEGRO_EVENT_QUEUE *queue)
{
unsigned int i;
ASSERT(queue);

heartbeat();

_al_mutex_lock(&queue->mutex);


i = queue->events_tail;
while (i != queue->events_head) {
ALLEGRO_EVENT *old_ev = _al_vector_ref(&queue->events, i);
unref_if_user_event(old_ev);
i = circ_array_next(&queue->events, i);
}

queue->events_head = queue->events_tail = 0;
_al_mutex_unlock(&queue->mutex);
}






void al_wait_for_event(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT *ret_event)
{
ALLEGRO_EVENT *next_event = NULL;

ASSERT(queue);

heartbeat();

_al_mutex_lock(&queue->mutex);
{
while (is_event_queue_empty(queue)) {
_al_cond_wait(&queue->cond, &queue->mutex);
}

if (ret_event) {
next_event = get_next_event_if_any(queue, true);
copy_event(ret_event, next_event);
}
}
_al_mutex_unlock(&queue->mutex);
}






bool al_wait_for_event_timed(ALLEGRO_EVENT_QUEUE *queue,
ALLEGRO_EVENT *ret_event, float secs)
{
ALLEGRO_TIMEOUT timeout;

ASSERT(queue);
ASSERT(secs >= 0);

heartbeat();

if (secs < 0.0)
al_init_timeout(&timeout, 0);
else
al_init_timeout(&timeout, secs);

return do_wait_for_event(queue, ret_event, &timeout);
}





bool al_wait_for_event_until(ALLEGRO_EVENT_QUEUE *queue,
ALLEGRO_EVENT *ret_event, ALLEGRO_TIMEOUT *timeout)
{
ASSERT(queue);

heartbeat();

return do_wait_for_event(queue, ret_event, timeout);
}



static bool do_wait_for_event(ALLEGRO_EVENT_QUEUE *queue,
ALLEGRO_EVENT *ret_event, ALLEGRO_TIMEOUT *timeout)
{
bool timed_out = false;
ALLEGRO_EVENT *next_event = NULL;

_al_mutex_lock(&queue->mutex);
{
int result = 0;





while (is_event_queue_empty(queue) && (result != -1)) {
result = _al_cond_timedwait(&queue->cond, &queue->mutex, timeout);
}

if (result == -1)
timed_out = true;
else if (ret_event) {
next_event = get_next_event_if_any(queue, true);
copy_event(ret_event, next_event);
}
}
_al_mutex_unlock(&queue->mutex);

if (timed_out)
return false;

return true;
}






static void expand_events_array(ALLEGRO_EVENT_QUEUE *queue)
{

const size_t old_size = _al_vector_size(&queue->events);
const size_t new_size = old_size * 2;
unsigned int i;

for (i = old_size; i < new_size; i++) {
_al_vector_alloc_back(&queue->events);
}


if (queue->events_head < queue->events_tail) {
for (i = 0; i < queue->events_head; i++) {
ALLEGRO_EVENT *old_ev = _al_vector_ref(&queue->events, i);
ALLEGRO_EVENT *new_ev = _al_vector_ref(&queue->events, old_size + i);
copy_event(new_ev, old_ev);
}
queue->events_head += old_size;
}
}








static ALLEGRO_EVENT *alloc_event(ALLEGRO_EVENT_QUEUE *queue)
{
ALLEGRO_EVENT *event;
unsigned int adv_head;

adv_head = circ_array_next(&queue->events, queue->events_head);
if (adv_head == queue->events_tail) {
expand_events_array(queue);
adv_head = circ_array_next(&queue->events, queue->events_head);
}

event = _al_vector_ref(&queue->events, queue->events_head);
queue->events_head = adv_head;
return event;
}






static void copy_event(ALLEGRO_EVENT *dest, const ALLEGRO_EVENT *src)
{
ASSERT(dest);
ASSERT(src);

*dest = *src;
}






static void ref_if_user_event(ALLEGRO_EVENT *event)
{
if (ALLEGRO_EVENT_TYPE_IS_USER(event->type)) {
ALLEGRO_USER_EVENT_DESCRIPTOR *descr = event->user.__internal__descr;
if (descr) {
_al_mutex_lock(&user_event_refcount_mutex);
descr->refcount++;
_al_mutex_unlock(&user_event_refcount_mutex);
}
}
}






static void unref_if_user_event(ALLEGRO_EVENT *event)
{
if (ALLEGRO_EVENT_TYPE_IS_USER(event->type)) {
al_unref_user_event(&event->user);
}
}











void _al_event_queue_push_event(ALLEGRO_EVENT_QUEUE *queue,
const ALLEGRO_EVENT *orig_event)
{
ALLEGRO_EVENT *new_event;
ASSERT(queue);
ASSERT(orig_event);

if (queue->paused)
return;

_al_mutex_lock(&queue->mutex);
{
new_event = alloc_event(queue);
copy_event(new_event, orig_event);
ref_if_user_event(new_event);




_al_cond_broadcast(&queue->cond);
}
_al_mutex_unlock(&queue->mutex);
}







static bool contains_event_of_source(const ALLEGRO_EVENT_QUEUE *queue,
const ALLEGRO_EVENT_SOURCE *source)
{
ALLEGRO_EVENT *event;
unsigned int i;

i = queue->events_tail;
while (i != queue->events_head) {
event = _al_vector_ref(&queue->events, i);
if (event->any.source == source) {
return true;
}
i = circ_array_next(&queue->events, i);
}

return false;
}




static int pot(int x)
{
int y = 1;
while (y < x) y *= 2;
return y;
}







static void discard_events_of_source(ALLEGRO_EVENT_QUEUE *queue,
const ALLEGRO_EVENT_SOURCE *source)
{
_AL_VECTOR old_events;
ALLEGRO_EVENT *old_event;
ALLEGRO_EVENT *new_event;
size_t old_size;
size_t new_size;
unsigned int i;

if (!contains_event_of_source(queue, source)) {
return;
}


old_events = queue->events;
_al_vector_init(&queue->events, sizeof(ALLEGRO_EVENT));

i = queue->events_tail;
while (i != queue->events_head) {
old_event = _al_vector_ref(&old_events, i);
if (old_event->any.source != source) {
new_event = _al_vector_alloc_back(&queue->events);
copy_event(new_event, old_event);
}
else {
unref_if_user_event(old_event);
}
i = circ_array_next(&old_events, i);
}

queue->events_tail = 0;
queue->events_head = _al_vector_size(&queue->events);


old_size = _al_vector_size(&queue->events);
new_size = pot(old_size + 1);
for (i = old_size; i < new_size; i++) {
_al_vector_alloc_back(&queue->events);
}

_al_vector_free(&old_events);
}





void al_unref_user_event(ALLEGRO_USER_EVENT *event)
{
ALLEGRO_USER_EVENT_DESCRIPTOR *descr;
int refcount;

ASSERT(event);

descr = event->__internal__descr;
if (descr) {
_al_mutex_lock(&user_event_refcount_mutex);
ASSERT(descr->refcount > 0);
refcount = --descr->refcount;
_al_mutex_unlock(&user_event_refcount_mutex);

if (refcount == 0) {
(descr->dtor)(event);
al_free(descr);
}
}
}










