














#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_system.h"
#include "allegro5/internal/aintern_timer.h"
#include "allegro5/internal/aintern_exitfunc.h"
#include "allegro5/platform/allegro_internal_sdl.h"





static ALLEGRO_THREAD *thread;

static void wakeup_with_fake_timer_event(void)
{
ALLEGRO_EVENT_SOURCE *es = al_get_keyboard_event_source();
_al_event_source_lock(es);
ALLEGRO_EVENT event;
event.timer.type = ALLEGRO_EVENT_TIMER;
event.timer.timestamp = al_get_time();
event.timer.count = 0;
event.timer.error = 0;
_al_event_source_emit_event(es, &event);
_al_event_source_unlock(es);
}

static void *wakeup_thread(ALLEGRO_THREAD *thread, void *user)
{
al_rest(1);
while (!al_get_thread_should_stop(thread)) {

if (_al_get_active_timers_count())
break;
if (!al_is_keyboard_installed())
break;
wakeup_with_fake_timer_event();
al_rest(0.01);
}
return user;
}

static void _uninstall_sdl_event_hack(void)
{
if (thread) {
al_set_thread_should_stop(thread);
al_join_thread(thread, NULL);
al_destroy_thread(thread);
}
}

void _al_sdl_event_hack(void)
{
if (thread)
return;
_al_add_exit_func(_uninstall_sdl_event_hack, "uninstall_sdl_event_hack");
thread = al_create_thread(wakeup_thread, NULL);
al_start_thread(thread);
}
