#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <uv.h>
#include "nvim/event/multiqueue.h"
#include "nvim/memory.h"
#include "nvim/os/time.h"
typedef struct multiqueue_item MultiQueueItem;
struct multiqueue_item {
union {
MultiQueue *queue;
struct {
Event event;
MultiQueueItem *parent_item;
} item;
} data;
bool link; 
QUEUE node;
};
struct multiqueue {
MultiQueue *parent;
QUEUE headtail; 
put_callback put_cb;
void *data;
size_t size;
};
typedef struct {
Event event;
bool fired;
int refcount;
} MulticastEvent; 
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "event/multiqueue.c.generated.h"
#endif
static Event NILEVENT = { .handler = NULL, .argv = {NULL} };
MultiQueue *multiqueue_new_parent(put_callback put_cb, void *data)
{
return multiqueue_new(NULL, put_cb, data);
}
MultiQueue *multiqueue_new_child(MultiQueue *parent)
FUNC_ATTR_NONNULL_ALL
{
assert(!parent->parent); 
parent->size++;
return multiqueue_new(parent, NULL, NULL);
}
static MultiQueue *multiqueue_new(MultiQueue *parent, put_callback put_cb,
void *data)
{
MultiQueue *rv = xmalloc(sizeof(MultiQueue));
QUEUE_INIT(&rv->headtail);
rv->size = 0;
rv->parent = parent;
rv->put_cb = put_cb;
rv->data = data;
return rv;
}
void multiqueue_free(MultiQueue *this)
{
assert(this);
while (!QUEUE_EMPTY(&this->headtail)) {
QUEUE *q = QUEUE_HEAD(&this->headtail);
MultiQueueItem *item = multiqueue_node_data(q);
if (this->parent) {
QUEUE_REMOVE(&item->data.item.parent_item->node);
xfree(item->data.item.parent_item);
}
QUEUE_REMOVE(q);
xfree(item);
}
xfree(this);
}
Event multiqueue_get(MultiQueue *this)
{
return multiqueue_empty(this) ? NILEVENT : multiqueue_remove(this);
}
void multiqueue_put_event(MultiQueue *this, Event event)
{
assert(this);
multiqueue_push(this, event);
if (this->parent && this->parent->put_cb) {
this->parent->put_cb(this->parent, this->parent->data);
}
}
void multiqueue_process_events(MultiQueue *this)
{
assert(this);
while (!multiqueue_empty(this)) {
Event event = multiqueue_remove(this);
if (event.handler) {
event.handler(event.argv);
}
}
}
void multiqueue_purge_events(MultiQueue *this)
{
assert(this);
while (!multiqueue_empty(this)) {
(void)multiqueue_remove(this);
}
}
bool multiqueue_empty(MultiQueue *this)
{
assert(this);
return QUEUE_EMPTY(&this->headtail);
}
void multiqueue_replace_parent(MultiQueue *this, MultiQueue *new_parent)
{
assert(multiqueue_empty(this));
this->parent = new_parent;
}
size_t multiqueue_size(MultiQueue *this)
{
return this->size;
}
static Event multiqueueitem_get_event(MultiQueueItem *item, bool remove)
{
assert(item != NULL);
Event ev;
if (item->link) {
MultiQueue *linked = item->data.queue;
assert(!multiqueue_empty(linked));
MultiQueueItem *child =
multiqueue_node_data(QUEUE_HEAD(&linked->headtail));
ev = child->data.item.event;
if (remove) {
QUEUE_REMOVE(&child->node);
xfree(child);
}
} else {
if (remove && item->data.item.parent_item) {
QUEUE_REMOVE(&item->data.item.parent_item->node);
xfree(item->data.item.parent_item);
item->data.item.parent_item = NULL;
}
ev = item->data.item.event;
}
return ev;
}
static Event multiqueue_remove(MultiQueue *this)
{
assert(!multiqueue_empty(this));
QUEUE *h = QUEUE_HEAD(&this->headtail);
QUEUE_REMOVE(h);
MultiQueueItem *item = multiqueue_node_data(h);
assert(!item->link || !this->parent); 
Event ev = multiqueueitem_get_event(item, true);
this->size--;
xfree(item);
return ev;
}
static void multiqueue_push(MultiQueue *this, Event event)
{
MultiQueueItem *item = xmalloc(sizeof(MultiQueueItem));
item->link = false;
item->data.item.event = event;
item->data.item.parent_item = NULL;
QUEUE_INSERT_TAIL(&this->headtail, &item->node);
if (this->parent) {
item->data.item.parent_item = xmalloc(sizeof(MultiQueueItem));
item->data.item.parent_item->link = true;
item->data.item.parent_item->data.queue = this;
QUEUE_INSERT_TAIL(&this->parent->headtail,
&item->data.item.parent_item->node);
}
this->size++;
}
static MultiQueueItem *multiqueue_node_data(QUEUE *q)
{
return QUEUE_DATA(q, MultiQueueItem, node);
}
Event event_create_oneshot(Event ev, int num)
{
MulticastEvent *data = xmalloc(sizeof(*data));
data->event = ev;
data->fired = false;
data->refcount = num;
return event_create(multiqueue_oneshot_event, 1, data);
}
static void multiqueue_oneshot_event(void **argv)
{
MulticastEvent *data = argv[0];
if (!data->fired) {
data->fired = true;
if (data->event.handler) {
data->event.handler(data->event.argv);
}
}
if ((--data->refcount) == 0) {
xfree(data);
}
}
