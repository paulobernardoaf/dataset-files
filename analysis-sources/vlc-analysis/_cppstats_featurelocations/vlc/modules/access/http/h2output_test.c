



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#undef NDEBUG

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <vlc_common.h>
#include <vlc_tls.h>
#include "h2frame.h"
#include "h2output.h"

#undef vlc_tick_sleep

const char vlc_module_name[] = "test_h2output";

static unsigned char counter = 0;
static bool send_failure = false;
static bool expect_hello = true;
static vlc_sem_t rx;

static int fd_callback(vlc_tls_t *tls, short *restrict events)
{
(void) tls;
(void) events;
return fileno(stderr); 
}

static ssize_t send_callback(vlc_tls_t *tls, const struct iovec *iov,
unsigned count)
{
assert(count == 1);
assert(tls->ops->writev == send_callback);

const uint8_t *p = iov->iov_base;
size_t len = iov->iov_len;

if (expect_hello)
{
assert(len >= 24);
assert(!memcmp(p, "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n", 24));

expect_hello = false;
vlc_sem_post(&rx);

if (len == 24)
return send_failure ? -1 : (ssize_t)len;

p += 24;
len -= 24;
}

assert(len == 9 + 1);
assert(p[9] == counter);

if (send_failure)
errno = EIO;
else
counter++;
vlc_sem_post(&rx);

return send_failure ? -1 : (ssize_t)len;
}

static const struct vlc_tls_operations fake_ops =
{
.get_fd = fd_callback,
.writev = send_callback,
};

static vlc_tls_t fake_tls =
{
.ops = &fake_ops,
};

static struct vlc_h2_frame *frame(unsigned char c)
{
struct vlc_h2_frame *f = vlc_h2_frame_data(1, &c, 1, false);
assert(f != NULL);
return f;
}

static struct vlc_h2_frame *frame_list(struct vlc_h2_frame *first, ...)
{
struct vlc_h2_frame **pp = &first;
va_list ap;

va_start(ap, first);
for (struct vlc_h2_frame *f = first;
f != NULL;
f = va_arg(ap, struct vlc_h2_frame *))
{
*pp = f;
pp = &f->next;
}
va_end(ap);
return first;
}

int main(void)
{
struct vlc_h2_output *out;


out = vlc_h2_output_create(&fake_tls, false);
assert(out != NULL);
vlc_h2_output_destroy(out);

vlc_sem_init(&rx, 0);
out = vlc_h2_output_create(&fake_tls, expect_hello = true);
assert(out != NULL);
vlc_h2_output_destroy(out);


vlc_sem_init(&rx, 0);
out = vlc_h2_output_create(&fake_tls, false);
assert(out != NULL);
assert(vlc_h2_output_send_prio(out, NULL) == -1);
assert(vlc_h2_output_send_prio(out, frame(0)) == 0);
assert(vlc_h2_output_send_prio(out, frame(1)) == 0);
assert(vlc_h2_output_send(out, NULL) == -1);
assert(vlc_h2_output_send(out, frame(2)) == 0);
assert(vlc_h2_output_send(out, frame(3)) == 0);
assert(vlc_h2_output_send(out, frame_list(frame(4), frame(5),
frame(6), NULL)) == 0);
assert(vlc_h2_output_send(out, frame(7)) == 0);
for (unsigned i = 0; i < 8; i++)
vlc_sem_wait(&rx);

assert(vlc_h2_output_send_prio(out, frame(8)) == 0);
assert(vlc_h2_output_send(out, frame(9)) == 0);

vlc_h2_output_destroy(out);


send_failure = true;

vlc_sem_init(&rx, 0);
counter = 10;
out = vlc_h2_output_create(&fake_tls, false);
assert(out != NULL);

assert(vlc_h2_output_send(out, frame(10)) == 0);
for (unsigned char i = 11; vlc_h2_output_send(out, frame(i)) == 0; i++)
vlc_tick_sleep(VLC_TICK_FROM_MS(100)); 
assert(vlc_h2_output_send(out, frame(0)) == -1);
assert(vlc_h2_output_send_prio(out, frame(0)) == -1);
vlc_h2_output_destroy(out);


vlc_sem_init(&rx, 0);
counter = 0;
out = vlc_h2_output_create(&fake_tls, expect_hello = true);
assert(out != NULL);
vlc_sem_wait(&rx);

for (unsigned char i = 1; vlc_h2_output_send_prio(out, frame(i)) == 0; i++)
vlc_tick_sleep(VLC_TICK_FROM_MS(100));
assert(vlc_h2_output_send(out, frame(0)) == -1);
assert(vlc_h2_output_send_prio(out, frame(0)) == -1);
vlc_h2_output_destroy(out);

return 0;
}
