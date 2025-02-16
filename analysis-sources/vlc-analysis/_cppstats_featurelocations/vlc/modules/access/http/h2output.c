



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#if defined(HAVE_POLL)
#include <poll.h>
#endif
#if defined(HAVE_SYS_UIO_H)
#include <sys/uio.h>
#endif
#include <vlc_common.h>
#include <vlc_tls.h>
#include "h2frame.h"
#include "h2output.h"

#define VLC_H2_MAX_QUEUE (1u << 24)

struct vlc_h2_queue
{
struct vlc_h2_frame *first;
struct vlc_h2_frame **last;
};

struct vlc_h2_output
{
struct vlc_tls *tls;

struct vlc_h2_queue prio; 
struct vlc_h2_queue queue; 
size_t size; 
bool failed; 
bool closing; 

vlc_mutex_t lock;
vlc_cond_t wait;
vlc_thread_t thread;
};


static int vlc_h2_output_queue(struct vlc_h2_output *out,
struct vlc_h2_queue *q, struct vlc_h2_frame *f)
{
if (unlikely(f == NULL))
return -1; 


struct vlc_h2_frame **lastp = &f;
size_t len = 0;

do
{
struct vlc_h2_frame *n = *lastp;

len += vlc_h2_frame_size(n);
lastp = &n->next;
}
while (*lastp != NULL);

vlc_mutex_lock(&out->lock);
if (out->failed)
goto error;

out->size += len;
if (out->size >= VLC_H2_MAX_QUEUE)
{ 



out->size -= len;
goto error;
}

assert(*(q->last) == NULL);
*(q->last) = f;
q->last = lastp;
vlc_cond_signal(&out->wait);
vlc_mutex_unlock(&out->lock);
return 0;

error:
vlc_mutex_unlock(&out->lock);
while (f != NULL)
{
struct vlc_h2_frame *n = f->next;

free(f);
f = n;
}
return -1;
}

int vlc_h2_output_send_prio(struct vlc_h2_output *out, struct vlc_h2_frame *f)
{
return vlc_h2_output_queue(out, &out->prio, f);
}

int vlc_h2_output_send(struct vlc_h2_output *out, struct vlc_h2_frame *f)
{
return vlc_h2_output_queue(out, &out->queue, f);
}


static struct vlc_h2_frame *vlc_h2_output_dequeue(struct vlc_h2_output *out)
{
struct vlc_h2_queue *q;
struct vlc_h2_frame *frame;
size_t len;

vlc_mutex_lock(&out->lock);

for (;;)
{
q = &out->prio;
if (q->first != NULL)
break;

q = &out->queue;
if (q->first != NULL)
break;

if (unlikely(out->closing))
{
vlc_mutex_unlock(&out->lock);
return NULL;
}

int canc = vlc_savecancel();
vlc_cond_wait(&out->wait, &out->lock);
vlc_restorecancel(canc);
}

frame = q->first;
q->first = frame->next;
if (frame->next == NULL)
{
assert(q->last == &frame->next);
q->last = &q->first;
}
assert(q->last != &frame->next);

len = vlc_h2_frame_size(frame);
assert(out->size >= len);
out->size -= len;

vlc_mutex_unlock(&out->lock);

frame->next = NULL;
return frame;
}

static void vlc_h2_output_flush_unlocked(struct vlc_h2_output *out)
{
for (struct vlc_h2_frame *f = out->prio.first, *n; f != NULL; f = n)
{
n = f->next;
free(f);
}
for (struct vlc_h2_frame *f = out->queue.first, *n; f != NULL; f = n)
{
n = f->next;
free(f);
}
}






static ssize_t vlc_https_send(vlc_tls_t *tls, const void *buf, size_t len)
{
struct iovec iov;
size_t count = 0;

iov.iov_base = (void *)buf;
iov.iov_len = len;

while (count < len)
{
int canc = vlc_savecancel();
ssize_t val = tls->ops->writev(tls, &iov, 1);

vlc_restorecancel(canc);

if (val > 0)
{
iov.iov_base = (char *)iov.iov_base + val;
iov.iov_len -= val;
count += val;
continue;
}

if (val == 0)
break;

if (errno != EINTR && errno != EAGAIN)
return count ? (ssize_t)count : -1;

struct pollfd ufd;

ufd.events = POLLOUT;
ufd.fd = vlc_tls_GetPollFD(tls, &ufd.events);
poll(&ufd, 1, -1);
}

return count;
}













static int vlc_h2_frame_send(struct vlc_tls *tls, struct vlc_h2_frame *f)
{
size_t len = vlc_h2_frame_size(f);
ssize_t val;

vlc_cleanup_push(free, f);
val = vlc_https_send(tls, f->data, len);
vlc_cleanup_pop();
free(f);

return ((size_t)val == len) ? 0 : -1;
}


static void *vlc_h2_output_thread(void *data)
{
struct vlc_h2_output *out = data;
struct vlc_h2_frame *frame;

while ((frame = vlc_h2_output_dequeue(out)) != NULL)
{
if (vlc_h2_frame_send(out->tls, frame))
{ 

vlc_mutex_lock(&out->lock);
out->failed = true;
vlc_mutex_unlock(&out->lock);



vlc_h2_output_flush_unlocked(out);
out->prio.first = NULL;
out->prio.last = &out->prio.first;
out->queue.first = NULL;
out->queue.last = &out->queue.first;
break;
}
}

return NULL;
}

static void *vlc_h2_client_output_thread(void *data)
{
static const char http2_hello[] = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
struct vlc_h2_output *out = data;

if (vlc_https_send(out->tls, http2_hello, 24) < 24)
{
vlc_mutex_lock(&out->lock);
out->failed = true;
vlc_mutex_unlock(&out->lock);
return NULL;
}

return vlc_h2_output_thread(data);
}

struct vlc_h2_output *vlc_h2_output_create(struct vlc_tls *tls, bool client)
{
struct vlc_h2_output *out = malloc(sizeof (*out));
if (unlikely(out == NULL))
return NULL;

out->tls = tls;

out->prio.first = NULL;
out->prio.last = &out->prio.first;
out->queue.first = NULL;
out->queue.last = &out->queue.first;
out->size = 0;
out->failed = false;
out->closing = false;

vlc_mutex_init(&out->lock);
vlc_cond_init(&out->wait);

void *(*cb)(void *) = client ? vlc_h2_client_output_thread
: vlc_h2_output_thread;
if (vlc_clone(&out->thread, cb, out, VLC_THREAD_PRIORITY_INPUT))
{
free(out);
out = NULL;
}
return out;
}

void vlc_h2_output_destroy(struct vlc_h2_output *out)
{
vlc_mutex_lock(&out->lock);
out->closing = true;
vlc_cond_signal(&out->wait);
vlc_mutex_unlock(&out->lock);

vlc_cancel(out->thread);
vlc_join(out->thread, NULL);



vlc_h2_output_flush_unlocked(out);
free(out);
}
