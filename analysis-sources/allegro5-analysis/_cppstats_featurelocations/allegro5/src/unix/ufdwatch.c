





















#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_thread.h"
#include "allegro5/internal/aintern_vector.h"
#include "allegro5/platform/aintunix.h"



typedef struct WATCH_ITEM
{
int fd;
void (*callback)(void *);
void *cb_data;
} WATCH_ITEM;


static _AL_THREAD fd_watch_thread;
static _AL_MUTEX fd_watch_mutex = _AL_MUTEX_UNINITED;
static _AL_VECTOR fd_watch_list = _AL_VECTOR_INITIALIZER(WATCH_ITEM);






static void fd_watch_thread_func(_AL_THREAD *self, void *unused)
{
(void)unused;

while (!_al_get_thread_should_stop(self)) {

fd_set rfds;
int max_fd;


_al_mutex_lock(&fd_watch_mutex);
{
WATCH_ITEM *wi;
unsigned int i;

FD_ZERO(&rfds);
max_fd = -1;

for (i = 0; i < _al_vector_size(&fd_watch_list); i++) {
wi = _al_vector_ref(&fd_watch_list, i);
FD_SET(wi->fd, &rfds);
if (wi->fd > max_fd)
max_fd = wi->fd;
}
}
_al_mutex_unlock(&fd_watch_mutex);


{
struct timeval tv;
int retval;

tv.tv_sec = 0;
tv.tv_usec = 250000;

retval = select(max_fd+1, &rfds, NULL, NULL, &tv);
if (retval < 1)
continue;
}


_al_mutex_lock(&fd_watch_mutex);
{
WATCH_ITEM *wi;
unsigned int i;

for (i = 0; i < _al_vector_size(&fd_watch_list); i++) {
wi = _al_vector_ref(&fd_watch_list, i);
if (FD_ISSET(wi->fd, &rfds)) {



wi->callback(wi->cb_data);
}
}
}
_al_mutex_unlock(&fd_watch_mutex);
}
}














void _al_unix_start_watching_fd(int fd, void (*callback)(void *), void *cb_data)
{
ASSERT(fd >= 0);
ASSERT(callback);


if (_al_vector_size(&fd_watch_list) == 0) {



_al_mutex_init_recursive(&fd_watch_mutex);
_al_thread_create(&fd_watch_thread, fd_watch_thread_func, NULL);
}


_al_mutex_lock(&fd_watch_mutex);
{
WATCH_ITEM *wi = _al_vector_alloc_back(&fd_watch_list);

wi->fd = fd;
wi->callback = callback;
wi->cb_data = cb_data;
}
_al_mutex_unlock(&fd_watch_mutex);
}










void _al_unix_stop_watching_fd(int fd)
{
bool list_empty = false;


_al_mutex_lock(&fd_watch_mutex);
{
WATCH_ITEM *wi;
unsigned int i;

for (i = 0; i < _al_vector_size(&fd_watch_list); i++) {
wi = _al_vector_ref(&fd_watch_list, i);
if (wi->fd == fd) {
_al_vector_delete_at(&fd_watch_list, i);
list_empty = _al_vector_is_empty(&fd_watch_list);
break;
}
}
}
_al_mutex_unlock(&fd_watch_mutex);


if (list_empty) {
_al_thread_join(&fd_watch_thread);
_al_mutex_destroy(&fd_watch_mutex);
_al_vector_free(&fd_watch_list);
}
}









