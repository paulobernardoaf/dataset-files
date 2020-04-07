#if !defined(NVIM_EVENT_LOOP_H)
#define NVIM_EVENT_LOOP_H

#include <stdint.h>

#include <uv.h>

#include "nvim/lib/klist.h"
#include "nvim/os/time.h"
#include "nvim/event/multiqueue.h"

typedef void * WatcherPtr;

#define _noop(x)
KLIST_INIT(WatcherPtr, WatcherPtr, _noop)

typedef struct loop {
uv_loop_t uv;
MultiQueue *events;
MultiQueue *thread_events;









MultiQueue *fast_events;


klist_t(WatcherPtr) *children;
uv_signal_t children_watcher;
uv_timer_t children_kill_timer;


uv_timer_t poll_timer;

uv_async_t async;
uv_mutex_t mutex;
int recursive;
} Loop;

#define CREATE_EVENT(multiqueue, handler, argc, ...) do { if (multiqueue) { multiqueue_put((multiqueue), (handler), argc, __VA_ARGS__); } else { void *argv[argc] = { __VA_ARGS__ }; (handler)(argv); } } while (0)












#define LOOP_PROCESS_EVENTS_UNTIL(loop, multiqueue, timeout, condition) do { int remaining = timeout; uint64_t before = (remaining > 0) ? os_hrtime() : 0; while (!(condition)) { LOOP_PROCESS_EVENTS(loop, multiqueue, remaining); if (remaining == 0) { break; } else if (remaining > 0) { uint64_t now = os_hrtime(); remaining -= (int) ((now - before) / 1000000); before = now; if (remaining <= 0) { break; } } } } while (0)


















#define LOOP_PROCESS_EVENTS(loop, multiqueue, timeout) do { if (multiqueue && !multiqueue_empty(multiqueue)) { multiqueue_process_events(multiqueue); } else { loop_poll_events(loop, timeout); } } while (0)









#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "event/loop.h.generated.h"
#endif

#endif 
